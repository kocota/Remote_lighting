#include "ModbusPacketTask.h"
#include "cmsis_os.h"
#include "modbus.h"
#include "fm25v02.h"
#include "m95.h"

extern osSemaphoreId ModbusPacketReceiveHandle;
extern osTimerId Ring_Center_TimerHandle;
extern osMutexId Fm25v02MutexHandle;
extern osMutexId UartMutexHandle;

extern uint32_t crc_temp;
extern uint8_t modbus_buffer[20][256];
extern uint8_t modbus_packet_number;
extern uint8_t modbus_packet_number1;
extern uint8_t Version_H;
extern control_register_struct control_registers;

uint8_t buf_out[256];
uint8_t buf_out1[256];
uint8_t level;
uint16_t modbus_size;
uint16_t modbus_address;

volatile uint8_t phase_a_control_state = 0; // переменная статуса включения фазы А
volatile uint8_t phase_b_control_state = 0; // переменная статуса включения фазы В
volatile uint8_t phase_c_control_state = 0; // переменная статуса включения фазы С


void ThreadModbusPacketTask(void const * argument)
{
	uint8_t temp_h1;
	uint8_t temp_l1;

	osSemaphoreWait(ModbusPacketReceiveHandle, osWaitForever); // обнуляем семафор, при создании семафора его значение равно 1



	for(;;)
	{
		osSemaphoreWait(ModbusPacketReceiveHandle, osWaitForever);

		switch(modbus_buffer[modbus_packet_number1][1]) // проверяем тип поступившей команды MODBUS и формируем соответствующий ответ
		{
			case(0x03): // чтение регистра

				modbus_address = (((((uint16_t)modbus_buffer[modbus_packet_number1][2])<<8)&0xFF00)|(((uint16_t)modbus_buffer[modbus_packet_number1][3])&0xFF)); // считаем адрес регистра для чтения
				modbus_size = (((((uint16_t)modbus_buffer[modbus_packet_number1][4])<<8)&0xFF00)|(((uint16_t)modbus_buffer[modbus_packet_number1][5])&0xFF)); //  считаем количество регистров для чтения
					/*
					if( modbus_address == VERSION_REG ) // костыль, при вычитывании версии прошивки
					{
						osMutexWait(Fm25v02MutexHandle, osWaitForever);
						fm25v02_fast_read( modbus_address , &buf_out[0] , modbus_size); // читаем из памяти необходимое количество регистров
						osMutexRelease(Fm25v02MutexHandle);

						buf_out1[0] = 0x01;
						buf_out1[1] = 0x03;
						buf_out1[2] = 2*modbus_size;
						for(uint8_t i=0; i<2*modbus_size; i++)
						{
							if(i==0)
							{
								buf_out1[2*i+3] = Version_H;
								buf_out1[2*i+4] = buf_out[i];
							}
							else if(i!=0)
							{
								buf_out1[2*i+3] = 0;
								buf_out1[2*i+4] = buf_out[i];
							}
						}
						crc_temp = CRC16(&buf_out1[0], 3+2*modbus_size);
						buf_out1[2*modbus_size+3] = (uint8_t)(crc_temp&0x00FF);
						buf_out1[2*modbus_size+4] = (uint8_t)((crc_temp>>8)&0x00FF);

						osMutexWait(UartMutexHandle, osWaitForever);
						if( AT_QISEND(&buf_out1[0], 2*modbus_size+5) != AT_OK )
						{
							//LED7_ON();
						}
						osMutexRelease(UartMutexHandle);

						osTimerStop(Ring_Center_TimerHandle);
						osTimerStart(Ring_Center_TimerHandle, 300000);
					}

					else
					{
						osMutexWait(Fm25v02MutexHandle, osWaitForever);
						fm25v02_fast_read( modbus_address , &buf_out[0] , modbus_size); // читаем из памяти необходимое количество регистров
						osMutexRelease(Fm25v02MutexHandle);

						buf_out1[0] = 0x01;
						buf_out1[1] = 0x03;
						buf_out1[2] = 2*modbus_size;
						for(uint8_t i=0; i<2*modbus_size; i++)
						{
							buf_out1[2*i+3] = 0;
							buf_out1[2*i+4] = buf_out[i];
						}
						crc_temp = CRC16(&buf_out1[0], 3+2*modbus_size);
						buf_out1[2*modbus_size+3] = (uint8_t)(crc_temp&0x00FF);
						buf_out1[2*modbus_size+4] = (uint8_t)((crc_temp>>8)&0x00FF);

						osMutexWait(UartMutexHandle, osWaitForever);
						if( AT_QISEND(&buf_out1[0], 2*modbus_size+5) != AT_OK )
						{
							//LED7_ON();
						}
						osMutexRelease(UartMutexHandle);

					}
					*/

				// обработка пакета чтения 16-битного регистра modbus

				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_fast_read( 2*modbus_address , &buf_out[0] , 2*modbus_size); // читаем из памяти необходимое количество регистров
				osMutexRelease(Fm25v02MutexHandle);

				buf_out1[0] = 0x01;
				buf_out1[1] = 0x03;
				buf_out1[2] = 2*modbus_size;
				for(uint8_t i=0; i<modbus_size; i++)
				{
					buf_out1[2*i+3] = buf_out[2*i];
					buf_out1[2*i+4] = buf_out[2*i+1];
				}
				crc_temp = CRC16(&buf_out1[0], 3+2*modbus_size);
				buf_out1[2*modbus_size+3] = (uint8_t)(crc_temp&0x00FF);
				buf_out1[2*modbus_size+4] = (uint8_t)((crc_temp>>8)&0x00FF);

				osMutexWait(UartMutexHandle, osWaitForever);
				if( AT_QISEND(&buf_out1[0], 2*modbus_size+5) != AT_OK )
				{
					//LED7_ON();
				}
				osMutexRelease(UartMutexHandle);

				if( modbus_address == VERSION_REG ) // если запрашивается адрес версии прошивки обновляем таймер перезагрузки
				{
					//osTimerStop(Ring_Center_TimerHandle);
					//osTimerStart(Ring_Center_TimerHandle, 300000);
				}


			break;

			case(0x10): // запись нескольких регистров

				modbus_address = (((((uint16_t)modbus_buffer[modbus_packet_number1][2])<<8)&0xFF00)|(((uint16_t)modbus_buffer[modbus_packet_number1][3])&0xFF)); // считаем адрес регистра для записи
				modbus_size = (((((uint16_t)modbus_buffer[modbus_packet_number1][4])<<8)&0xFF00)|(((uint16_t)modbus_buffer[modbus_packet_number1][5])&0xFF)); //  считаем количество регистров для чтения

				if( !( (modbus_address>=0x1000) && (modbus_address<=0x108F) ) && !( (modbus_address<0x1000) && (modbus_address+modbus_size>0x1000) ) ) // модбас адресс не должен находиться в области статусных регистров, а также запись не должна затрагивать статусные регистры
				{
					// обработка пакета записи 8-битного регистра modbus
					/*
					for(uint8_t a=0; a<(modbus_buffer[modbus_packet_number1][6])/2; a++) // исправил 'i' на 'a', так как в функции fm25v02_fast_write() внутри уже есть 'i'
					{
						osMutexWait(Fm25v02MutexHandle, osWaitForever);
						fm25v02_fast_write(modbus_address+a, &modbus_buffer[modbus_packet_number1][8+a*2], 1); // Записываем в память значения регистров полученных от сервера
						osMutexRelease(Fm25v02MutexHandle);
					}

					buf_out1[0] = 0x01; // формируем буфер для ответа на сервер, о том что полученные от сервера регистры записаны в память
					buf_out1[1] = 0x10;
					buf_out1[2] = modbus_buffer[modbus_packet_number1][2];
					buf_out1[3] = modbus_buffer[modbus_packet_number1][3];
					buf_out1[4] = modbus_buffer[modbus_packet_number1][4];
					buf_out1[5] = modbus_buffer[modbus_packet_number1][5];

					crc_temp = CRC16(&buf_out1[0], 6);

					buf_out1[6] = (uint8_t)(crc_temp&0x00FF);
					buf_out1[7] = (uint8_t)((crc_temp>>8)&0x00FF);

					osMutexWait(UartMutexHandle, osWaitForever); // берем мьютекс для работы с модемом
					if( AT_QISEND(&buf_out1[0], 8) != AT_OK ) // отправляем буфер с ответом на сервер о том, что полученные регистры записаны в память
					{
						//LED7_ON();
					}
					osMutexRelease(UartMutexHandle); // отдаем мьютекс для работы с модемом

					if( modbus_address == CONTROL_LOOP_REG)
					{
						osMutexWait(Fm25v02MutexHandle, osWaitForever);
						fm25v02_write(GPRS_CALL_REG, CALL_ON);
						osMutexRelease(Fm25v02MutexHandle);
						//osTimerStart(Ring_Center_TimerHandle, 1);
					}
					if( modbus_address == 0x2710)
					{
						osMutexWait(Fm25v02MutexHandle, osWaitForever);
						fm25v02_write(GPRS_CALL_REG, CALL_ON);
						osMutexRelease(Fm25v02MutexHandle);
						//osTimerStart(Ring_Center_TimerHandle, 1);
					}

					*/

					// обработка пакета записи 16-битного регистра modbus

					for(uint8_t a=0; a<(modbus_buffer[modbus_packet_number1][6])/2; a++)
					{
						osMutexWait(Fm25v02MutexHandle, osWaitForever);
						fm25v02_fast_write( (2*modbus_address+2*a), &modbus_buffer[modbus_packet_number1][7+a*2], 1 );
						fm25v02_fast_write( (2*modbus_address+2*a+1), &modbus_buffer[modbus_packet_number1][8+a*2], 1 );
						osMutexRelease(Fm25v02MutexHandle);
					}

					buf_out1[0] = 0x01;
					buf_out1[1] = 0x10;
					buf_out1[2] = modbus_buffer[modbus_packet_number1][2];
					buf_out1[3] = modbus_buffer[modbus_packet_number1][3];
					buf_out1[4] = modbus_buffer[modbus_packet_number1][4];
					buf_out1[5] = modbus_buffer[modbus_packet_number1][5];

					crc_temp = CRC16(&buf_out1[0], 6);

					buf_out1[6] = (uint8_t)(crc_temp&0x00FF);
					buf_out1[7] = (uint8_t)((crc_temp>>8)&0x00FF);

					osMutexWait(UartMutexHandle, osWaitForever);
					if( AT_QISEND(&buf_out1[0], 8) != AT_OK )
					{

					}
					osMutexRelease(UartMutexHandle);
					/*
					if( modbus_address == CONTROL_LOOP_REG)
					{
						osMutexWait(Fm25v02MutexHandle, osWaitForever);
						fm25v02_write(2*GPRS_CALL_REG, 0x00);
						fm25v02_write(2*GPRS_CALL_REG+1, CALL_ON);
						osMutexRelease(Fm25v02MutexHandle);
					}
					*/
					if( modbus_address == 0x2710)
					{
						osMutexWait(Fm25v02MutexHandle, osWaitForever);
						fm25v02_write(2*GPRS_CALL_REG, 0x00);
						fm25v02_write(2*GPRS_CALL_REG+1, CALL_ON);
						osMutexRelease(Fm25v02MutexHandle);
					}

					if( modbus_address == LIGHT_CONTROL_REG)
					{
						osMutexWait(Fm25v02MutexHandle, osWaitForever);
						fm25v02_read(2*LIGHT_CONTROL_REG, &temp_h1);
						fm25v02_read(2*LIGHT_CONTROL_REG+1, &temp_l1);
						control_registers.light_control_reg = (((uint16_t)temp_h1)<<8)|temp_l1;
						osMutexRelease(Fm25v02MutexHandle);

					}

				}


			break;
		}


		osDelay(1);
	}
}
