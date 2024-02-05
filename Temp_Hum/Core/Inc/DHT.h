#ifndef DHT_H_
#define DHT_H_

typedef struct
{
	float Temperatura;
	float Humedad;
}DHT_DataTypedef;


void DHT_GetData (DHT_DataTypedef *DHT_Data);

#endif
