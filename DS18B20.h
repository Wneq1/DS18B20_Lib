#ifndef DS18B20_H
#define DS18B20_H

#include "stm32l1xx_hal.h"

extern TIM_HandleTypeDef htim6;

extern uint8_t Temp_byte1;
extern uint8_t Temp_byte2;
extern uint16_t TEMP;

void delay(uint16_t time);
void Set_Pin_Input(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
void Set_Pin_Output(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
uint8_t DS18B20_Start(GPIO_TypeDef* port, uint16_t pin);
void DS18B20_Write(GPIO_TypeDef* port, uint16_t pin, uint8_t data);
uint8_t DS18B20_Read(GPIO_TypeDef* port, uint16_t pin);
float GetTemperature(GPIO_TypeDef* port, uint16_t pin,char scale); // TEMP ON DISP
uint8_t ALARM(float temp, float Alarm);
#endif
