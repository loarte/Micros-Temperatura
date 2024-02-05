#ifndef DHT22_H_
#define DHT22_H_

typedef struct
{
	float Temperatura;
	float Humedad;
}DHT_DataTypedef;


void DHT_GetData (DHT_DataTypedef *DHT_Data);

#endif /* INC_DHT22_H_ */
