#ifndef SWITCH_H
#define SWITCH_H

#include "BuildConfig.h"
#include "BaseDevice.h"
#include "IDFixTask.h"
#include "DeviceProperties.h"
#include "GPIOButton.h"
#include "GPIOLEDControl.h"
#include "timers.h"
extern "C"
{
    #include <esp_log.h>

    #include "HLW8012_ESP82.h"
    #if MEMORY_DEBUGGING == 1
        #include <freertos/FreeRTOS.h>
        #include <freertos/task.h>
    #endif
}

using namespace IDFix;


namespace _2log
{

    class Switch : public BaseDevice
    {

        public:
                            Switch(void);
            void            sensorDataAvailableCallback();
            static  void    sensorDataUpdateHelper(TimerHandle_t timer);

        private:

            void            init();
            void            setSwitch(bool on);

            virtual void    baseDeviceEventHandler(BaseDeviceEvent event) override;
            virtual void    baseDeviceStateChanged(BaseDeviceState state) override;

            virtual void    initProperties(cJSON *argument) override;
            virtual void    resetDeviceConfigurationAndRestart() override;


            void            setOn(cJSON* argument);
            void            resetEnergyCounter(cJSON*);


            IODevices::GPIOButton       _button;
            IODevices::GPIOLEDControl   _led;
            TimerHandle_t               _timer;
            uint8_t                     _sendRssiCounter = 0;
            bool                        _isInitalized = { false };
            bool                        _switchOn = false;

            #if MEMORY_DEBUGGING == 1
                TaskStatus_t taskStatusArray[MAX_DEBUGGING_TASKS_NUMBER];
            #endif
    };

}


#endif
