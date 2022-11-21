/*
 * FlashPROM.c
 *
 *  Created on: 30 дек. 2019 г.
 *      Author: dima
 */
 /*События, подлежащие записи в лог файл:
 1. Положение по концевому датчику
 2. Команда
 3. Значение с АЦП
 4. Отсутствие смены положения (с 10 раза)
 5. Попытка смены положения
 6. Температура чипа
*/

#include "FlashPROM.h"
#include "stm32f3xx_hal_CRC.h"

extern CRC_HandleTypeDef hcrc;
extern UART_HandleTypeDef huart3;
extern uint32_t res_addr;
///////////////////////Определение температуры кристалла/////////////////////////////////////////////////////////

uint8_t process_temperature (uint16_t raw_data) {
  // see datasheet for position of the calibration values, this is for STM32F429
  const uint16_t* const ADC_TEMP_3V3_30C =  (uint16_t*)(0x1FFFF7B8);
  const uint16_t* const ADC_TEMP_3V3_110C =  (uint16_t*)(0x1FFFF7C2);
  const float CALIBRATION_REFERENCE_VOLTAGE = 3.3F;

  const float REFERENCE_VOLTAGE = 3.0F; // supplied with Vref+ or VDDA

  // scale constants to current reference voltage
  float adcCalTemp30C = (float)(*ADC_TEMP_3V3_30C) * (REFERENCE_VOLTAGE/CALIBRATION_REFERENCE_VOLTAGE);
  float adcCalTemp110C = (float)(*ADC_TEMP_3V3_110C) * (REFERENCE_VOLTAGE/CALIBRATION_REFERENCE_VOLTAGE);

  float chip_temperature = ((float)(raw_data) - adcCalTemp30C)/
		(adcCalTemp110C - adcCalTemp30C) * (110.0F - 30.0F) + 30.0F;
  //uint16_t chip_temperature = ((1.43 - raw_data)/4.3) + 25;
	
	if (chip_temperature < 0 || chip_temperature > 255)
		return 255;

  return chip_temperature;
}

//////////////////////// ОЧИСТКА ПАМЯТИ /////////////////////////////
void erase_flash(void) {
	static FLASH_EraseInitTypeDef EraseInitStruct;     // структура для очистки флеша

	EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES; // постраничная очистка, FLASH_TYPEERASE_MASSERASE - очистка всего флеша
	EraseInitStruct.PageAddress = STARTADDR;
	EraseInitStruct.NbPages = PAGES;
	//EraseInitStruct.Banks = FLASH_BANK_1; // FLASH_BANK_2 - банк №2, FLASH_BANK_BOTH - оба банка
	uint32_t page_error = 0; // переменная, в которую запишется адрес страницы при неудачном стирании

	HAL_FLASH_Unlock(); // разблокировать флеш

	if(HAL_FLASHEx_Erase(&EraseInitStruct, &page_error) != HAL_OK) {
		uint32_t er = HAL_FLASH_GetError();
		char str[64] = {0,};
		snprintf(str, 64, "ER %lu\n", er);
		HAL_UART_Transmit(&huart3, (uint8_t*)str, strlen(str), 100);
	}	else	{
		#if DEBUG
		HAL_UART_Transmit(&huart3, (uint8_t*)"Erase OK\n", 9, 100);
		#endif
	}
	HAL_FLASH_Lock();
}

//////////////////////// ПОИСК СВОБОДНЫХ ЯЧЕЕК /////////////////////////////
uint32_t flash_search_adress(uint32_t address, uint16_t cnt) {
	uint16_t count_byte = cnt;

	while(count_byte) {
		if(0xFF == *(uint8_t*)address++) count_byte--;
		else count_byte = cnt;

		if(address == ENDMEMORY - 1) {  // если достигнут конец флеша 
			erase_flash();	// тогда очищаем память
			#if DEBUG
			HAL_UART_Transmit(&huart3, (uint8_t*)"New cicle\n", 10, 100);
			#endif
			return STARTADDR;	// устанавливаем адрес для записи с самого начала
		}
	}
	return address -= cnt;
}

//////////////////////// ЗАПИСЬ ДАННЫХ /////////////////////////////
void write_to_flash(myBuf_t *buff) {
	res_addr = flash_search_adress(res_addr, BUFFSIZE * DATAWIDTH); 
	// ищем свободные ячейки начиная с последнего известного адреса

	//////////////////////// ЗАПИСЬ ////////////////////////////
	HAL_FLASH_Unlock(); // разблокировать флеш

	for(uint16_t i = 0; i < BUFFSIZE; i++)
	{
		if(HAL_FLASH_Program(WIDTHWRITE, res_addr, buff[i]) != HAL_OK)
		{
			uint32_t er = HAL_FLASH_GetError();
			char str[64] = {0,};
			snprintf(str, 64, "ER %lu\n", er);
			HAL_UART_Transmit(&huart3, (uint8_t*)str, strlen(str), 100);
		}

		res_addr = res_addr + DATAWIDTH;
	}

	HAL_FLASH_Lock(); // заблокировать флеш

	//////////////////////// проверка записанного (это можно удплить если неохота проверять) ////////////////////////
	uint32_t crcbuff[BUFFSIZE] = {0,};

	for(uint16_t i = 0; i < BUFFSIZE; i++) crcbuff[i] = (uint32_t)buff[i]; // в функцию CRC32 нужно подавать 32-х битные значения, поэтому перегоняем 16-ти битный буфер в 32-х битный

	uint32_t sum1 = HAL_CRC_Calculate(&hcrc, (uint32_t*)crcbuff, BUFFSIZE); // crc буфера который только что записали

	buff[0] = 0;
	read_last_data_in_flash(buff); // читаем что записали

	for(uint16_t i = 0; i < BUFFSIZE; i++) crcbuff[i] = (uint32_t)buff[i];

	uint32_t sum2 = HAL_CRC_Calculate(&hcrc, (uint32_t*)crcbuff, BUFFSIZE); // crc прочитанного

	/*#if DEBUG
	char str[64] = {0,};
	snprintf(str, 64, "SUM %lu %lu\n", sum1, sum2);
	HAL_UART_Transmit(&huart3, (uint8_t*)str, strlen(str), 100);
	
	#endif*/

	if(sum1 != sum2) { // если суммы записанного и прочитанного не равны, тогда что-то пошло не так
		#if DEBUG
		HAL_UART_Transmit(&huart3, (uint8_t*)"Write buff ERROR\n", 17, 100);
		#endif
		return;
	}
	//////////////////////// конец проверки записанного ////////////////////////
/*
	#if DEBUG
	HAL_UART_Transmit(&huart3, (uint8_t*)"Write buff OK\n", 14, 100);
	#endif*/
}

//////////////////////// ЧТЕНИЕ ПОСЛЕДНИХ ДАННЫХ /////////////////////////////
void read_last_data_in_flash(myBuf_t *buff)
{
	if(res_addr == STARTADDR) {
		#if DEBUG
		HAL_UART_Transmit(&huart3, (uint8_t*)"Flash empty\n", 12, 100);
		#endif
		return;
	}

	uint32_t adr = res_addr - BUFFSIZE * DATAWIDTH; // сдвигаемся на начало последних данных

	for(uint16_t i = 0; i < BUFFSIZE; i++) {
		buff[i] = *(myBuf_t*)adr; // читаем
		adr = adr + DATAWIDTH;
	}
}

