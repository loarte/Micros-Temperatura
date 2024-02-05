#include "stm32f4xx_hal.h"
#include "DHT22.h"
#include "i2c-lcd.h"

//estamos trabajando con la libreria de DHT, pero puede ser para DHT11 o DHT22, al ser el nuestro el DHT22 la resolución es de 16 bits
#define TYPE_DHT22

//usamos el puerto PA1
#define DHT_PORT GPIOA
#define DHT_PIN GPIO_PIN_1

//Nuestras variables globales son:
uint8_t HR_byte1, HR_byte2, Temp_byte1, Temp_byte2; //Guardan los byte con información de la Humedad relativa y la temp
uint16_t SUM ; //Comprueba que se han enviado correctamente los datos
uint8_t Presence = 0 ;	//Indica si el sensor ha detectado la petición de recibir datos de la placa

uint32_t DWT_Delay_Init(void)
{
  /* Disable TRC */
  CoreDebug->DEMCR &= ~CoreDebug_DEMCR_TRCENA_Msk; // ~0x01000000;
  /* Enable TRC */
  CoreDebug->DEMCR |=  CoreDebug_DEMCR_TRCENA_Msk; // 0x01000000;

  /* Disable clock cycle counter */
  DWT->CTRL &= ~DWT_CTRL_CYCCNTENA_Msk; //~0x00000001;
  /* Enable  clock cycle counter */
  DWT->CTRL |=  DWT_CTRL_CYCCNTENA_Msk; //0x00000001;

  /* Reset the clock cycle counter value */
  DWT->CYCCNT = 0;

     /* 3 NO OPERATION instructions */
     __ASM volatile ("NOP");
     __ASM volatile ("NOP");
     __ASM volatile ("NOP");

  /* Check if clock cycle counter has started */
     if(DWT->CYCCNT)
     {
       return 0; /*clock cycle counter started*/
     }
     else
  {
    return 1; /*clock cycle counter not started*/
  }
}

__STATIC_INLINE void delay(volatile uint32_t microseconds) //Usando la frecuencia del reloj del sistema creamos un delay
{
  uint32_t clk_cycle_start = DWT->CYCCNT;

  /* Go to number of cycles for system */
  microseconds *= (HAL_RCC_GetHCLKFreq() / 1000000);

  /* Delay till end */
  while ((DWT->CYCCNT - clk_cycle_start) < microseconds);
}

void Set_Pin_Output (GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin) //Definimos como salida un pin dado
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = GPIO_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}

void Set_Pin_Input (GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin) //Definimos un pin dado como entrada
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = GPIO_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}


void DHT_Start (void){ //Petición de la placa para recibir datos.
					   //Se crea un pulso a nivel bajo de 1-10ms, seguido de uno a nivel alto de 20-40us. (DTH22)
					   //Tras mandar el pulso se pone el pin como entrada para recibir datos del sensor

	DWT_Delay_Init();
	Set_Pin_Output (DHT_PORT, DHT_PIN);  // set the pin as output
	HAL_GPIO_WritePin (DHT_PORT, DHT_PIN, 0);   // pull the pin low

#if defined(TYPE_DHT22)
	delay (10000);  // >1ms delay
#endif

    HAL_GPIO_WritePin (DHT_PORT, DHT_PIN, 1);   // pull the pin high
    delay (30);   // wait for 43us
	Set_Pin_Input(DHT_PORT, DHT_PIN);    // set as input
}

uint8_t DHT_Check_Response (void){ //Comprobación de que el sensor ha detectado la petición de la placa
								   //Devuelve 0 si no lo ha detectado, -1 si el pulso de confirmación es errónea y 1 si es correcto

	uint8_t Response = 0;
	delay (40);
	if (!(HAL_GPIO_ReadPin (DHT_PORT, DHT_PIN)))
	{
		delay (80);
		if ((HAL_GPIO_ReadPin (DHT_PORT, DHT_PIN))) Response = 1;
		else Response = -1;
	}
	while (HAL_GPIO_ReadPin (DHT_PORT, DHT_PIN));   // wait for the pin to go low

	return Response;
}

uint8_t DHT_Read (void){ //Lee un byte de datos (1 bit cada 35us). Desplaza y concatena cada bit en una sola variable

	uint8_t i,j;
	for (j=0;j<8;j++)
	{
		while (!(HAL_GPIO_ReadPin (DHT_PORT, DHT_PIN)));   // wait for the pin to go high
		delay (35);   // wait for 35 us
		if (!(HAL_GPIO_ReadPin (DHT_PORT, DHT_PIN)))   // if the pin is low
		{
			i&= ~(1<<(7-j));   // write 0
		}
		else i|= (1<<(7-j));  // if the pin is high, write 1
		while ((HAL_GPIO_ReadPin (DHT_PORT, DHT_PIN)));  // wait for the pin to go low
	}
	return i;
}



void DHT_GetData (DHT_DataTypedef *DHT_Data)
{
    DHT_Start ();
	Presence = DHT_Check_Response ();
	if (Presence == 1){
		HR_byte1 = DHT_Read ();
		HR_byte2 = DHT_Read ();
		Temp_byte1 = DHT_Read ();
		Temp_byte2 = DHT_Read ();
		SUM = DHT_Read();

		if (SUM == (HR_byte1+HR_byte2+Temp_byte1+Temp_byte2)) //Comprobamos que se haya envado correctamente
		{
			#if defined(TYPE_DHT22)
				DHT_Data->Temperatura = ((Temp_byte1<<8)|Temp_byte2);
				DHT_Data->Humedad = ((HR_byte1<<8)|HR_byte2);
			#endif
		}
	}
	else if (Presence == -1){ //Si se produce un error en la detección, se ponen a nivel alto las señales

		#if defined(TYPE_DHT22)
			DHT_Data->Temperatura = 0xFFFF;
			DHT_Data->Humedad = 0xFFFF;
		#endif
	}

}

