/****************************************************************************************************************************
 Simple Exsamples control and read PWM signal on esp32
*****************************************************************************************************************************/

#include <Arduino.h>
#include <driver/ledc.h>

//set pins
#define SET_FAN GPIO_NUM_16 // Set  PWM Fan pin
#define SET_TACHO GPIO_NUM_18 //Set Tacho pin

//Global varies 
uint32_t counterTacho = 0;
uint32_t speed = 1023;


static void IRAM_ATTR intr_handler(void *args)
{
  counterTacho++;
}

void tachoTask(void *pvParameters)
{
  while(1)
  {
    vTaskDelay(1000/portTICK_RATE_MS);
    Serial.println(counterTacho*30);
    counterTacho=0;
  }
  
  vTaskDelete(NULL);
}

static void speedChange(void *pvParameters)
{
  while (true)
  {
    for (int i = 1; i <= 1023; i +=2)
    {
      speed = i ;
      ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, speed));
      ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0));
      Serial.print("i=");
      Serial.println(i);
      vTaskDelay(200 / portTICK_RATE_MS);
    }
    for (int i = 1023; i >= 1; i -=2)
    {
      speed = i ;
      ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, speed));
      ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0));
      Serial.print("i=");
      Serial.println(i);
      vTaskDelay(200 / portTICK_RATE_MS);
    }
  }
}

void setup()
{
  Serial.begin(115200);
  vTaskDelay(1000/portTICK_RATE_MS);
 
  //config interrupt
  gpio_set_intr_type(SET_TACHO,GPIO_INTR_POSEDGE);
  gpio_install_isr_service(0);
  gpio_isr_handler_add(SET_TACHO,intr_handler,NULL);

  //config timer for interrupt
  ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_LOW_SPEED_MODE,
        .duty_resolution  = LEDC_TIMER_10_BIT,
        .timer_num        = LEDC_TIMER_0,
        .freq_hz          = 25000,
        .clk_cfg          = LEDC_AUTO_CLK
    };
  ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));


  //config for PWM
    ledc_channel_config_t ledc_channel = {
        .gpio_num       = SET_FAN,
        .speed_mode     = LEDC_LOW_SPEED_MODE,
        .channel        = LEDC_CHANNEL_0,
        .timer_sel      = LEDC_TIMER_0,
        .duty           = speed,
        .hpoint         = 0
    };
  ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
  
  //config task
  xTaskCreate(speedChange, "SpeeedChange", 1024, NULL, 1, NULL);
  xTaskCreate(tachoTask,"Tacho Task",2048,NULL,20,NULL);
  
}

void loop()
{
}