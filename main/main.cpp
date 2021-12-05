#include "Switch.h"

extern "C"
{
	#include <stdio.h>
	#include "freertos/FreeRTOS.h"
	#include "freertos/task.h"
	#include "esp_system.h"
	#include "esp_spi_flash.h"
}

static _2log::Switch switchInstance;


extern "C"
{
    void app_main()
	{        
        switchInstance.startDevice();
		vTaskDelete(nullptr);
	}
}
