#include "Switch.h"


namespace
{
    const char* LOG_TAG = "_2log::Switch";
}

_2log::Switch::Switch() : BaseDevice(),
    _button(SWITCH_BUTTON_GPIO, true),
    _led(LED_GPIO, true)
{
    {
        gpio_config_t switchPin;
        switchPin.intr_type		= GPIO_INTR_DISABLE;
        switchPin.pin_bit_mask	= (BIT(SWITCH_ACTOR_GPIO));
        switchPin.mode			= GPIO_MODE_OUTPUT;
        switchPin.pull_down_en	= GPIO_PULLDOWN_DISABLE;
        switchPin.pull_up_en	= GPIO_PULLUP_DISABLE;
        ESP_ERROR_CHECK(gpio_config(&switchPin));
    }

    bool success;
    bool value = DeviceProperties::instance().getProperty("on", false).asBool(&success);
    if(success)
        setSwitch(value);

    _timer = xTimerCreate("update_timer", pdMS_TO_TICKS(SEND_DATA_INTERVAL), false, this, &Switch::sensorDataUpdateHelper);

    if(_timer == nullptr)
        ESP_LOGE(LOG_TAG, "Failed to create software timer for data updates");

    if (xTimerStart(_timer, 0)!=pdPASS) {
        ESP_LOGD(LOG_TAG, "Failed to start update timer...");
    }
    _led.setOff();
    _button.setPressDuration(3000);
    _button.setButtonPressedCallback(std::bind(static_cast<void(Switch::*)(void)>(&Switch::resetDeviceConfigurationAndRestart), this));
}


void _2log::Switch::baseDeviceEventHandler(BaseDeviceEvent event)
{
    switch(event)
    {
        case BaseDeviceEvent::NetworkConnected:
        {
            init();
            return;
        }

        default: ESP_LOGD(LOG_TAG, "No handler implemented for this event.");
    }
}

void _2log::Switch::baseDeviceStateChanged(BaseDeviceState state)
{
    switch(state)
    {
        case BaseDeviceState::Configuring:      _led.blink(500); return;
        case BaseDeviceState::Connecting:
            _led.blink(100);
            _button.start();
            if( _button.isPressed() )
            {
                resetDeviceConfigurationAndRestart();
            }
            return;
        case BaseDeviceState::Connected:
            _led.setOn();
        return;
        case BaseDeviceState::UpdatingFirmware: _led.blinkSequence(400, 100); return;
        default: return;
    }
}

void _2log::Switch::init()
{
    if ( _isInitalized )
    {
        ESP_LOGW(LOG_TAG, "Already initialized!");
        return;
    }


    HLW8012_init(GPIO_NUM_5,GPIO_NUM_14,GPIO_NUM_12,1,0);

    _deviceNode->registerRPC("setOn", std::bind( static_cast<void(Switch::*)(cJSON*)>(&Switch::setOn), this, std::placeholders::_1) );
    _deviceNode->registerRPC("resetCounter", std::bind( static_cast<void(Switch::*)(cJSON*)>(&Switch::resetEnergyCounter), this, std::placeholders::_1) );
    _isInitalized = true;

    return;
}

void _2log::Switch::setSwitch(bool on)
{
    ESP_LOGI(LOG_TAG, "Switch ON/OFF");
	gpio_set_level( static_cast<gpio_num_t>(SWITCH_ACTOR_GPIO), on ? 1 : 0);
    DeviceProperties::instance().saveProperty("on", on);
    _switchOn = on;
    if(getState() == BaseDeviceState::Connected)
        _deviceNode->setProperty("on", _switchOn);
}

void _2log::Switch::initProperties(cJSON *argument)
{
    BaseDevice::initProperties(argument);

    cJSON_AddStringToObject(argument, ".fwvers",     DEVICE_FIRMWARE_VERSION);
    cJSON_AddStringToObject(argument, ".fwbuild",    DEVICE_FIRMWARE_BUILD);

    cJSON_AddNumberToObject(argument, "volt",       0);
    cJSON_AddNumberToObject(argument, "curr",       0);
    cJSON_AddNumberToObject(argument, "power",      0);
    cJSON_AddNumberToObject(argument, "cons",       0);
    cJSON_AddNumberToObject(argument, "on",         _switchOn);
}

void _2log::Switch::resetDeviceConfigurationAndRestart()
{
    ESP_LOGI(LOG_TAG, "resetDeviceConfigurationAndRestart");

    _led.setOn();
    IDFix::Task::delay(1000);

    setSwitch(false);


    BaseDevice::resetDeviceConfigurationAndRestart();
}

void _2log::Switch::sensorDataAvailableCallback()
{
    if(getState() != BaseDeviceState::Connected)
        return;

    cJSON *payload = cJSON_CreateObject();
    cJSON_AddNumberToObject(payload, "curr", (float) HLW8012_getCurrent() / 100.0);
    cJSON_AddNumberToObject(payload, "volt", HLW8012_getVoltage());
    cJSON_AddNumberToObject(payload, "power", HLW8012_getActivePower());
    cJSON_AddNumberToObject(payload, "cons", HLW8012_getEnergy());

    #if MEMORY_DEBUGGING == 1
    {
        cJSON_AddNumberToObject(payload, ".freeHeapMemory", esp_get_free_heap_size() );
        cJSON_AddNumberToObject(payload, ".minFreeHeapSize", esp_get_minimum_free_heap_size() );

        // .taskStackMin- + 15
        char propertyName[configMAX_TASK_NAME_LEN + 15];

        uint32_t ulTotalRunTime;
        UBaseType_t numberOfTasks = uxTaskGetSystemState( taskStatusArray, MAX_DEBUGGING_TASKS_NUMBER, &ulTotalRunTime );

        for(UBaseType_t taskNumber = 0; taskNumber < numberOfTasks; taskNumber++ )
        {
            snprintf(propertyName, configMAX_TASK_NAME_LEN + 15, ".taskStackMin-%s", taskStatusArray[taskNumber].pcTaskName );
            cJSON_AddNumberToObject(payload, propertyName, taskStatusArray[taskNumber].usStackHighWaterMark );
        }
    }
    #endif

    if(_sendRssiCounter++ == 15) // send Wifi siginal every ~15s
    {
        cJSON_AddNumberToObject(payload, ".rssi",       getRSSI());
        _sendRssiCounter = 0;
    }
    _deviceNode->setProperties(payload);
    cJSON_Delete(payload);
}

void _2log::Switch::sensorDataUpdateHelper(TimerHandle_t timer)
{
    auto switchPtr = static_cast<Switch*>(pvTimerGetTimerID( timer ));
    switchPtr->sensorDataAvailableCallback();

    if (xTimerStart(timer, 0)!=pdPASS) {
        ESP_LOGD(LOG_TAG, "Failed to start update timer...");
    }
}

void _2log::Switch::setOn(cJSON *argument)
{
    cJSON *stateValItem = cJSON_GetObjectItem(argument, "val");
    if(cJSON_IsBool(stateValItem) )
    {
         setSwitch(cJSON_IsTrue(stateValItem));
    }
    else
    {
        ESP_LOGE(LOG_TAG, "cJSON_IsNumber(stateValItem) failed");
    }
}


void _2log::Switch::resetEnergyCounter(cJSON *)
{
   HLW8012_resetEnergy();
   _deviceNode->setProperty("cons", 0);
}



