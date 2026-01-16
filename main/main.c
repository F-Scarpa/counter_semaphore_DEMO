#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "stdint.h"
#include "stdlib.h"
#include "freertos/semphr.h"  
#include "esp_adc/adc_oneshot.h"          


static SemaphoreHandle_t counterSemaphore; 
adc_oneshot_unit_handle_t adc_handle;

void producer(void *params)
{
    
    while(true)
    {
        int potVal = 0;
        adc_oneshot_read(adc_handle, ADC_CHANNEL_6, &potVal);
        printf("%d\n",potVal);
        vTaskDelay(pdMS_TO_TICKS(1000)); 
        if(potVal > 1000)                           
        {
            if (uxSemaphoreGetCount(counterSemaphore) < 5)              //verify semaphore counter is below 5
            {
                xSemaphoreGive(counterSemaphore);                       //increase counter by 1     
            }
        }
    }
}

void consumer(void *params)                  
{
    while(true)
    {
        if(xSemaphoreTake(counterSemaphore, portMAX_DELAY) == pdTRUE)
        {
        printf("consumer called\n");
        vTaskDelay(pdMS_TO_TICKS(2000));
        }

    }
}




void app_main(void)
{
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE                
    };
    adc_oneshot_new_unit(&init_config, &adc_handle);

  
    adc_oneshot_chan_cfg_t channel_config = {
        .bitwidth = ADC_BITWIDTH_12,      
        .atten = ADC_ATTEN_DB_12,          
    };
    adc_oneshot_config_channel(adc_handle, ADC_CHANNEL_6, &channel_config);

    
    counterSemaphore = xSemaphoreCreateCounting(5,0);       //1. max counter val
                                                            //2. start counter val      
    
    xTaskCreate(&producer,"producer",2048, NULL, 2, NULL);
    xTaskCreate(&consumer,"consumer_1",2048, NULL, 1, NULL);

            
}