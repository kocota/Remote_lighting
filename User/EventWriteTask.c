#include "EventWriteTask.h"
#include "cmsis_os.h"
#include "modbus.h"
#include "fm25v02.h"
//#include "m95.h"

extern osMutexId Fm25v02MutexHandle;
extern osThreadId EventWriteTaskHandle;
extern status_register_struct status_registers;
extern control_register_struct control_registers;


void ThreadEventWriteTask(void const * argument)
{
	uint8_t temp_data[30]; // буфер для записи событий
	uint16_t address_event_temp; // переменная для записи адреса последнего записанного события
	uint8_t read_temp; // временная переменная для чтения из памяти

	osMutexWait(Fm25v02MutexHandle, osWaitForever); // выставляем адрес начала записи событий по умолчанию
	fm25v02_write(2*ADDRESS_PROCESSED_EVENT_H_REG, 0x00);
	fm25v02_write(2*ADDRESS_PROCESSED_EVENT_H_REG+1, 0x20);
	fm25v02_write(2*ADDRESS_PROCESSED_EVENT_L_REG, 0x00);
	fm25v02_write(2*ADDRESS_PROCESSED_EVENT_L_REG+1, 0x00);
	osMutexRelease(Fm25v02MutexHandle);

	osMutexWait(Fm25v02MutexHandle, osWaitForever); // вычитывавем из памяти значение адреса последнего события
	fm25v02_read(2*ADDRESS_LAST_EVENT_H_REG+1, &read_temp);
	status_registers.address_last_event_h_reg = read_temp;
	fm25v02_read(2*ADDRESS_LAST_EVENT_L_REG+1, &read_temp);
	status_registers.address_last_event_l_reg = read_temp;
	osMutexRelease(Fm25v02MutexHandle);

	address_event_temp = (((status_registers.address_last_event_h_reg)<<8)|(status_registers.address_last_event_l_reg&0x00FF)); // высчитываем адрес последнего события

	if( (address_event_temp < 0x2000) || (address_event_temp > 0x7FFF) ) // проверяем, входит ли значение последнего события в диапазон памяти событий, если нет, то выставляем по умолчанию
	{
		osMutexWait(Fm25v02MutexHandle, osWaitForever);
		fm25v02_write(2*ADDRESS_LAST_EVENT_H_REG, 0x00);
		fm25v02_write(2*ADDRESS_LAST_EVENT_H_REG+1, 0x20);
		fm25v02_write(2*ADDRESS_LAST_EVENT_L_REG, 0x00);
		fm25v02_write(2*ADDRESS_LAST_EVENT_L_REG+1, 0x00);
		osMutexRelease(Fm25v02MutexHandle);
	}



	for(;;)
	{
		osThreadSuspend(EventWriteTaskHandle);

		osMutexWait(Fm25v02MutexHandle, osWaitForever); // вычитывавем из памяти значение последнего события
		fm25v02_read(2*ADDRESS_LAST_EVENT_H_REG+1, &read_temp);
		status_registers.address_last_event_h_reg = read_temp;
		fm25v02_read(2*ADDRESS_LAST_EVENT_L_REG+1, &read_temp);
		status_registers.address_last_event_l_reg = read_temp;
		osMutexRelease(Fm25v02MutexHandle);

		address_event_temp = (((status_registers.address_last_event_h_reg)<<8)|(status_registers.address_last_event_l_reg&0x00FF)); // считаем значение адреса последнего события

		if( (address_event_temp >= 0x2000) && (address_event_temp <= 0x7FFF) ) // проверяем, чтобы значение адреса события лежало в области памяти ведения протокола событий
		{

			osMutexWait(Fm25v02MutexHandle, osWaitForever);

			fm25v02_read(2*TIME_CURRENT_YEAR_REG+1, &temp_data[0]);
			fm25v02_read(2*TIME_CURRENT_MONTH_REG+1, &temp_data[1]);
			fm25v02_read(2*TIME_CURRENT_DAY_REG+1, &temp_data[2]);
			fm25v02_read(2*TIME_CURRENT_HOUR_REG+1, &temp_data[3]);
			fm25v02_read(2*TIME_CURRENT_MINUTE_REG+1, &temp_data[4]);
			fm25v02_read(2*TIME_CURRENT_SECOND_REG+1, &temp_data[5]);
			fm25v02_read(2*SYSTEM_STATUS_REG+1, &temp_data[6]);
			fm25v02_read(2*SECURITY_STATUS_REG+1, &temp_data[7]);
			fm25v02_read(2*STATUS_LOOP_REG+1, &temp_data[8]);
			fm25v02_read(2*ALARM_LOOP_REG+1, &temp_data[9]);
			fm25v02_read(2*ERROR_LOOP_REG+1, &temp_data[10]);
			fm25v02_read(2*IBUTTON_COMPLETE_0_REG+1, &temp_data[11]);
			fm25v02_read(2*IBUTTON_COMPLETE_1_REG+1, &temp_data[12]);
			fm25v02_read(2*IBUTTON_COMPLETE_2_REG+1, &temp_data[13]);
			fm25v02_read(2*IBUTTON_COMPLETE_3_REG+1, &temp_data[14]);
			fm25v02_read(2*IBUTTON_COMPLETE_4_REG+1, &temp_data[15]);
			fm25v02_read(2*IBUTTON_COMPLETE_5_REG+1, &temp_data[16]);
			fm25v02_read(2*IBUTTON_COMPLETE_6_REG+1, &temp_data[17]);
			fm25v02_read(2*IBUTTON_COMPLETE_7_REG+1, &temp_data[18]);
			fm25v02_read(2*POWER_ON_REG+1, &temp_data[19]);

			osMutexRelease(Fm25v02MutexHandle);

			osMutexWait(Fm25v02MutexHandle, osWaitForever);
			//fm25v02_fast_write(address_event_temp, &temp_data[0], 30); // переписываем текущие значения переменных для события в память

			for(uint8_t i=0; i<30; i++) // переписываем текущие значения переменных для события в память
			{
				fm25v02_write(2*(address_event_temp+i), 0x00);
				fm25v02_write(2*(address_event_temp+i)+1, temp_data[i]);
			}

			osMutexRelease(Fm25v02MutexHandle);

			address_event_temp = address_event_temp + 30;

			if( address_event_temp > 0x7FFF )
			{
				osMutexWait(Fm25v02MutexHandle, osWaitForever);

				address_event_temp = 0x2000;

				read_temp = (uint8_t)((address_event_temp>>8)&0x00FF);
				fm25v02_write(2*ADDRESS_LAST_EVENT_H_REG+1, read_temp);
				read_temp = (uint8_t)(address_event_temp&0x00FF);
				fm25v02_write(2*ADDRESS_LAST_EVENT_L_REG+1, read_temp);

				osMutexRelease(Fm25v02MutexHandle);

			}
			else if( (address_event_temp >= 0x2000) && (address_event_temp <= 0x7FFF) )
			{
				osMutexWait(Fm25v02MutexHandle, osWaitForever);

				read_temp = (uint8_t)((address_event_temp>>8)&0x00FF);
				fm25v02_write(2*ADDRESS_LAST_EVENT_H_REG+1, read_temp);
				read_temp = (uint8_t)(address_event_temp&0x00FF);
				fm25v02_write(2*ADDRESS_LAST_EVENT_L_REG+1, read_temp);

				osMutexRelease(Fm25v02MutexHandle);


			}

		}
		else // здесь должно быть то, что нужно сделать если значение адреса не вошло в область памяти ведения протокола событий, например можно выставить соответствующий флаг и сделать запрос на сервер
		{

		}

		osDelay(1);
	}
}
