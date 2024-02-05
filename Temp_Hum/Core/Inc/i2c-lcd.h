#include "stm32f4xx_hal.h"

void lcd_init(void);   // Inicializamos el LCD

void lcd_send_cmd(char cmd);  // funcion para envío de comandos

void lcd_send_data(char data);  // funcion para envio de información

void lcd_send_string(char *str);  // funcion para envío de una cadena de caracteres

void lcd_put_cur(int row, int col);  // funcion para el desplazamiento del cursor: row (0 or 1), col (0-15);

void lcd_clear(void);  // funcion para la limpieza de toda la pantalla

void lcd_clear_row (int row);  // funcion para la limpieza de una fila

void Display_Rh(float Rh);  // funcion que muestra el valor Rh por pantalla, siempre en la fila 1

void Display_Temp(float Temp, int row);  // funcion que muestra el valor Temp por pantalla, tanto en la fila 0 y 1
