#include "DS18B20.h"


uint8_t Temp_byte1;
uint8_t Temp_byte2;
uint16_t TEMP;

void TIM6_Configuration(void)
{
    htim6.Instance = TIM6;
    htim6.Init.Prescaler = (50 - 1);  // Prescaler for 1us resolution
    htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim6.Init.Period = 0xffff-1;
    htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    HAL_TIM_Base_Init(&htim6);
    HAL_TIM_Base_Start(&htim6);
}

void delay (uint16_t time)
{
	/* change your code here for the delay in microseconds */
	__HAL_TIM_SET_COUNTER(&htim6, 0);
	while ((__HAL_TIM_GET_COUNTER(&htim6))<time);
}


uint8_t Rh_byte1, Rh_byte2, Temp_byte1, Temp_byte2;
uint16_t SUM, RH, TEMP;

void Set_Pin_Output (GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = GPIO_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}

void Set_Pin_Input (GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = GPIO_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}

uint8_t DS18B20_Start(GPIO_TypeDef* port, uint16_t pin)
{
	uint8_t Response = 0;
	Set_Pin_Output(port, pin);   // set the pin as output
	HAL_GPIO_WritePin (port, pin, 0);  // pull the pin low
	delay (480);   // delay according to datasheet

	Set_Pin_Input(port, pin);    // set the pin as input
	delay (80);    // delay according to datasheet

	if (!(HAL_GPIO_ReadPin (port, pin))) Response = 1;    // if the pin is low i.e the presence pulse is detected
	else Response = -1;

	delay (400); // 480 us delay totally.

	return Response;
}

void DS18B20_Write (GPIO_TypeDef* port, uint16_t pin,uint8_t data)
{
	Set_Pin_Output(port, pin);  // set as output

	for (int i=0; i<8; i++)
	{

		if ((data & (1<<i))!=0)  // if the bit is high
		{
			// write 1

			Set_Pin_Output(port, pin);  // set as output
			HAL_GPIO_WritePin (port, pin, 0);  // pull the pin LOW
			delay (1);  // wait for 1 us

			Set_Pin_Input(port, pin);  // set as input
			delay (50);  // wait for 60 us
		}

		else  // if the bit is low
		{
			// write 0

			Set_Pin_Output(port, pin);
			HAL_GPIO_WritePin (port, pin, 0);  // pull the pin LOW
			delay (50);  // wait for 60 us

			Set_Pin_Input(port, pin);
		}
	}
}

uint8_t DS18B20_Read (GPIO_TypeDef* port, uint16_t pin)
{
	uint8_t value=0;

	Set_Pin_Input(port, pin);

	for (int i=0;i<8;i++)
	{
		Set_Pin_Output(port, pin);   // set as output

		HAL_GPIO_WritePin (port, pin, 0);  // pull the data pin LOW
		delay (1);  // wait for > 1us

		Set_Pin_Input(port, pin);  // set as input
		if (HAL_GPIO_ReadPin (port, pin))  // if the pin is HIGH
		{
			value |= 1<<i;  // read = 1
		}
		delay (50);  // wait for 60 us
	}
	return value;
}

float GetTemperature(GPIO_TypeDef* port, uint16_t pin,char scale)
{
    float temperature = 0.0f;
    uint8_t presence = 0;
    uint8_t tempByte1 = 0;
    uint8_t tempByte2 = 0;
    uint16_t temp = 0;

    presence = DS18B20_Start(port, pin);     // Sprawdzenie obecności czujnika
    HAL_Delay(1);
    DS18B20_Write(port, pin, 0xCC);          // Wysłanie komendy "Skip ROM"
    DS18B20_Write(port, pin, 0x44);          // Wysłanie komendy "Convert T"
    HAL_Delay(800);
    presence = DS18B20_Start(port, pin);     // Ponowne sprawdzenie obecności czujnika
    HAL_Delay(1);
    DS18B20_Write(port, pin, 0xCC);          // Wysłanie komendy "Skip ROM"
    DS18B20_Write(port, pin, 0xBE);          // Wysłanie komendy "Read Scratch-pad"
    tempByte1 = DS18B20_Read(port, pin);     // Odczyt pierwszego bajtu danych temperatury
    tempByte2 = DS18B20_Read(port, pin);     // Odczyt drugiego bajtu danych temperatury
    temp = (tempByte2 << 8) | tempByte1;     // Połączenie odczytanych bajtów w jedną wartość temperatury
    temperature = (float)temp / 16;          // Obliczenie temperatury w stopniach Celsiusza

    if (scale == 'F' || scale == 'f') {
           temperature = (temperature * 9 / 5) + 32;
       }

    return temperature;
}

uint8_t ALARM(float temp, float Alarm) {
    if (temp > Alarm) {
        return 1; // Temperatura przekroczyła próg alarmu
    } else {
        return 0; // Temperatura nie przekroczyła progu alarmu
    }
}
