#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include "modbus.h"
#include "fm25v02.h"


extern osSemaphoreId RS485TransmissionStateHandle;
extern osMutexId Fm25v02MutexHandle;
extern UART_HandleTypeDef huart6;
extern uint8_t meter_rx_buffer[256];
extern uint8_t meter_rx_number;
extern status_register_struct status_registers;

uint8_t start_buf[8] = {0x2F, 0x3F, 0x00, 0x00, 0x00, 0x21, 0x0D, 0x0A};
uint8_t password_buf[6] = {0x06, 0x30, 0x35, 0x31, 0x0D, 0x0A};
uint8_t voltage_buf[13] = {0x01, 0x52, 0x31, 0x02, 0x56, 0x4F, 0x4C, 0x54, 0x41, 0x28, 0x29, 0x03, 0x5F};
uint8_t end_buf[5] = {0x01, 0x42, 0x30, 0x03, 0x75};
uint8_t current_buf[13] = {0x01, 0x52, 0x31, 0x02, 0x43, 0x55, 0x52, 0x52, 0x45, 0x28, 0x29, 0x03, 0x5A};
uint8_t power_buf[13] = {0x01, 0x52, 0x31, 0x02, 0x50, 0x4F, 0x57, 0x50, 0x50, 0x28, 0x29, 0x03, 0x6F};
uint8_t tarif_buf[13] = {0x01, 0x52, 0x31, 0x02, 0x45, 0x54, 0x30, 0x50, 0x45, 0x28, 0x29, 0x03, 0x37};

void meter_rx_buffer_clear (void)
{
	for(uint16_t i=0; i<256; i++)
	{
		meter_rx_buffer[i] = 0;
	}
}

void ce303_voltage (uint8_t meter_id) // функция чтения напряения из счетчика СЕ303
{
	uint16_t temp;
	uint16_t temp_mil;
	uint8_t voltage_a_state; // переменные состояния прочтения значений напряжения ( если переменная равна 0, то значение напряжения не прочитано, если 1 - значение прочитанно )
	uint8_t voltage_b_state;
	uint8_t voltage_c_state;

	if( (meter_id>99)&&(meter_id<1000) )
	{
		start_buf[0] = 0x2F;
		start_buf[1] = 0x3F;
		start_buf[2] = (meter_id/100) + 0x30;
		start_buf[3] = ((meter_id - (meter_id/100)*100)/10) + 0x30;
		start_buf[4] = ((meter_id - ((meter_id/100)*100)) - (((meter_id - ((meter_id/100)*100))/10)*10)) + 0x30 ;
		start_buf[5] = 0x21;
		start_buf[6] = 0x0D;
		start_buf[7] = 0x0A;

		meter_rx_number = 0;
		meter_rx_buffer_clear(); // очищаем буфер приема счетчика
		HAL_GPIO_WritePin(GPIOG, GPIO_PIN_10, GPIO_PIN_SET); // устанавливаем микросхему на передачу
		HAL_UART_Transmit_DMA(&huart6, &start_buf[0], 8); // передаем команду
		HAL_Delay(1000); // ждем

	}
	else if( (meter_id>9)&&(meter_id<100) )
	{
		start_buf[0] = 0x2F;
		start_buf[1] = 0x3F;
		start_buf[2] = (meter_id/10) + 0x30;
		start_buf[3] = (meter_id - ((meter_id/10)*10)) + 0x30;
		start_buf[4] = 0x21;
		start_buf[5] = 0x0D;
		start_buf[6] = 0x0A;

		meter_rx_number = 0;
		meter_rx_buffer_clear(); // очищаем буфер приема счетчика
		HAL_GPIO_WritePin(GPIOG, GPIO_PIN_10, GPIO_PIN_SET); // устанавливаем микросхему на передачу
		HAL_UART_Transmit_DMA(&huart6, &start_buf[0], 7); // передаем команду
		HAL_Delay(1000); // ждем

	}
	else if( (meter_id>=0)&&(meter_id<10) )
	{
		start_buf[0] = 0x2F;
		start_buf[1] = 0x3F;
		start_buf[2] = meter_id + 0x30;
		start_buf[3] = 0x21;
		start_buf[4] = 0x0D;
		start_buf[5] = 0x0A;

		meter_rx_number = 0;
		meter_rx_buffer_clear(); // очищаем буфер приема счетчика
		HAL_GPIO_WritePin(GPIOG, GPIO_PIN_10, GPIO_PIN_SET); // устанавливаем микросхему на передачу
		HAL_UART_Transmit_DMA(&huart6, &start_buf[0], 6); // передаем команду
		HAL_Delay(1000); // ждем
	}


	if( (meter_id>99)&&(meter_id<1000) )
	{
		start_buf[0] = 0x2F;
		start_buf[1] = 0x3F;
		start_buf[2] = (meter_id/100) + 0x30;
		start_buf[3] = ((meter_id - (meter_id/100)*100)/10) + 0x30;
		start_buf[4] = ((meter_id - ((meter_id/100)*100)) - (((meter_id - ((meter_id/100)*100))/10)*10)) + 0x30 ;
		start_buf[5] = 0x21;
		start_buf[6] = 0x0D;
		start_buf[7] = 0x0A;

		meter_rx_number = 0;
		meter_rx_buffer_clear(); // очищаем буфер приема счетчика
		HAL_GPIO_WritePin(GPIOG, GPIO_PIN_10, GPIO_PIN_SET); // устанавливаем микросхему на передачу
		HAL_UART_Transmit_DMA(&huart6, &start_buf[0], 8); // передаем команду
		HAL_Delay(1000); // ждем

	}
	else if( (meter_id>9)&&(meter_id<100) )
	{
		start_buf[0] = 0x2F;
		start_buf[1] = 0x3F;
		start_buf[2] = (meter_id/10) + 0x30;
		start_buf[3] = (meter_id - ((meter_id/10)*10)) + 0x30;
		start_buf[4] = 0x21;
		start_buf[5] = 0x0D;
		start_buf[6] = 0x0A;

		meter_rx_number = 0;
		meter_rx_buffer_clear(); // очищаем буфер приема счетчика
		HAL_GPIO_WritePin(GPIOG, GPIO_PIN_10, GPIO_PIN_SET); // устанавливаем микросхему на передачу
		HAL_UART_Transmit_DMA(&huart6, &start_buf[0], 7); // передаем команду
		HAL_Delay(1000); // ждем

	}
	else if( (meter_id>=0)&&(meter_id<10) )
	{
		start_buf[0] = 0x2F;
		start_buf[1] = 0x3F;
		start_buf[2] = meter_id + 0x30;
		start_buf[3] = 0x21;
		start_buf[4] = 0x0D;
		start_buf[5] = 0x0A;

		meter_rx_number = 0;
		meter_rx_buffer_clear(); // очищаем буфер приема счетчика
		HAL_GPIO_WritePin(GPIOG, GPIO_PIN_10, GPIO_PIN_SET); // устанавливаем микросхему на передачу
		HAL_UART_Transmit_DMA(&huart6, &start_buf[0], 6); // передаем команду
		HAL_Delay(1000); // ждем
	}



	meter_rx_number = 0;
	meter_rx_buffer_clear(); // очищаем буфер приема счетчика
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_10, GPIO_PIN_SET); // устанавливаем микросхему на передачу
	HAL_UART_Transmit_DMA(&huart6, &password_buf[0], 6); // передаем команду
	HAL_Delay(1000);

	meter_rx_number = 0;
	meter_rx_buffer_clear(); // очищаем буфер приема счетчика
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_10, GPIO_PIN_SET); // устанавливаем микросхему на передачу
	HAL_UART_Transmit_DMA(&huart6, &voltage_buf[0], 13); // передаем команду
	HAL_Delay(1000);

	voltage_a_state = 0; // обнуляем переменные состояния прочтения напряжения перед их прочтением
	voltage_b_state = 0;
	voltage_c_state = 0;

	for(uint16_t i=0; i<256; i++)
	{

		if( (meter_rx_buffer[i] == 0x28) && (voltage_a_state==0) && (voltage_b_state==0) && (voltage_c_state==0) ) // 0х28 означает символ "(" в ASCII таблице
		{
			if( meter_rx_buffer[i+2] == 0x2E) // 0х2Е означает символ "." в ASCII таблице
			{
				temp = (uint16_t)(meter_rx_buffer[i+1]-0x30);

				if( meter_rx_buffer[i+4] == 0x29 ) // если 1 разряд после запятой
				{
					temp_mil = (((uint16_t)meter_rx_buffer[i+3])-0x30)*100;
				}
				else if( meter_rx_buffer[i+5] == 0x29 ) // если 2 разряда после запятой
				{
					temp_mil = (((((uint16_t)meter_rx_buffer[i+3])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+4])-0x30))*10;
				}
				else if( meter_rx_buffer[i+6] == 0x29 ) // если 3 разряда после запятой
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+3])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+4])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+5])-0x30);
				}
				//if( meter_rx_buffer[i+7] == 0x29 ) // если 4 разряда после запятой
				//{
					//temp_mil = ((((uint16_t)meter_rx_buffer[i+3])-0x30)*1000) + ((((uint16_t)meter_rx_buffer[i+4])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+5])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+6])-0x30);
				//}
				else
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+3])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+4])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+5])-0x30);
				}

				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*CE_303_VOLT_A_REG, ((uint8_t)(temp>>8)) );
				fm25v02_write(2*CE_303_VOLT_A_REG+1, ((uint8_t)(temp&0x00FF)) );
				fm25v02_write(2*CE_303_VOLT_MIL_A_REG, ((uint8_t)(temp_mil>>8)) );
				fm25v02_write(2*CE_303_VOLT_MIL_A_REG+1, ((uint8_t)(temp_mil&0x00FF)) );
				osMutexRelease(Fm25v02MutexHandle);
				status_registers.ce303_volt_a_reg = temp;
				status_registers.ce303_volt_mil_a_reg =  temp_mil;
				voltage_a_state = 1;
			}
			else if( meter_rx_buffer[i+3] == 0x2E)
			{
				temp = ((((uint16_t)meter_rx_buffer[i+1])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+2])-0x30);

				if( meter_rx_buffer[i+5] == 0x29 ) // если 1 разряд после запятой
				{
					temp_mil = (((uint16_t)meter_rx_buffer[i+4])-0x30)*100;
				}
				else if( meter_rx_buffer[i+6] == 0x29 ) // если 2 разряда после запятой
				{
					temp_mil = (((((uint16_t)meter_rx_buffer[i+4])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+5])-0x30))*10;
				}
				else if( meter_rx_buffer[i+7] == 0x29 ) // если 3 разряда после запятой
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+4])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+5])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+6])-0x30);
				}
				//if( meter_rx_buffer[i+8] == 0x29 ) // если 4 разряда после запятой
				//{
					//temp_mil = ((((uint16_t)meter_rx_buffer[i+4])-0x30)*1000) + ((((uint16_t)meter_rx_buffer[i+5])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+6])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+7])-0x30);
				//}
				else
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+4])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+5])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+6])-0x30);
				}

				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*CE_303_VOLT_A_REG, ((uint8_t)(temp>>8)) );
				fm25v02_write(2*CE_303_VOLT_A_REG+1, ((uint8_t)(temp&0x00FF)) );
				fm25v02_write(2*CE_303_VOLT_MIL_A_REG, ((uint8_t)(temp_mil>>8)) );
				fm25v02_write(2*CE_303_VOLT_MIL_A_REG+1, ((uint8_t)(temp_mil&0x00FF)) );
				osMutexRelease(Fm25v02MutexHandle);
				status_registers.ce303_volt_a_reg = temp;
				status_registers.ce303_volt_mil_a_reg = temp_mil;
				voltage_a_state = 1;
			}

			else if( meter_rx_buffer[i+4] == 0x2E)
			{
				temp = ((((uint16_t)meter_rx_buffer[i+1])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+2])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+3])-0x30);

				if( meter_rx_buffer[i+6] == 0x29 ) // если 1 разряд после запятой
				{
					temp_mil = (((uint16_t)meter_rx_buffer[i+5])-0x30)*100;
				}
				else if( meter_rx_buffer[i+7] == 0x29 ) // если 2 разряда после запятой
				{
					temp_mil = (((((uint16_t)meter_rx_buffer[i+5])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+6])-0x30))*10;
				}
				else if( meter_rx_buffer[i+8] == 0x29 ) // если 3 разряда после запятой
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+5])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+6])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+7])-0x30);
				}
				//if( meter_rx_buffer[i+9] == 0x29 ) // если 4 разряда после запятой
				//{
					//temp_mil = ((((uint16_t)meter_rx_buffer[i+5])-0x30)*1000) + ((((uint16_t)meter_rx_buffer[i+6])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+7])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+8])-0x30);
				//}
				else
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+5])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+6])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+7])-0x30);
				}

				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*CE_303_VOLT_A_REG, ((uint8_t)(temp>>8)) );
				fm25v02_write(2*CE_303_VOLT_A_REG+1, ((uint8_t)(temp&0x00FF)) );
				fm25v02_write(2*CE_303_VOLT_MIL_A_REG, ((uint8_t)(temp_mil>>8)) );
				fm25v02_write(2*CE_303_VOLT_MIL_A_REG+1, ((uint8_t)(temp_mil&0x00FF)) );
				osMutexRelease(Fm25v02MutexHandle);
				status_registers.ce303_volt_a_reg = temp;
				status_registers.ce303_volt_mil_a_reg = temp_mil;
				voltage_a_state = 1;
			}
		}

		else if( (meter_rx_buffer[i] == 0x28) && (voltage_a_state==1) && (voltage_b_state==0) && (voltage_c_state==0) )
		{
			if( meter_rx_buffer[i+2] == 0x2E) // 0х2Е означает символ "." в ASCII таблице
			{
				temp = (uint16_t)(meter_rx_buffer[i+1]-0x30);

				if( meter_rx_buffer[i+4] == 0x29 ) // если 1 разряд после запятой
				{
					temp_mil = (((uint16_t)meter_rx_buffer[i+3])-0x30)*100;
				}
				else if( meter_rx_buffer[i+5] == 0x29 ) // если 2 разряда после запятой
				{
					temp_mil = (((((uint16_t)meter_rx_buffer[i+3])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+4])-0x30))*10;
				}
				else if( meter_rx_buffer[i+6] == 0x29 ) // если 3 разряда после запятой
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+3])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+4])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+5])-0x30);
				}
				//if( meter_rx_buffer[i+7] == 0x29 ) // если 4 разряда после запятой
				//{
					//temp_mil = ((((uint16_t)meter_rx_buffer[i+3])-0x30)*1000) + ((((uint16_t)meter_rx_buffer[i+4])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+5])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+6])-0x30);
				//}
				else
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+3])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+4])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+5])-0x30);
				}

				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*CE_303_VOLT_B_REG, ((uint8_t)(temp>>8)) );
				fm25v02_write(2*CE_303_VOLT_B_REG+1, ((uint8_t)(temp&0x00FF)) );
				fm25v02_write(2*CE_303_VOLT_MIL_B_REG, ((uint8_t)(temp_mil>>8)) );
				fm25v02_write(2*CE_303_VOLT_MIL_B_REG+1, ((uint8_t)(temp_mil&0x00FF)) );
				osMutexRelease(Fm25v02MutexHandle);
				status_registers.ce303_volt_b_reg = temp;
				status_registers.ce303_volt_mil_b_reg =  temp_mil;
				voltage_b_state = 1;
			}
			else if( meter_rx_buffer[i+3] == 0x2E)
			{
				temp = ((((uint16_t)meter_rx_buffer[i+1])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+2])-0x30);

				if( meter_rx_buffer[i+5] == 0x29 ) // если 1 разряд после запятой
				{
					temp_mil = (((uint16_t)meter_rx_buffer[i+4])-0x30)*100;
				}
				else if( meter_rx_buffer[i+6] == 0x29 ) // если 2 разряда после запятой
				{
					temp_mil = (((((uint16_t)meter_rx_buffer[i+4])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+5])-0x30))*10;
				}
				else if( meter_rx_buffer[i+7] == 0x29 ) // если 3 разряда после запятой
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+4])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+5])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+6])-0x30);
				}
				//if( meter_rx_buffer[i+8] == 0x29 ) // если 4 разряда после запятой
				//{
					//temp_mil = ((((uint16_t)meter_rx_buffer[i+4])-0x30)*1000) + ((((uint16_t)meter_rx_buffer[i+5])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+6])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+7])-0x30);
				//}
				else
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+4])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+5])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+6])-0x30);
				}

				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*CE_303_VOLT_B_REG, ((uint8_t)(temp>>8)) );
				fm25v02_write(2*CE_303_VOLT_B_REG+1, ((uint8_t)(temp&0x00FF)) );
				fm25v02_write(2*CE_303_VOLT_MIL_B_REG, ((uint8_t)(temp_mil>>8)) );
				fm25v02_write(2*CE_303_VOLT_MIL_B_REG+1, ((uint8_t)(temp_mil&0x00FF)) );
				osMutexRelease(Fm25v02MutexHandle);
				status_registers.ce303_volt_b_reg = temp;
				status_registers.ce303_volt_mil_b_reg = temp_mil;
				voltage_b_state = 1;
			}

			else if( meter_rx_buffer[i+4] == 0x2E)
			{
				temp = ((((uint16_t)meter_rx_buffer[i+1])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+2])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+3])-0x30);

				if( meter_rx_buffer[i+6] == 0x29 ) // если 1 разряд после запятой
				{
					temp_mil = (((uint16_t)meter_rx_buffer[i+5])-0x30)*100;
				}
				else if( meter_rx_buffer[i+7] == 0x29 ) // если 2 разряда после запятой
				{
					temp_mil = (((((uint16_t)meter_rx_buffer[i+5])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+6])-0x30))*10;
				}
				else if( meter_rx_buffer[i+8] == 0x29 ) // если 3 разряда после запятой
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+5])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+6])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+7])-0x30);
				}
				//if( meter_rx_buffer[i+9] == 0x29 ) // если 4 разряда после запятой
				//{
					//temp_mil = ((((uint16_t)meter_rx_buffer[i+5])-0x30)*1000) + ((((uint16_t)meter_rx_buffer[i+6])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+7])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+8])-0x30);
				//}
				else
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+5])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+6])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+7])-0x30);
				}

				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*CE_303_VOLT_B_REG, ((uint8_t)(temp>>8)) );
				fm25v02_write(2*CE_303_VOLT_B_REG+1, ((uint8_t)(temp&0x00FF)) );
				fm25v02_write(2*CE_303_VOLT_MIL_B_REG, ((uint8_t)(temp_mil>>8)) );
				fm25v02_write(2*CE_303_VOLT_MIL_B_REG+1, ((uint8_t)(temp_mil&0x00FF)) );
				osMutexRelease(Fm25v02MutexHandle);
				status_registers.ce303_volt_b_reg = temp;
				status_registers.ce303_volt_mil_b_reg = temp_mil;
				voltage_b_state = 1;
			}
		}

		else if( (meter_rx_buffer[i] == 0x28) && (voltage_a_state==1) && (voltage_b_state==1) && (voltage_c_state==0) )
		{
			if( meter_rx_buffer[i+2] == 0x2E) // 0х2Е означает символ "." в ASCII таблице
			{
				temp = (uint16_t)(meter_rx_buffer[i+1]-0x30);

				if( meter_rx_buffer[i+4] == 0x29 ) // если 1 разряд после запятой
				{
					temp_mil = (((uint16_t)meter_rx_buffer[i+3])-0x30)*100;
				}
				else if( meter_rx_buffer[i+5] == 0x29 ) // если 2 разряда после запятой
				{
					temp_mil = (((((uint16_t)meter_rx_buffer[i+3])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+4])-0x30))*10;
				}
				else if( meter_rx_buffer[i+6] == 0x29 ) // если 3 разряда после запятой
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+3])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+4])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+5])-0x30);
				}
				//if( meter_rx_buffer[i+7] == 0x29 ) // если 4 разряда после запятой
				//{
					//temp_mil = ((((uint16_t)meter_rx_buffer[i+3])-0x30)*1000) + ((((uint16_t)meter_rx_buffer[i+4])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+5])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+6])-0x30);
				//}
				else
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+3])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+4])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+5])-0x30);
				}

				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*CE_303_VOLT_C_REG, ((uint8_t)(temp>>8)) );
				fm25v02_write(2*CE_303_VOLT_C_REG+1, ((uint8_t)(temp&0x00FF)) );
				fm25v02_write(2*CE_303_VOLT_MIL_C_REG, ((uint8_t)(temp_mil>>8)) );
				fm25v02_write(2*CE_303_VOLT_MIL_C_REG+1, ((uint8_t)(temp_mil&0x00FF)) );
				osMutexRelease(Fm25v02MutexHandle);
				status_registers.ce303_volt_c_reg = temp;
				status_registers.ce303_volt_mil_c_reg = temp_mil;
				voltage_c_state = 1;
			}
			else if( meter_rx_buffer[i+3] == 0x2E)
			{
				temp = ((((uint16_t)meter_rx_buffer[i+1])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+2])-0x30);

				if( meter_rx_buffer[i+5] == 0x29 ) // если 1 разряд после запятой
				{
					temp_mil = (((uint16_t)meter_rx_buffer[i+4])-0x30)*100;
				}
				else if( meter_rx_buffer[i+6] == 0x29 ) // если 2 разряда после запятой
				{
					temp_mil = (((((uint16_t)meter_rx_buffer[i+4])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+5])-0x30))*10;
				}
				else if( meter_rx_buffer[i+7] == 0x29 ) // если 3 разряда после запятой
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+4])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+5])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+6])-0x30);
				}
				//if( meter_rx_buffer[i+8] == 0x29 ) // если 4 разряда после запятой
				//{
					//temp_mil = ((((uint16_t)meter_rx_buffer[i+4])-0x30)*1000) + ((((uint16_t)meter_rx_buffer[i+5])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+6])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+7])-0x30);
				//}
				else
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+4])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+5])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+6])-0x30);
				}

				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*CE_303_VOLT_C_REG, ((uint8_t)(temp>>8)) );
				fm25v02_write(2*CE_303_VOLT_C_REG+1, ((uint8_t)(temp&0x00FF)) );
				fm25v02_write(2*CE_303_VOLT_MIL_C_REG, ((uint8_t)(temp_mil>>8)) );
				fm25v02_write(2*CE_303_VOLT_MIL_C_REG+1, ((uint8_t)(temp_mil&0x00FF)) );
				osMutexRelease(Fm25v02MutexHandle);
				status_registers.ce303_volt_c_reg = temp;
				status_registers.ce303_volt_mil_c_reg = temp_mil;
				voltage_c_state = 1;
			}

			else if( meter_rx_buffer[i+4] == 0x2E)
			{
				temp = ((((uint16_t)meter_rx_buffer[i+1])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+2])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+3])-0x30);

				if( meter_rx_buffer[i+6] == 0x29 ) // если 1 разряд после запятой
				{
					temp_mil = (((uint16_t)meter_rx_buffer[i+5])-0x30)*100;
				}
				else if( meter_rx_buffer[i+7] == 0x29 ) // если 2 разряда после запятой
				{
					temp_mil = (((((uint16_t)meter_rx_buffer[i+5])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+6])-0x30))*10;
				}
				else if( meter_rx_buffer[i+8] == 0x29 ) // если 3 разряда после запятой
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+5])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+6])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+7])-0x30);
				}
				//if( meter_rx_buffer[i+9] == 0x29 ) // если 4 разряда после запятой
				//{
					//temp_mil = ((((uint16_t)meter_rx_buffer[i+5])-0x30)*1000) + ((((uint16_t)meter_rx_buffer[i+6])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+7])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+8])-0x30);
				//}
				else
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+5])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+6])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+7])-0x30);
				}

				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*CE_303_VOLT_C_REG, ((uint8_t)(temp>>8)) );
				fm25v02_write(2*CE_303_VOLT_C_REG+1, ((uint8_t)(temp&0x00FF)) );
				fm25v02_write(2*CE_303_VOLT_MIL_C_REG, ((uint8_t)(temp_mil>>8)) );
				fm25v02_write(2*CE_303_VOLT_MIL_C_REG+1, ((uint8_t)(temp_mil&0x00FF)) );
				osMutexRelease(Fm25v02MutexHandle);
				status_registers.ce303_volt_c_reg = temp;
				status_registers.ce303_volt_mil_c_reg = temp_mil;
				voltage_c_state = 1;
			}
		}

	}

	meter_rx_number = 0;
	meter_rx_buffer_clear(); // очищаем буфер приема счетчика
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_10, GPIO_PIN_SET); // устанавливаем микросхему на передачу
	HAL_UART_Transmit_DMA(&huart6, &end_buf[0], 5); // передаем команду
	HAL_Delay(1000);

}

void ce303_current (uint8_t meter_id) // функция чтения тока из счетчика СЕ303
{
	uint16_t temp;
	uint16_t temp_mil;
	uint8_t current_a_state; // переменные состояния прочтения значений тока ( если переменная равна 0, то значение тока не прочитано, если 1 - значение прочитанно )
	uint8_t current_b_state;
	uint8_t current_c_state;

	if( (meter_id>99)&&(meter_id<1000) )
	{
		start_buf[0] = 0x2F;
		start_buf[1] = 0x3F;
		start_buf[2] = (meter_id/100) + 0x30;
		start_buf[3] = ((meter_id - (meter_id/100)*100)/10) + 0x30;
		start_buf[4] = ((meter_id - ((meter_id/100)*100)) - (((meter_id - ((meter_id/100)*100))/10)*10)) + 0x30 ;
		start_buf[5] = 0x21;
		start_buf[6] = 0x0D;
		start_buf[7] = 0x0A;

		meter_rx_number = 0;
		meter_rx_buffer_clear(); // очищаем буфер приема счетчика
		HAL_GPIO_WritePin(GPIOG, GPIO_PIN_10, GPIO_PIN_SET); // устанавливаем микросхему на передачу
		HAL_UART_Transmit_DMA(&huart6, &start_buf[0], 8); // передаем команду
		HAL_Delay(1000); // ждем

	}
	else if( (meter_id>9)&&(meter_id<100) )
	{
		start_buf[0] = 0x2F;
		start_buf[1] = 0x3F;
		start_buf[2] = (meter_id/10) + 0x30;
		start_buf[3] = (meter_id - ((meter_id/10)*10)) + 0x30;
		start_buf[4] = 0x21;
		start_buf[5] = 0x0D;
		start_buf[6] = 0x0A;

		meter_rx_number = 0;
		meter_rx_buffer_clear(); // очищаем буфер приема счетчика
		HAL_GPIO_WritePin(GPIOG, GPIO_PIN_10, GPIO_PIN_SET); // устанавливаем микросхему на передачу
		HAL_UART_Transmit_DMA(&huart6, &start_buf[0], 7); // передаем команду
		HAL_Delay(1000); // ждем

	}
	else if( (meter_id>=0)&&(meter_id<10) )
	{
		start_buf[0] = 0x2F;
		start_buf[1] = 0x3F;
		start_buf[2] = meter_id + 0x30;
		start_buf[3] = 0x21;
		start_buf[4] = 0x0D;
		start_buf[5] = 0x0A;

		meter_rx_number = 0;
		meter_rx_buffer_clear(); // очищаем буфер приема счетчика
		HAL_GPIO_WritePin(GPIOG, GPIO_PIN_10, GPIO_PIN_SET); // устанавливаем микросхему на передачу
		HAL_UART_Transmit_DMA(&huart6, &start_buf[0], 6); // передаем команду
		HAL_Delay(1000); // ждем
	}


	if( (meter_id>99)&&(meter_id<1000) )
	{
		start_buf[0] = 0x2F;
		start_buf[1] = 0x3F;
		start_buf[2] = (meter_id/100) + 0x30;
		start_buf[3] = ((meter_id - (meter_id/100)*100)/10) + 0x30;
		start_buf[4] = ((meter_id - ((meter_id/100)*100)) - (((meter_id - ((meter_id/100)*100))/10)*10)) + 0x30 ;
		start_buf[5] = 0x21;
		start_buf[6] = 0x0D;
		start_buf[7] = 0x0A;

		meter_rx_number = 0;
		meter_rx_buffer_clear(); // очищаем буфер приема счетчика
		HAL_GPIO_WritePin(GPIOG, GPIO_PIN_10, GPIO_PIN_SET); // устанавливаем микросхему на передачу
		HAL_UART_Transmit_DMA(&huart6, &start_buf[0], 8); // передаем команду
		HAL_Delay(1000); // ждем

	}
	else if( (meter_id>9)&&(meter_id<100) )
	{
		start_buf[0] = 0x2F;
		start_buf[1] = 0x3F;
		start_buf[2] = (meter_id/10) + 0x30;
		start_buf[3] = (meter_id - ((meter_id/10)*10)) + 0x30;
		start_buf[4] = 0x21;
		start_buf[5] = 0x0D;
		start_buf[6] = 0x0A;

		meter_rx_number = 0;
		meter_rx_buffer_clear(); // очищаем буфер приема счетчика
		HAL_GPIO_WritePin(GPIOG, GPIO_PIN_10, GPIO_PIN_SET); // устанавливаем микросхему на передачу
		HAL_UART_Transmit_DMA(&huart6, &start_buf[0], 7); // передаем команду
		HAL_Delay(1000); // ждем

	}
	else if( (meter_id>=0)&&(meter_id<10) )
	{
		start_buf[0] = 0x2F;
		start_buf[1] = 0x3F;
		start_buf[2] = meter_id + 0x30;
		start_buf[3] = 0x21;
		start_buf[4] = 0x0D;
		start_buf[5] = 0x0A;

		meter_rx_number = 0;
		meter_rx_buffer_clear(); // очищаем буфер приема счетчика
		HAL_GPIO_WritePin(GPIOG, GPIO_PIN_10, GPIO_PIN_SET); // устанавливаем микросхему на передачу
		HAL_UART_Transmit_DMA(&huart6, &start_buf[0], 6); // передаем команду
		HAL_Delay(1000); // ждем
	}


	/*
	meter_rx_number = 0;
	meter_rx_buffer_clear(); // очищаем буфер приема счетчика
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_10, GPIO_PIN_SET); // устанавливаем микросхему на передачу
	HAL_UART_Transmit_DMA(&huart6, &start_buf[0], 6); // передаем команду
	HAL_Delay(1000); // ждем

	meter_rx_number = 0;
	meter_rx_buffer_clear(); // очищаем буфер приема счетчика
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_10, GPIO_PIN_SET); // устанавливаем микросхему на передачу
	HAL_UART_Transmit_DMA(&huart6, &start_buf[0], 6); // передаем команду
	HAL_Delay(1000); // ждем
	*/

	meter_rx_number = 0;
	meter_rx_buffer_clear(); // очищаем буфер приема счетчика
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_10, GPIO_PIN_SET); // устанавливаем микросхему на передачу
	HAL_UART_Transmit_DMA(&huart6, &password_buf[0], 6); // передаем команду
	HAL_Delay(1000);

	meter_rx_number = 0;
	meter_rx_buffer_clear(); // очищаем буфер приема счетчика
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_10, GPIO_PIN_SET); // устанавливаем микросхему на передачу
	HAL_UART_Transmit_DMA(&huart6, &current_buf[0], 13); // передаем команду
	HAL_Delay(1000);

	current_a_state = 0; // обнуляем переменные состояния прочтения тока перед их прочтением
	current_b_state = 0;
	current_c_state = 0;

	for(uint16_t i=0; i<256; i++)
	{

		if( (meter_rx_buffer[i] == 0x28) && (current_a_state==0) && (current_b_state==0) && (current_c_state==0) ) // 0х28 означает символ "(" в ASCII таблице
		{
			if( meter_rx_buffer[i+2] == 0x2E) // 0х2Е означает символ "." в ASCII таблице
			{
				temp = (uint16_t)(meter_rx_buffer[i+1]-0x30);

				if( meter_rx_buffer[i+4] == 0x29 ) // если 1 разряд после запятой
				{
					temp_mil = (((uint16_t)meter_rx_buffer[i+3])-0x30)*100;
				}
				else if( meter_rx_buffer[i+5] == 0x29 ) // если 2 разряда после запятой
				{
					temp_mil = (((((uint16_t)meter_rx_buffer[i+3])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+4])-0x30))*10;
				}
				else if( meter_rx_buffer[i+6] == 0x29 ) // если 3 разряда после запятой
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+3])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+4])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+5])-0x30);
				}
				//if( meter_rx_buffer[i+7] == 0x29 ) // если 4 разряда после запятой
				//{
					//temp_mil = ((((uint16_t)meter_rx_buffer[i+3])-0x30)*1000) + ((((uint16_t)meter_rx_buffer[i+4])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+5])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+6])-0x30);
				//}
				else
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+3])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+4])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+5])-0x30);
				}

				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*CE_303_CURRENT_A_REG, ((uint8_t)(temp>>8)) );
				fm25v02_write(2*CE_303_CURRENT_A_REG+1, ((uint8_t)(temp&0x00FF)) );
				fm25v02_write(2*CE_303_CURRENT_MIL_A_REG, ((uint8_t)(temp_mil>>8)) );
				fm25v02_write(2*CE_303_CURRENT_MIL_A_REG+1, ((uint8_t)(temp_mil&0x00FF)) );
				osMutexRelease(Fm25v02MutexHandle);
				status_registers.ce303_current_a_reg = temp;
				status_registers.ce303_current_mil_a_reg =  temp_mil;
				current_a_state = 1;
			}
			else if( meter_rx_buffer[i+3] == 0x2E)
			{
				temp = ((((uint16_t)meter_rx_buffer[i+1])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+2])-0x30);

				if( meter_rx_buffer[i+5] == 0x29 ) // если 1 разряд после запятой
				{
					temp_mil = (((uint16_t)meter_rx_buffer[i+4])-0x30)*100;
				}
				else if( meter_rx_buffer[i+6] == 0x29 ) // если 2 разряда после запятой
				{
					temp_mil = (((((uint16_t)meter_rx_buffer[i+4])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+5])-0x30))*10;
				}
				else if( meter_rx_buffer[i+7] == 0x29 ) // если 3 разряда после запятой
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+4])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+5])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+6])-0x30);
				}
				//if( meter_rx_buffer[i+8] == 0x29 ) // если 4 разряда после запятой
				//{
					//temp_mil = ((((uint16_t)meter_rx_buffer[i+4])-0x30)*1000) + ((((uint16_t)meter_rx_buffer[i+5])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+6])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+7])-0x30);
				//}
				else
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+4])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+5])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+6])-0x30);
				}

				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*CE_303_CURRENT_A_REG, ((uint8_t)(temp>>8)) );
				fm25v02_write(2*CE_303_CURRENT_A_REG+1, ((uint8_t)(temp&0x00FF)) );
				fm25v02_write(2*CE_303_CURRENT_MIL_A_REG, ((uint8_t)(temp_mil>>8)) );
				fm25v02_write(2*CE_303_CURRENT_MIL_A_REG+1, ((uint8_t)(temp_mil&0x00FF)) );
				osMutexRelease(Fm25v02MutexHandle);
				status_registers.ce303_current_a_reg = temp;
				status_registers.ce303_current_mil_a_reg = temp_mil;
				current_a_state = 1;
			}

			else if( meter_rx_buffer[i+4] == 0x2E)
			{
				temp = ((((uint16_t)meter_rx_buffer[i+1])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+2])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+3])-0x30);

				if( meter_rx_buffer[i+6] == 0x29 ) // если 1 разряд после запятой
				{
					temp_mil = (((uint16_t)meter_rx_buffer[i+5])-0x30)*100;
				}
				else if( meter_rx_buffer[i+7] == 0x29 ) // если 2 разряда после запятой
				{
					temp_mil = (((((uint16_t)meter_rx_buffer[i+5])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+6])-0x30))*10;
				}
				else if( meter_rx_buffer[i+8] == 0x29 ) // если 3 разряда после запятой
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+5])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+6])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+7])-0x30);
				}
				//if( meter_rx_buffer[i+9] == 0x29 ) // если 4 разряда после запятой
				//{
					//temp_mil = ((((uint16_t)meter_rx_buffer[i+5])-0x30)*1000) + ((((uint16_t)meter_rx_buffer[i+6])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+7])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+8])-0x30);
				//}
				else
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+5])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+6])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+7])-0x30);
				}

				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*CE_303_CURRENT_A_REG, ((uint8_t)(temp>>8)) );
				fm25v02_write(2*CE_303_CURRENT_A_REG+1, ((uint8_t)(temp&0x00FF)) );
				fm25v02_write(2*CE_303_CURRENT_MIL_A_REG, ((uint8_t)(temp_mil>>8)) );
				fm25v02_write(2*CE_303_CURRENT_MIL_A_REG+1, ((uint8_t)(temp_mil&0x00FF)) );
				osMutexRelease(Fm25v02MutexHandle);
				status_registers.ce303_current_a_reg = temp;
				status_registers.ce303_current_mil_a_reg = temp_mil;
				current_a_state = 1;
			}
		}

		else if( (meter_rx_buffer[i] == 0x28) && (current_a_state==1) && (current_b_state==0) && (current_c_state==0) )
		{
			if( meter_rx_buffer[i+2] == 0x2E) // 0х2Е означает символ "." в ASCII таблице
			{
				temp = (uint16_t)(meter_rx_buffer[i+1]-0x30);

				if( meter_rx_buffer[i+4] == 0x29 ) // если 1 разряд после запятой
				{
					temp_mil = (((uint16_t)meter_rx_buffer[i+3])-0x30)*100;
				}
				else if( meter_rx_buffer[i+5] == 0x29 ) // если 2 разряда после запятой
				{
					temp_mil = (((((uint16_t)meter_rx_buffer[i+3])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+4])-0x30))*10;
				}
				else if( meter_rx_buffer[i+6] == 0x29 ) // если 3 разряда после запятой
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+3])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+4])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+5])-0x30);
				}
				//if( meter_rx_buffer[i+7] == 0x29 ) // если 4 разряда после запятой
				//{
					//temp_mil = ((((uint16_t)meter_rx_buffer[i+3])-0x30)*1000) + ((((uint16_t)meter_rx_buffer[i+4])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+5])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+6])-0x30);
				//}
				else
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+3])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+4])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+5])-0x30);
				}

				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*CE_303_CURRENT_B_REG, ((uint8_t)(temp>>8)) );
				fm25v02_write(2*CE_303_CURRENT_B_REG+1, ((uint8_t)(temp&0x00FF)) );
				fm25v02_write(2*CE_303_CURRENT_MIL_B_REG, ((uint8_t)(temp_mil>>8)) );
				fm25v02_write(2*CE_303_CURRENT_MIL_B_REG+1, ((uint8_t)(temp_mil&0x00FF)) );
				osMutexRelease(Fm25v02MutexHandle);
				status_registers.ce303_current_b_reg = temp;
				status_registers.ce303_current_mil_b_reg =  temp_mil;
				current_b_state = 1;
			}
			else if( meter_rx_buffer[i+3] == 0x2E)
			{
				temp = ((((uint16_t)meter_rx_buffer[i+1])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+2])-0x30);

				if( meter_rx_buffer[i+5] == 0x29 ) // если 1 разряд после запятой
				{
					temp_mil = (((uint16_t)meter_rx_buffer[i+4])-0x30)*100;
				}
				else if( meter_rx_buffer[i+6] == 0x29 ) // если 2 разряда после запятой
				{
					temp_mil = (((((uint16_t)meter_rx_buffer[i+4])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+5])-0x30))*10;
				}
				else if( meter_rx_buffer[i+7] == 0x29 ) // если 3 разряда после запятой
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+4])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+5])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+6])-0x30);
				}
				//if( meter_rx_buffer[i+8] == 0x29 ) // если 4 разряда после запятой
				//{
					//temp_mil = ((((uint16_t)meter_rx_buffer[i+4])-0x30)*1000) + ((((uint16_t)meter_rx_buffer[i+5])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+6])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+7])-0x30);
				//}
				else
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+4])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+5])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+6])-0x30);
				}

				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*CE_303_CURRENT_B_REG, ((uint8_t)(temp>>8)) );
				fm25v02_write(2*CE_303_CURRENT_B_REG+1, ((uint8_t)(temp&0x00FF)) );
				fm25v02_write(2*CE_303_CURRENT_MIL_B_REG, ((uint8_t)(temp_mil>>8)) );
				fm25v02_write(2*CE_303_CURRENT_MIL_B_REG+1, ((uint8_t)(temp_mil&0x00FF)) );
				osMutexRelease(Fm25v02MutexHandle);
				status_registers.ce303_current_b_reg = temp;
				status_registers.ce303_current_mil_b_reg = temp_mil;
				current_b_state = 1;
			}

			else if( meter_rx_buffer[i+4] == 0x2E)
			{
				temp = ((((uint16_t)meter_rx_buffer[i+1])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+2])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+3])-0x30);

				if( meter_rx_buffer[i+6] == 0x29 ) // если 1 разряд после запятой
				{
					temp_mil = (((uint16_t)meter_rx_buffer[i+5])-0x30)*100;
				}
				else if( meter_rx_buffer[i+7] == 0x29 ) // если 2 разряда после запятой
				{
					temp_mil = (((((uint16_t)meter_rx_buffer[i+5])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+6])-0x30))*10;
				}
				else if( meter_rx_buffer[i+8] == 0x29 ) // если 3 разряда после запятой
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+5])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+6])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+7])-0x30);
				}
				//if( meter_rx_buffer[i+9] == 0x29 ) // если 4 разряда после запятой
				//{
					//temp_mil = ((((uint16_t)meter_rx_buffer[i+5])-0x30)*1000) + ((((uint16_t)meter_rx_buffer[i+6])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+7])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+8])-0x30);
				//}
				else
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+5])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+6])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+7])-0x30);
				}

				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*CE_303_CURRENT_B_REG, ((uint8_t)(temp>>8)) );
				fm25v02_write(2*CE_303_CURRENT_B_REG+1, ((uint8_t)(temp&0x00FF)) );
				fm25v02_write(2*CE_303_CURRENT_MIL_B_REG, ((uint8_t)(temp_mil>>8)) );
				fm25v02_write(2*CE_303_CURRENT_MIL_B_REG+1, ((uint8_t)(temp_mil&0x00FF)) );
				osMutexRelease(Fm25v02MutexHandle);
				status_registers.ce303_current_b_reg = temp;
				status_registers.ce303_current_mil_b_reg = temp_mil;
				current_b_state = 1;
			}
		}

		else if( (meter_rx_buffer[i] == 0x28) && (current_a_state==1) && (current_b_state==1) && (current_c_state==0) )
		{
			if( meter_rx_buffer[i+2] == 0x2E) // 0х2Е означает символ "." в ASCII таблице
			{
				temp = (uint16_t)(meter_rx_buffer[i+1]-0x30);

				if( meter_rx_buffer[i+4] == 0x29 ) // если 1 разряд после запятой
				{
					temp_mil = (((uint16_t)meter_rx_buffer[i+3])-0x30)*100;
				}
				else if( meter_rx_buffer[i+5] == 0x29 ) // если 2 разряда после запятой
				{
					temp_mil = (((((uint16_t)meter_rx_buffer[i+3])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+4])-0x30))*10;
				}
				else if( meter_rx_buffer[i+6] == 0x29 ) // если 3 разряда после запятой
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+3])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+4])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+5])-0x30);
				}
				//if( meter_rx_buffer[i+7] == 0x29 ) // если 4 разряда после запятой
				//{
					//temp_mil = ((((uint16_t)meter_rx_buffer[i+3])-0x30)*1000) + ((((uint16_t)meter_rx_buffer[i+4])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+5])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+6])-0x30);
				//}
				else
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+3])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+4])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+5])-0x30);
				}

				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*CE_303_CURRENT_C_REG, ((uint8_t)(temp>>8)) );
				fm25v02_write(2*CE_303_CURRENT_C_REG+1, ((uint8_t)(temp&0x00FF)) );
				fm25v02_write(2*CE_303_CURRENT_MIL_C_REG, ((uint8_t)(temp_mil>>8)) );
				fm25v02_write(2*CE_303_CURRENT_MIL_C_REG+1, ((uint8_t)(temp_mil&0x00FF)) );
				osMutexRelease(Fm25v02MutexHandle);
				status_registers.ce303_current_c_reg = temp;
				status_registers.ce303_current_mil_c_reg = temp_mil;
				current_c_state = 1;
			}
			else if( meter_rx_buffer[i+3] == 0x2E)
			{
				temp = ((((uint16_t)meter_rx_buffer[i+1])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+2])-0x30);

				if( meter_rx_buffer[i+5] == 0x29 ) // если 1 разряд после запятой
				{
					temp_mil = (((uint16_t)meter_rx_buffer[i+4])-0x30)*100;
				}
				else if( meter_rx_buffer[i+6] == 0x29 ) // если 2 разряда после запятой
				{
					temp_mil = (((((uint16_t)meter_rx_buffer[i+4])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+5])-0x30))*10;
				}
				else if( meter_rx_buffer[i+7] == 0x29 ) // если 3 разряда после запятой
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+4])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+5])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+6])-0x30);
				}
				//if( meter_rx_buffer[i+8] == 0x29 ) // если 4 разряда после запятой
				//{
					//temp_mil = ((((uint16_t)meter_rx_buffer[i+4])-0x30)*1000) + ((((uint16_t)meter_rx_buffer[i+5])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+6])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+7])-0x30);
				//}
				else
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+4])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+5])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+6])-0x30);
				}

				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*CE_303_CURRENT_C_REG, ((uint8_t)(temp>>8)) );
				fm25v02_write(2*CE_303_CURRENT_C_REG+1, ((uint8_t)(temp&0x00FF)) );
				fm25v02_write(2*CE_303_CURRENT_MIL_C_REG, ((uint8_t)(temp_mil>>8)) );
				fm25v02_write(2*CE_303_CURRENT_MIL_C_REG+1, ((uint8_t)(temp_mil&0x00FF)) );
				osMutexRelease(Fm25v02MutexHandle);
				status_registers.ce303_current_c_reg = temp;
				status_registers.ce303_current_mil_c_reg = temp_mil;
				current_c_state = 1;
			}

			else if( meter_rx_buffer[i+4] == 0x2E)
			{
				temp = ((((uint16_t)meter_rx_buffer[i+1])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+2])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+3])-0x30);

				if( meter_rx_buffer[i+6] == 0x29 ) // если 1 разряд после запятой
				{
					temp_mil = (((uint16_t)meter_rx_buffer[i+5])-0x30)*100;
				}
				else if( meter_rx_buffer[i+7] == 0x29 ) // если 2 разряда после запятой
				{
					temp_mil = (((((uint16_t)meter_rx_buffer[i+5])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+6])-0x30))*10;
				}
				else if( meter_rx_buffer[i+8] == 0x29 ) // если 3 разряда после запятой
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+5])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+6])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+7])-0x30);
				}
				//if( meter_rx_buffer[i+9] == 0x29 ) // если 4 разряда после запятой
				//{
					//temp_mil = ((((uint16_t)meter_rx_buffer[i+5])-0x30)*1000) + ((((uint16_t)meter_rx_buffer[i+6])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+7])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+8])-0x30);
				//}
				else
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+5])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+6])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+7])-0x30);
				}

				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*CE_303_CURRENT_C_REG, ((uint8_t)(temp>>8)) );
				fm25v02_write(2*CE_303_CURRENT_C_REG+1, ((uint8_t)(temp&0x00FF)) );
				fm25v02_write(2*CE_303_CURRENT_MIL_C_REG, ((uint8_t)(temp_mil>>8)) );
				fm25v02_write(2*CE_303_CURRENT_MIL_C_REG+1, ((uint8_t)(temp_mil&0x00FF)) );
				osMutexRelease(Fm25v02MutexHandle);
				status_registers.ce303_current_c_reg = temp;
				status_registers.ce303_current_mil_c_reg = temp_mil;
				current_c_state = 1;
			}
		}

	}

	meter_rx_number = 0;
	meter_rx_buffer_clear(); // очищаем буфер приема счетчика
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_10, GPIO_PIN_SET); // устанавливаем микросхему на передачу
	HAL_UART_Transmit_DMA(&huart6, &end_buf[0], 5); // передаем команду
	HAL_Delay(1000);

}

void ce303_power (uint8_t meter_id)
{
	uint16_t temp;
	uint16_t temp_mil;
	uint8_t power_a_state; // переменные состояния прочтения значений мощности ( если переменная равна 0, то значение мощности не прочитано, если 1 - значение прочитанно )
	uint8_t power_b_state;
	uint8_t power_c_state;

	if( (meter_id>99)&&(meter_id<1000) )
	{
		start_buf[0] = 0x2F;
		start_buf[1] = 0x3F;
		start_buf[2] = (meter_id/100) + 0x30;
		start_buf[3] = ((meter_id - (meter_id/100)*100)/10) + 0x30;
		start_buf[4] = ((meter_id - ((meter_id/100)*100)) - (((meter_id - ((meter_id/100)*100))/10)*10)) + 0x30 ;
		start_buf[5] = 0x21;
		start_buf[6] = 0x0D;
		start_buf[7] = 0x0A;

		meter_rx_number = 0;
		meter_rx_buffer_clear(); // очищаем буфер приема счетчика
		HAL_GPIO_WritePin(GPIOG, GPIO_PIN_10, GPIO_PIN_SET); // устанавливаем микросхему на передачу
		HAL_UART_Transmit_DMA(&huart6, &start_buf[0], 8); // передаем команду
		HAL_Delay(1000); // ждем

	}
	else if( (meter_id>9)&&(meter_id<100) )
	{
		start_buf[0] = 0x2F;
		start_buf[1] = 0x3F;
		start_buf[2] = (meter_id/10) + 0x30;
		start_buf[3] = (meter_id - ((meter_id/10)*10)) + 0x30;
		start_buf[4] = 0x21;
		start_buf[5] = 0x0D;
		start_buf[6] = 0x0A;

		meter_rx_number = 0;
		meter_rx_buffer_clear(); // очищаем буфер приема счетчика
		HAL_GPIO_WritePin(GPIOG, GPIO_PIN_10, GPIO_PIN_SET); // устанавливаем микросхему на передачу
		HAL_UART_Transmit_DMA(&huart6, &start_buf[0], 7); // передаем команду
		HAL_Delay(1000); // ждем

	}
	else if( (meter_id>=0)&&(meter_id<10) )
	{
		start_buf[0] = 0x2F;
		start_buf[1] = 0x3F;
		start_buf[2] = meter_id + 0x30;
		start_buf[3] = 0x21;
		start_buf[4] = 0x0D;
		start_buf[5] = 0x0A;

		meter_rx_number = 0;
		meter_rx_buffer_clear(); // очищаем буфер приема счетчика
		HAL_GPIO_WritePin(GPIOG, GPIO_PIN_10, GPIO_PIN_SET); // устанавливаем микросхему на передачу
		HAL_UART_Transmit_DMA(&huart6, &start_buf[0], 6); // передаем команду
		HAL_Delay(1000); // ждем
	}


	if( (meter_id>99)&&(meter_id<1000) )
	{
		start_buf[0] = 0x2F;
		start_buf[1] = 0x3F;
		start_buf[2] = (meter_id/100) + 0x30;
		start_buf[3] = ((meter_id - (meter_id/100)*100)/10) + 0x30;
		start_buf[4] = ((meter_id - ((meter_id/100)*100)) - (((meter_id - ((meter_id/100)*100))/10)*10)) + 0x30 ;
		start_buf[5] = 0x21;
		start_buf[6] = 0x0D;
		start_buf[7] = 0x0A;

		meter_rx_number = 0;
		meter_rx_buffer_clear(); // очищаем буфер приема счетчика
		HAL_GPIO_WritePin(GPIOG, GPIO_PIN_10, GPIO_PIN_SET); // устанавливаем микросхему на передачу
		HAL_UART_Transmit_DMA(&huart6, &start_buf[0], 8); // передаем команду
		HAL_Delay(1000); // ждем

	}
	else if( (meter_id>9)&&(meter_id<100) )
	{
		start_buf[0] = 0x2F;
		start_buf[1] = 0x3F;
		start_buf[2] = (meter_id/10) + 0x30;
		start_buf[3] = (meter_id - ((meter_id/10)*10)) + 0x30;
		start_buf[4] = 0x21;
		start_buf[5] = 0x0D;
		start_buf[6] = 0x0A;

		meter_rx_number = 0;
		meter_rx_buffer_clear(); // очищаем буфер приема счетчика
		HAL_GPIO_WritePin(GPIOG, GPIO_PIN_10, GPIO_PIN_SET); // устанавливаем микросхему на передачу
		HAL_UART_Transmit_DMA(&huart6, &start_buf[0], 7); // передаем команду
		HAL_Delay(1000); // ждем

	}
	else if( (meter_id>=0)&&(meter_id<10) )
	{
		start_buf[0] = 0x2F;
		start_buf[1] = 0x3F;
		start_buf[2] = meter_id + 0x30;
		start_buf[3] = 0x21;
		start_buf[4] = 0x0D;
		start_buf[5] = 0x0A;

		meter_rx_number = 0;
		meter_rx_buffer_clear(); // очищаем буфер приема счетчика
		HAL_GPIO_WritePin(GPIOG, GPIO_PIN_10, GPIO_PIN_SET); // устанавливаем микросхему на передачу
		HAL_UART_Transmit_DMA(&huart6, &start_buf[0], 6); // передаем команду
		HAL_Delay(1000); // ждем
	}


	/*
	meter_rx_number = 0;
	meter_rx_buffer_clear(); // очищаем буфер приема счетчика
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_10, GPIO_PIN_SET); // устанавливаем микросхему на передачу
	HAL_UART_Transmit_DMA(&huart6, &start_buf[0], 6); // передаем команду
	HAL_Delay(1000); // ждем

	meter_rx_number = 0;
	meter_rx_buffer_clear(); // очищаем буфер приема счетчика
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_10, GPIO_PIN_SET); // устанавливаем микросхему на передачу
	HAL_UART_Transmit_DMA(&huart6, &start_buf[0], 6); // передаем команду
	HAL_Delay(1000); // ждем
	*/

	meter_rx_number = 0;
	meter_rx_buffer_clear(); // очищаем буфер приема счетчика
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_10, GPIO_PIN_SET); // устанавливаем микросхему на передачу
	HAL_UART_Transmit_DMA(&huart6, &password_buf[0], 6); // передаем команду
	HAL_Delay(1000);

	meter_rx_number = 0;
	meter_rx_buffer_clear(); // очищаем буфер приема счетчика
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_10, GPIO_PIN_SET); // устанавливаем микросхему на передачу
	HAL_UART_Transmit_DMA(&huart6, &power_buf[0], 13); // передаем команду
	HAL_Delay(1000);

	power_a_state = 0; // обнуляем переменные состояния прочтения мощщности перед их прочтением
	power_b_state = 0;
	power_c_state = 0;

	for(uint16_t i=0; i<256; i++)
	{

		if( (meter_rx_buffer[i] == 0x28) && (power_a_state==0) && (power_b_state==0) && (power_c_state==0) ) // 0х28 означает символ "(" в ASCII таблице
		{
			if( meter_rx_buffer[i+2] == 0x2E) // 0х2Е означает символ "." в ASCII таблице
			{
				temp = (uint16_t)(meter_rx_buffer[i+1]-0x30);

				if( meter_rx_buffer[i+4] == 0x29 ) // если 1 разряд после запятой
				{
					temp_mil = (((uint16_t)meter_rx_buffer[i+3])-0x30)*100;
				}
				else if( meter_rx_buffer[i+5] == 0x29 ) // если 2 разряда после запятой
				{
					temp_mil = (((((uint16_t)meter_rx_buffer[i+3])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+4])-0x30))*10;
				}
				else if( meter_rx_buffer[i+6] == 0x29 ) // если 3 разряда после запятой
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+3])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+4])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+5])-0x30);
				}
				//if( meter_rx_buffer[i+7] == 0x29 ) // если 4 разряда после запятой
				//{
					//temp_mil = ((((uint16_t)meter_rx_buffer[i+3])-0x30)*1000) + ((((uint16_t)meter_rx_buffer[i+4])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+5])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+6])-0x30);
				//}
				else
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+3])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+4])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+5])-0x30);
				}

				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*CE_303_POWER_A_REG, ((uint8_t)(temp>>8)) );
				fm25v02_write(2*CE_303_POWER_A_REG+1, ((uint8_t)(temp&0x00FF)) );
				fm25v02_write(2*CE_303_POWER_MIL_A_REG, ((uint8_t)(temp_mil>>8)) );
				fm25v02_write(2*CE_303_POWER_MIL_A_REG+1, ((uint8_t)(temp_mil&0x00FF)) );
				osMutexRelease(Fm25v02MutexHandle);
				status_registers.ce303_power_a_reg = temp;
				status_registers.ce303_power_mil_a_reg =  temp_mil;
				power_a_state = 1;
			}
			else if( meter_rx_buffer[i+3] == 0x2E)
			{
				temp = ((((uint16_t)meter_rx_buffer[i+1])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+2])-0x30);

				if( meter_rx_buffer[i+5] == 0x29 ) // если 1 разряд после запятой
				{
					temp_mil = (((uint16_t)meter_rx_buffer[i+4])-0x30)*100;
				}
				else if( meter_rx_buffer[i+6] == 0x29 ) // если 2 разряда после запятой
				{
					temp_mil = (((((uint16_t)meter_rx_buffer[i+4])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+5])-0x30))*10;
				}
				else if( meter_rx_buffer[i+7] == 0x29 ) // если 3 разряда после запятой
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+4])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+5])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+6])-0x30);
				}
				//if( meter_rx_buffer[i+8] == 0x29 ) // если 4 разряда после запятой
				//{
					//temp_mil = ((((uint16_t)meter_rx_buffer[i+4])-0x30)*1000) + ((((uint16_t)meter_rx_buffer[i+5])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+6])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+7])-0x30);
				//}
				else
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+4])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+5])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+6])-0x30);
				}

				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*CE_303_POWER_A_REG, ((uint8_t)(temp>>8)) );
				fm25v02_write(2*CE_303_POWER_A_REG+1, ((uint8_t)(temp&0x00FF)) );
				fm25v02_write(2*CE_303_POWER_MIL_A_REG, ((uint8_t)(temp_mil>>8)) );
				fm25v02_write(2*CE_303_POWER_MIL_A_REG+1, ((uint8_t)(temp_mil&0x00FF)) );
				osMutexRelease(Fm25v02MutexHandle);
				status_registers.ce303_power_a_reg = temp;
				status_registers.ce303_power_mil_a_reg = temp_mil;
				power_a_state = 1;
			}

			else if( meter_rx_buffer[i+4] == 0x2E)
			{
				temp = ((((uint16_t)meter_rx_buffer[i+1])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+2])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+3])-0x30);

				if( meter_rx_buffer[i+6] == 0x29 ) // если 1 разряд после запятой
				{
					temp_mil = (((uint16_t)meter_rx_buffer[i+5])-0x30)*100;
				}
				else if( meter_rx_buffer[i+7] == 0x29 ) // если 2 разряда после запятой
				{
					temp_mil = (((((uint16_t)meter_rx_buffer[i+5])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+6])-0x30))*10;
				}
				else if( meter_rx_buffer[i+8] == 0x29 ) // если 3 разряда после запятой
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+5])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+6])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+7])-0x30);
				}
				//if( meter_rx_buffer[i+9] == 0x29 ) // если 4 разряда после запятой
				//{
					//temp_mil = ((((uint16_t)meter_rx_buffer[i+5])-0x30)*1000) + ((((uint16_t)meter_rx_buffer[i+6])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+7])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+8])-0x30);
				//}
				else
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+5])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+6])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+7])-0x30);
				}

				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*CE_303_POWER_A_REG, ((uint8_t)(temp>>8)) );
				fm25v02_write(2*CE_303_POWER_A_REG+1, ((uint8_t)(temp&0x00FF)) );
				fm25v02_write(2*CE_303_POWER_MIL_A_REG, ((uint8_t)(temp_mil>>8)) );
				fm25v02_write(2*CE_303_POWER_MIL_A_REG+1, ((uint8_t)(temp_mil&0x00FF)) );
				osMutexRelease(Fm25v02MutexHandle);
				status_registers.ce303_power_a_reg = temp;
				status_registers.ce303_power_mil_a_reg = temp_mil;
				power_a_state = 1;
			}
		}

		else if( (meter_rx_buffer[i] == 0x28) && (power_a_state==1) && (power_b_state==0) && (power_c_state==0) )
		{
			if( meter_rx_buffer[i+2] == 0x2E) // 0х2Е означает символ "." в ASCII таблице
			{
				temp = (uint16_t)(meter_rx_buffer[i+1]-0x30);

				if( meter_rx_buffer[i+4] == 0x29 ) // если 1 разряд после запятой
				{
					temp_mil = (((uint16_t)meter_rx_buffer[i+3])-0x30)*100;
				}
				else if( meter_rx_buffer[i+5] == 0x29 ) // если 2 разряда после запятой
				{
					temp_mil = (((((uint16_t)meter_rx_buffer[i+3])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+4])-0x30))*10;
				}
				else if( meter_rx_buffer[i+6] == 0x29 ) // если 3 разряда после запятой
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+3])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+4])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+5])-0x30);
				}
				//if( meter_rx_buffer[i+7] == 0x29 ) // если 4 разряда после запятой
				//{
					//temp_mil = ((((uint16_t)meter_rx_buffer[i+3])-0x30)*1000) + ((((uint16_t)meter_rx_buffer[i+4])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+5])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+6])-0x30);
				//}
				else
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+3])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+4])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+5])-0x30);
				}

				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*CE_303_POWER_B_REG, ((uint8_t)(temp>>8)) );
				fm25v02_write(2*CE_303_POWER_B_REG+1, ((uint8_t)(temp&0x00FF)) );
				fm25v02_write(2*CE_303_POWER_MIL_B_REG, ((uint8_t)(temp_mil>>8)) );
				fm25v02_write(2*CE_303_POWER_MIL_B_REG+1, ((uint8_t)(temp_mil&0x00FF)) );
				osMutexRelease(Fm25v02MutexHandle);
				status_registers.ce303_power_b_reg = temp;
				status_registers.ce303_power_mil_b_reg =  temp_mil;
				power_b_state = 1;
			}
			else if( meter_rx_buffer[i+3] == 0x2E)
			{
				temp = ((((uint16_t)meter_rx_buffer[i+1])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+2])-0x30);

				if( meter_rx_buffer[i+5] == 0x29 ) // если 1 разряд после запятой
				{
					temp_mil = (((uint16_t)meter_rx_buffer[i+4])-0x30)*100;
				}
				else if( meter_rx_buffer[i+6] == 0x29 ) // если 2 разряда после запятой
				{
					temp_mil = (((((uint16_t)meter_rx_buffer[i+4])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+5])-0x30))*10;
				}
				else if( meter_rx_buffer[i+7] == 0x29 ) // если 3 разряда после запятой
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+4])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+5])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+6])-0x30);
				}
				//if( meter_rx_buffer[i+8] == 0x29 ) // если 4 разряда после запятой
				//{
					//temp_mil = ((((uint16_t)meter_rx_buffer[i+4])-0x30)*1000) + ((((uint16_t)meter_rx_buffer[i+5])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+6])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+7])-0x30);
				//}
				else
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+4])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+5])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+6])-0x30);
				}

				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*CE_303_POWER_B_REG, ((uint8_t)(temp>>8)) );
				fm25v02_write(2*CE_303_POWER_B_REG+1, ((uint8_t)(temp&0x00FF)) );
				fm25v02_write(2*CE_303_POWER_MIL_B_REG, ((uint8_t)(temp_mil>>8)) );
				fm25v02_write(2*CE_303_POWER_MIL_B_REG+1, ((uint8_t)(temp_mil&0x00FF)) );
				osMutexRelease(Fm25v02MutexHandle);
				status_registers.ce303_power_b_reg = temp;
				status_registers.ce303_power_mil_b_reg = temp_mil;
				power_b_state = 1;
			}

			else if( meter_rx_buffer[i+4] == 0x2E)
			{
				temp = ((((uint16_t)meter_rx_buffer[i+1])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+2])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+3])-0x30);

				if( meter_rx_buffer[i+6] == 0x29 ) // если 1 разряд после запятой
				{
					temp_mil = (((uint16_t)meter_rx_buffer[i+5])-0x30)*100;
				}
				else if( meter_rx_buffer[i+7] == 0x29 ) // если 2 разряда после запятой
				{
					temp_mil = (((((uint16_t)meter_rx_buffer[i+5])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+6])-0x30))*10;
				}
				else if( meter_rx_buffer[i+8] == 0x29 ) // если 3 разряда после запятой
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+5])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+6])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+7])-0x30);
				}
				//if( meter_rx_buffer[i+9] == 0x29 ) // если 4 разряда после запятой
				//{
					//temp_mil = ((((uint16_t)meter_rx_buffer[i+5])-0x30)*1000) + ((((uint16_t)meter_rx_buffer[i+6])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+7])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+8])-0x30);
				//}
				else
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+5])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+6])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+7])-0x30);
				}

				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*CE_303_POWER_B_REG, ((uint8_t)(temp>>8)) );
				fm25v02_write(2*CE_303_POWER_B_REG+1, ((uint8_t)(temp&0x00FF)) );
				fm25v02_write(2*CE_303_POWER_MIL_B_REG, ((uint8_t)(temp_mil>>8)) );
				fm25v02_write(2*CE_303_POWER_MIL_B_REG+1, ((uint8_t)(temp_mil&0x00FF)) );
				osMutexRelease(Fm25v02MutexHandle);
				status_registers.ce303_power_b_reg = temp;
				status_registers.ce303_power_mil_b_reg = temp_mil;
				power_b_state = 1;
			}
		}

		else if( (meter_rx_buffer[i] == 0x28) && (power_a_state==1) && (power_b_state==1) && (power_c_state==0) )
		{
			if( meter_rx_buffer[i+2] == 0x2E) // 0х2Е означает символ "." в ASCII таблице
			{
				temp = (uint16_t)(meter_rx_buffer[i+1]-0x30);

				if( meter_rx_buffer[i+4] == 0x29 ) // если 1 разряд после запятой
				{
					temp_mil = (((uint16_t)meter_rx_buffer[i+3])-0x30)*100;
				}
				else if( meter_rx_buffer[i+5] == 0x29 ) // если 2 разряда после запятой
				{
					temp_mil = (((((uint16_t)meter_rx_buffer[i+3])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+4])-0x30))*10;
				}
				else if( meter_rx_buffer[i+6] == 0x29 ) // если 3 разряда после запятой
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+3])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+4])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+5])-0x30);
				}
				//if( meter_rx_buffer[i+7] == 0x29 ) // если 4 разряда после запятой
				//{
					//temp_mil = ((((uint16_t)meter_rx_buffer[i+3])-0x30)*1000) + ((((uint16_t)meter_rx_buffer[i+4])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+5])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+6])-0x30);
				//}
				else
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+3])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+4])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+5])-0x30);
				}

				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*CE_303_POWER_C_REG, ((uint8_t)(temp>>8)) );
				fm25v02_write(2*CE_303_POWER_C_REG+1, ((uint8_t)(temp&0x00FF)) );
				fm25v02_write(2*CE_303_POWER_MIL_C_REG, ((uint8_t)(temp_mil>>8)) );
				fm25v02_write(2*CE_303_POWER_MIL_C_REG+1, ((uint8_t)(temp_mil&0x00FF)) );
				osMutexRelease(Fm25v02MutexHandle);
				status_registers.ce303_power_c_reg = temp;
				status_registers.ce303_power_mil_c_reg = temp_mil;
				power_c_state = 1;
			}
			else if( meter_rx_buffer[i+3] == 0x2E)
			{
				temp = ((((uint16_t)meter_rx_buffer[i+1])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+2])-0x30);

				if( meter_rx_buffer[i+5] == 0x29 ) // если 1 разряд после запятой
				{
					temp_mil = (((uint16_t)meter_rx_buffer[i+4])-0x30)*100;
				}
				else if( meter_rx_buffer[i+6] == 0x29 ) // если 2 разряда после запятой
				{
					temp_mil = (((((uint16_t)meter_rx_buffer[i+4])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+5])-0x30))*10;
				}
				else if( meter_rx_buffer[i+7] == 0x29 ) // если 3 разряда после запятой
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+4])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+5])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+6])-0x30);
				}
				//if( meter_rx_buffer[i+8] == 0x29 ) // если 4 разряда после запятой
				//{
					//temp_mil = ((((uint16_t)meter_rx_buffer[i+4])-0x30)*1000) + ((((uint16_t)meter_rx_buffer[i+5])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+6])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+7])-0x30);
				//}
				else
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+4])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+5])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+6])-0x30);
				}

				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*CE_303_POWER_C_REG, ((uint8_t)(temp>>8)) );
				fm25v02_write(2*CE_303_POWER_C_REG+1, ((uint8_t)(temp&0x00FF)) );
				fm25v02_write(2*CE_303_POWER_MIL_C_REG, ((uint8_t)(temp_mil>>8)) );
				fm25v02_write(2*CE_303_POWER_MIL_C_REG+1, ((uint8_t)(temp_mil&0x00FF)) );
				osMutexRelease(Fm25v02MutexHandle);
				status_registers.ce303_power_c_reg = temp;
				status_registers.ce303_power_mil_c_reg = temp_mil;
				power_c_state = 1;
			}

			else if( meter_rx_buffer[i+4] == 0x2E)
			{
				temp = ((((uint16_t)meter_rx_buffer[i+1])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+2])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+3])-0x30);

				if( meter_rx_buffer[i+6] == 0x29 ) // если 1 разряд после запятой
				{
					temp_mil = (((uint16_t)meter_rx_buffer[i+5])-0x30)*100;
				}
				else if( meter_rx_buffer[i+7] == 0x29 ) // если 2 разряда после запятой
				{
					temp_mil = (((((uint16_t)meter_rx_buffer[i+5])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+6])-0x30))*10;
				}
				else if( meter_rx_buffer[i+8] == 0x29 ) // если 3 разряда после запятой
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+5])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+6])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+7])-0x30);
				}
				//if( meter_rx_buffer[i+9] == 0x29 ) // если 4 разряда после запятой
				//{
					//temp_mil = ((((uint16_t)meter_rx_buffer[i+5])-0x30)*1000) + ((((uint16_t)meter_rx_buffer[i+6])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+7])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+8])-0x30);
				//}
				else
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+5])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+6])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+7])-0x30);
				}

				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*CE_303_POWER_C_REG, ((uint8_t)(temp>>8)) );
				fm25v02_write(2*CE_303_POWER_C_REG+1, ((uint8_t)(temp&0x00FF)) );
				fm25v02_write(2*CE_303_POWER_MIL_C_REG, ((uint8_t)(temp_mil>>8)) );
				fm25v02_write(2*CE_303_POWER_MIL_C_REG+1, ((uint8_t)(temp_mil&0x00FF)) );
				osMutexRelease(Fm25v02MutexHandle);
				status_registers.ce303_power_c_reg = temp;
				status_registers.ce303_power_mil_c_reg = temp_mil;
				power_c_state = 1;
			}
		}

	}

	meter_rx_number = 0;
	meter_rx_buffer_clear(); // очищаем буфер приема счетчика
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_10, GPIO_PIN_SET); // устанавливаем микросхему на передачу
	HAL_UART_Transmit_DMA(&huart6, &end_buf[0], 5); // передаем команду
	HAL_Delay(1000);

}

void ce303_tarif (uint8_t meter_id)
{
	uint32_t temp;
	uint16_t temp_mil;
	uint8_t tarif_general_state; // переменные состояния прочтения значений мощности ( если переменная равна 0, то значение мощности не прочитано, если 1 - значение прочитанно )
	uint8_t tarif_1_state;
	uint8_t tarif_2_state;
	uint8_t tarif_3_state;
	uint8_t tarif_4_state;
	uint8_t tarif_5_state;

	if( (meter_id>99)&&(meter_id<1000) )
	{
		start_buf[0] = 0x2F;
		start_buf[1] = 0x3F;
		start_buf[2] = (meter_id/100) + 0x30;
		start_buf[3] = ((meter_id - (meter_id/100)*100)/10) + 0x30;
		start_buf[4] = ((meter_id - ((meter_id/100)*100)) - (((meter_id - ((meter_id/100)*100))/10)*10)) + 0x30 ;
		start_buf[5] = 0x21;
		start_buf[6] = 0x0D;
		start_buf[7] = 0x0A;

		meter_rx_number = 0;
		meter_rx_buffer_clear(); // очищаем буфер приема счетчика
		HAL_GPIO_WritePin(GPIOG, GPIO_PIN_10, GPIO_PIN_SET); // устанавливаем микросхему на передачу
		HAL_UART_Transmit_DMA(&huart6, &start_buf[0], 8); // передаем команду
		HAL_Delay(1000); // ждем

	}
	else if( (meter_id>9)&&(meter_id<100) )
	{
		start_buf[0] = 0x2F;
		start_buf[1] = 0x3F;
		start_buf[2] = (meter_id/10) + 0x30;
		start_buf[3] = (meter_id - ((meter_id/10)*10)) + 0x30;
		start_buf[4] = 0x21;
		start_buf[5] = 0x0D;
		start_buf[6] = 0x0A;

		meter_rx_number = 0;
		meter_rx_buffer_clear(); // очищаем буфер приема счетчика
		HAL_GPIO_WritePin(GPIOG, GPIO_PIN_10, GPIO_PIN_SET); // устанавливаем микросхему на передачу
		HAL_UART_Transmit_DMA(&huart6, &start_buf[0], 7); // передаем команду
		HAL_Delay(1000); // ждем

	}
	else if( (meter_id>=0)&&(meter_id<10) )
	{
		start_buf[0] = 0x2F;
		start_buf[1] = 0x3F;
		start_buf[2] = meter_id + 0x30;
		start_buf[3] = 0x21;
		start_buf[4] = 0x0D;
		start_buf[5] = 0x0A;

		meter_rx_number = 0;
		meter_rx_buffer_clear(); // очищаем буфер приема счетчика
		HAL_GPIO_WritePin(GPIOG, GPIO_PIN_10, GPIO_PIN_SET); // устанавливаем микросхему на передачу
		HAL_UART_Transmit_DMA(&huart6, &start_buf[0], 6); // передаем команду
		HAL_Delay(1000); // ждем
	}

	if( (meter_id>99)&&(meter_id<1000) )
	{
		start_buf[0] = 0x2F;
		start_buf[1] = 0x3F;
		start_buf[2] = (meter_id/100) + 0x30;
		start_buf[3] = ((meter_id - (meter_id/100)*100)/10) + 0x30;
		start_buf[4] = ((meter_id - ((meter_id/100)*100)) - (((meter_id - ((meter_id/100)*100))/10)*10)) + 0x30 ;
		start_buf[5] = 0x21;
		start_buf[6] = 0x0D;
		start_buf[7] = 0x0A;

		meter_rx_number = 0;
		meter_rx_buffer_clear(); // очищаем буфер приема счетчика
		HAL_GPIO_WritePin(GPIOG, GPIO_PIN_10, GPIO_PIN_SET); // устанавливаем микросхему на передачу
		HAL_UART_Transmit_DMA(&huart6, &start_buf[0], 8); // передаем команду
		HAL_Delay(1000); // ждем

	}
	else if( (meter_id>9)&&(meter_id<100) )
	{
		start_buf[0] = 0x2F;
		start_buf[1] = 0x3F;
		start_buf[2] = (meter_id/10) + 0x30;
		start_buf[3] = (meter_id - ((meter_id/10)*10)) + 0x30;
		start_buf[4] = 0x21;
		start_buf[5] = 0x0D;
		start_buf[6] = 0x0A;

		meter_rx_number = 0;
		meter_rx_buffer_clear(); // очищаем буфер приема счетчика
		HAL_GPIO_WritePin(GPIOG, GPIO_PIN_10, GPIO_PIN_SET); // устанавливаем микросхему на передачу
		HAL_UART_Transmit_DMA(&huart6, &start_buf[0], 7); // передаем команду
		HAL_Delay(1000); // ждем

	}
	else if( (meter_id>=0)&&(meter_id<10) )
	{
		start_buf[0] = 0x2F;
		start_buf[1] = 0x3F;
		start_buf[2] = meter_id + 0x30;
		start_buf[3] = 0x21;
		start_buf[4] = 0x0D;
		start_buf[5] = 0x0A;

		meter_rx_number = 0;
		meter_rx_buffer_clear(); // очищаем буфер приема счетчика
		HAL_GPIO_WritePin(GPIOG, GPIO_PIN_10, GPIO_PIN_SET); // устанавливаем микросхему на передачу
		HAL_UART_Transmit_DMA(&huart6, &start_buf[0], 6); // передаем команду
		HAL_Delay(1000); // ждем
	}


	/*
	meter_rx_number = 0;
	meter_rx_buffer_clear(); // очищаем буфер приема счетчика
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_10, GPIO_PIN_SET); // устанавливаем микросхему на передачу
	HAL_UART_Transmit_DMA(&huart6, &start_buf[0], 6); // передаем команду
	HAL_Delay(1000); // ждем

	meter_rx_number = 0;
	meter_rx_buffer_clear(); // очищаем буфер приема счетчика
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_10, GPIO_PIN_SET); // устанавливаем микросхему на передачу
	HAL_UART_Transmit_DMA(&huart6, &start_buf[0], 6); // передаем команду
	HAL_Delay(1000); // ждем
	*/

	meter_rx_number = 0;
	meter_rx_buffer_clear(); // очищаем буфер приема счетчика
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_10, GPIO_PIN_SET); // устанавливаем микросхему на передачу
	HAL_UART_Transmit_DMA(&huart6, &password_buf[0], 6); // передаем команду
	HAL_Delay(1000);

	meter_rx_number = 0;
	meter_rx_buffer_clear(); // очищаем буфер приема счетчика
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_10, GPIO_PIN_SET); // устанавливаем микросхему на передачу
	HAL_UART_Transmit_DMA(&huart6, &tarif_buf[0], 13); // передаем команду
	HAL_Delay(1000);

	tarif_general_state = 0; // обнуляем переменные состояния прочтения мощщности перед их прочтением
	tarif_1_state = 0;
	tarif_2_state = 0;
	tarif_3_state = 0;
	tarif_4_state = 0;
	tarif_5_state = 0;

	for(uint16_t i=0; i<256; i++)
	{
		if( (meter_rx_buffer[i] == 0x28) && (tarif_general_state==0) && (tarif_1_state==0) && (tarif_2_state==0) && (tarif_3_state==0) && (tarif_4_state==0) && (tarif_5_state==0) ) // 0х28 означает символ "(" в ASCII таблице
		{
			if( meter_rx_buffer[i+2] == 0x2E) // 0х2Е означает символ "." в ASCII таблице, 1 цифра перед запятой
			{
				temp = (uint32_t)(meter_rx_buffer[i+1]-0x30);

				if( meter_rx_buffer[i+4] == 0x29 ) // если 1 разряд после запятой
				{
					temp_mil = (((uint16_t)meter_rx_buffer[i+3])-0x30)*100;
				}
				else if( meter_rx_buffer[i+5] == 0x29 ) // если 2 разряда после запятой
				{
					temp_mil = (((((uint16_t)meter_rx_buffer[i+3])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+4])-0x30))*10;
				}
				else if( meter_rx_buffer[i+6] == 0x29 ) // если 3 разряда после запятой
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+3])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+4])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+5])-0x30);
				}
				//if( meter_rx_buffer[i+7] == 0x29 ) // если 4 разряда после запятой
				//{
					//temp_mil = ((((uint16_t)meter_rx_buffer[i+3])-0x30)*1000) + ((((uint16_t)meter_rx_buffer[i+4])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+5])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+6])-0x30);
				//}
				else
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+3])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+4])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+5])-0x30);
				}

				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*CE_303_TOTAL_POWER_H_REG, ((uint8_t)(temp>>24)) );
				fm25v02_write(2*CE_303_TOTAL_POWER_H_REG+1, ((uint8_t)(temp>>16)) );
				fm25v02_write(2*CE_303_TOTAL_POWER_L_REG, ((uint8_t)(temp>>8)) );
				fm25v02_write(2*CE_303_TOTAL_POWER_L_REG+1, ((uint8_t)temp) );
				fm25v02_write(2*CE_303_TOTAL_POWER_M_REG, ((uint8_t)(temp_mil>>8)) );
				fm25v02_write(2*CE_303_TOTAL_POWER_M_REG+1, ((uint8_t)(temp_mil)) );
				osMutexRelease(Fm25v02MutexHandle);
				status_registers.ce303_total_power_h_reg = (uint16_t)(temp>>16);
				status_registers.ce303_total_power_l_reg = (uint16_t)temp;
				status_registers.ce303_total_power_m_reg =  temp_mil;
				tarif_general_state = 1;

			}

			else if( meter_rx_buffer[i+3] == 0x2E) // 2 цифры перед запятой
			{
				temp = ((((uint16_t)meter_rx_buffer[i+1])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+2])-0x30);

				if( meter_rx_buffer[i+5] == 0x29 ) // если 1 разряд после запятой
				{
					temp_mil = (((uint16_t)meter_rx_buffer[i+4])-0x30)*100;
				}
				else if( meter_rx_buffer[i+6] == 0x29 ) // если 2 разряда после запятой
				{
					temp_mil = (((((uint16_t)meter_rx_buffer[i+4])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+5])-0x30))*10;
				}
				else if( meter_rx_buffer[i+7] == 0x29 ) // если 3 разряда после запятой
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+4])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+5])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+6])-0x30);
				}
				//if( meter_rx_buffer[i+8] == 0x29 ) // если 4 разряда после запятой
				//{
					//temp_mil = ((((uint16_t)meter_rx_buffer[i+4])-0x30)*1000) + ((((uint16_t)meter_rx_buffer[i+5])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+6])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+7])-0x30);
				//}
				else
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+4])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+5])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+6])-0x30);
				}

				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*CE_303_TOTAL_POWER_H_REG, ((uint8_t)(temp>>24)) );
				fm25v02_write(2*CE_303_TOTAL_POWER_H_REG+1, ((uint8_t)(temp>>16)) );
				fm25v02_write(2*CE_303_TOTAL_POWER_L_REG, ((uint8_t)(temp>>8)) );
				fm25v02_write(2*CE_303_TOTAL_POWER_L_REG+1, ((uint8_t)temp) );
				fm25v02_write(2*CE_303_TOTAL_POWER_M_REG, ((uint8_t)(temp_mil>>8)) );
				fm25v02_write(2*CE_303_TOTAL_POWER_M_REG+1, ((uint8_t)(temp_mil)) );
				osMutexRelease(Fm25v02MutexHandle);
				status_registers.ce303_total_power_h_reg = (uint16_t)(temp>>16);
				status_registers.ce303_total_power_l_reg = (uint16_t)temp;
				status_registers.ce303_total_power_m_reg =  temp_mil;
				tarif_general_state = 1;

			}

			else if( meter_rx_buffer[i+4] == 0x2E) // 3 цифры перед запятой
			{
				temp = ((((uint16_t)meter_rx_buffer[i+1])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+2])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+3])-0x30);

				if( meter_rx_buffer[i+6] == 0x29 ) // если 1 разряд после запятой
				{
					temp_mil = (((uint16_t)meter_rx_buffer[i+5])-0x30)*100;
				}
				else if( meter_rx_buffer[i+7] == 0x29 ) // если 2 разряда после запятой
				{
					temp_mil = (((((uint16_t)meter_rx_buffer[i+5])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+6])-0x30))*10;
				}
				else if( meter_rx_buffer[i+8] == 0x29 ) // если 3 разряда после запятой
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+5])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+6])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+7])-0x30);
				}
				//if( meter_rx_buffer[i+9] == 0x29 ) // если 4 разряда после запятой
				//{
					//temp_mil = ((((uint16_t)meter_rx_buffer[i+5])-0x30)*1000) + ((((uint16_t)meter_rx_buffer[i+6])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+7])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+8])-0x30);
				//}
				else
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+5])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+6])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+7])-0x30);
				}

				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*CE_303_TOTAL_POWER_H_REG, ((uint8_t)(temp>>24)) );
				fm25v02_write(2*CE_303_TOTAL_POWER_H_REG+1, ((uint8_t)(temp>>16)) );
				fm25v02_write(2*CE_303_TOTAL_POWER_L_REG, ((uint8_t)(temp>>8)) );
				fm25v02_write(2*CE_303_TOTAL_POWER_L_REG+1, ((uint8_t)temp) );
				fm25v02_write(2*CE_303_TOTAL_POWER_M_REG, ((uint8_t)(temp_mil>>8)) );
				fm25v02_write(2*CE_303_TOTAL_POWER_M_REG+1, ((uint8_t)(temp_mil)) );
				osMutexRelease(Fm25v02MutexHandle);
				status_registers.ce303_total_power_h_reg = (uint16_t)(temp>>16);
				status_registers.ce303_total_power_l_reg = (uint16_t)temp;
				status_registers.ce303_total_power_m_reg =  temp_mil;
				tarif_general_state = 1;

			}

			else if( meter_rx_buffer[i+5] == 0x2E) // четыре цифры перед запятой
			{
				temp = ((((uint16_t)meter_rx_buffer[i+1])-0x30)*1000) + ((((uint16_t)meter_rx_buffer[i+2])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+3])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+4])-0x30);

				if( meter_rx_buffer[i+7] == 0x29 ) // если 1 разряд после запятой
				{
					temp_mil = (((uint16_t)meter_rx_buffer[i+6])-0x30)*100;
				}
				else if( meter_rx_buffer[i+8] == 0x29 ) // если 2 разряда после запятой
				{
					temp_mil = (((((uint16_t)meter_rx_buffer[i+6])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+7])-0x30))*10;
				}
				else if( meter_rx_buffer[i+9] == 0x29 ) // если 3 разряда после запятой
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+6])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+7])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+8])-0x30);
				}
				//if( meter_rx_buffer[i+9] == 0x29 ) // если 4 разряда после запятой
				//{
					//temp_mil = ((((uint16_t)meter_rx_buffer[i+5])-0x30)*1000) + ((((uint16_t)meter_rx_buffer[i+6])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+7])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+8])-0x30);
				//}
				else
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+6])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+7])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+8])-0x30);
				}

				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*CE_303_TOTAL_POWER_H_REG, ((uint8_t)(temp>>24)) );
				fm25v02_write(2*CE_303_TOTAL_POWER_H_REG+1, ((uint8_t)(temp>>16)) );
				fm25v02_write(2*CE_303_TOTAL_POWER_L_REG, ((uint8_t)(temp>>8)) );
				fm25v02_write(2*CE_303_TOTAL_POWER_L_REG+1, ((uint8_t)temp) );
				fm25v02_write(2*CE_303_TOTAL_POWER_M_REG, ((uint8_t)(temp_mil>>8)) );
				fm25v02_write(2*CE_303_TOTAL_POWER_M_REG+1, ((uint8_t)(temp_mil)) );
				osMutexRelease(Fm25v02MutexHandle);
				status_registers.ce303_total_power_h_reg = (uint16_t)(temp>>16);
				status_registers.ce303_total_power_l_reg = (uint16_t)temp;
				status_registers.ce303_total_power_m_reg =  temp_mil;
				tarif_general_state = 1;

			}

		}

		else if( (meter_rx_buffer[i] == 0x28) && (tarif_general_state==1) && (tarif_1_state==0) && (tarif_2_state==0) && (tarif_3_state==0) && (tarif_4_state==0) && (tarif_5_state==0) ) // 0х28 означает символ "(" в ASCII таблице
		{
			if( meter_rx_buffer[i+2] == 0x2E) // 0х2Е означает символ "." в ASCII таблице, 1 цифра перед запятой
			{
				temp = (uint32_t)(meter_rx_buffer[i+1]-0x30);

				if( meter_rx_buffer[i+4] == 0x29 ) // если 1 разряд после запятой
				{
					temp_mil = (((uint16_t)meter_rx_buffer[i+3])-0x30)*100;
				}
				else if( meter_rx_buffer[i+5] == 0x29 ) // если 2 разряда после запятой
				{
					temp_mil = (((((uint16_t)meter_rx_buffer[i+3])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+4])-0x30))*10;
				}
				else if( meter_rx_buffer[i+6] == 0x29 ) // если 3 разряда после запятой
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+3])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+4])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+5])-0x30);
				}
				//if( meter_rx_buffer[i+7] == 0x29 ) // если 4 разряда после запятой
				//{
					//temp_mil = ((((uint16_t)meter_rx_buffer[i+3])-0x30)*1000) + ((((uint16_t)meter_rx_buffer[i+4])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+5])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+6])-0x30);
				//}
				else
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+3])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+4])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+5])-0x30);
				}

				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*CE_303_TARIF1_POWER_H_REG, ((uint8_t)(temp>>24)) );
				fm25v02_write(2*CE_303_TARIF1_POWER_H_REG+1, ((uint8_t)(temp>>16)) );
				fm25v02_write(2*CE_303_TARIF1_POWER_L_REG, ((uint8_t)(temp>>8)) );
				fm25v02_write(2*CE_303_TARIF1_POWER_L_REG+1, ((uint8_t)temp) );
				fm25v02_write(2*CE_303_TARIF1_POWER_MIL_REG, ((uint8_t)(temp_mil>>8)) );
				fm25v02_write(2*CE_303_TARIF1_POWER_MIL_REG+1, ((uint8_t)(temp_mil)) );
				osMutexRelease(Fm25v02MutexHandle);
				status_registers.ce303_tarif1_power_h_reg = (uint16_t)(temp>>16);
				status_registers.ce303_tarif1_power_l_reg = (uint16_t)temp;
				status_registers.ce303_tarif1_power_mil_reg =  temp_mil;
				tarif_1_state = 1;

			}

			else if( meter_rx_buffer[i+3] == 0x2E) // 2 цифры перед запятой
			{
				temp = ((((uint16_t)meter_rx_buffer[i+1])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+2])-0x30);

				if( meter_rx_buffer[i+5] == 0x29 ) // если 1 разряд после запятой
				{
					temp_mil = (((uint16_t)meter_rx_buffer[i+4])-0x30)*100;
				}
				else if( meter_rx_buffer[i+6] == 0x29 ) // если 2 разряда после запятой
				{
					temp_mil = (((((uint16_t)meter_rx_buffer[i+4])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+5])-0x30))*10;
				}
				else if( meter_rx_buffer[i+7] == 0x29 ) // если 3 разряда после запятой
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+4])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+5])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+6])-0x30);
				}
				//if( meter_rx_buffer[i+8] == 0x29 ) // если 4 разряда после запятой
				//{
					//temp_mil = ((((uint16_t)meter_rx_buffer[i+4])-0x30)*1000) + ((((uint16_t)meter_rx_buffer[i+5])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+6])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+7])-0x30);
				//}
				else
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+4])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+5])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+6])-0x30);
				}

				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*CE_303_TARIF1_POWER_H_REG, ((uint8_t)(temp>>24)) );
				fm25v02_write(2*CE_303_TARIF1_POWER_H_REG+1, ((uint8_t)(temp>>16)) );
				fm25v02_write(2*CE_303_TARIF1_POWER_L_REG, ((uint8_t)(temp>>8)) );
				fm25v02_write(2*CE_303_TARIF1_POWER_L_REG+1, ((uint8_t)temp) );
				fm25v02_write(2*CE_303_TARIF1_POWER_MIL_REG, ((uint8_t)(temp_mil>>8)) );
				fm25v02_write(2*CE_303_TARIF1_POWER_MIL_REG+1, ((uint8_t)(temp_mil)) );
				osMutexRelease(Fm25v02MutexHandle);
				status_registers.ce303_tarif1_power_h_reg = (uint16_t)(temp>>16);
				status_registers.ce303_tarif1_power_l_reg = (uint16_t)temp;
				status_registers.ce303_tarif1_power_mil_reg =  temp_mil;
				tarif_1_state = 1;

			}

			else if( meter_rx_buffer[i+4] == 0x2E) // 3 цифры перед запятой
			{
				temp = ((((uint16_t)meter_rx_buffer[i+1])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+2])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+3])-0x30);

				if( meter_rx_buffer[i+6] == 0x29 ) // если 1 разряд после запятой
				{
					temp_mil = (((uint16_t)meter_rx_buffer[i+5])-0x30)*100;
				}
				else if( meter_rx_buffer[i+7] == 0x29 ) // если 2 разряда после запятой
				{
					temp_mil = (((((uint16_t)meter_rx_buffer[i+5])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+6])-0x30))*10;
				}
				else if( meter_rx_buffer[i+8] == 0x29 ) // если 3 разряда после запятой
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+5])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+6])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+7])-0x30);
				}
				//if( meter_rx_buffer[i+9] == 0x29 ) // если 4 разряда после запятой
				//{
					//temp_mil = ((((uint16_t)meter_rx_buffer[i+5])-0x30)*1000) + ((((uint16_t)meter_rx_buffer[i+6])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+7])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+8])-0x30);
				//}
				else
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+5])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+6])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+7])-0x30);
				}

				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*CE_303_TARIF1_POWER_H_REG, ((uint8_t)(temp>>24)) );
				fm25v02_write(2*CE_303_TARIF1_POWER_H_REG+1, ((uint8_t)(temp>>16)) );
				fm25v02_write(2*CE_303_TARIF1_POWER_L_REG, ((uint8_t)(temp>>8)) );
				fm25v02_write(2*CE_303_TARIF1_POWER_L_REG+1, ((uint8_t)temp) );
				fm25v02_write(2*CE_303_TARIF1_POWER_MIL_REG, ((uint8_t)(temp_mil>>8)) );
				fm25v02_write(2*CE_303_TARIF1_POWER_MIL_REG+1, ((uint8_t)(temp_mil)) );
				osMutexRelease(Fm25v02MutexHandle);
				status_registers.ce303_tarif1_power_h_reg = (uint16_t)(temp>>16);
				status_registers.ce303_tarif1_power_l_reg = (uint16_t)temp;
				status_registers.ce303_tarif1_power_mil_reg =  temp_mil;
				tarif_1_state = 1;

			}

			else if( meter_rx_buffer[i+5] == 0x2E) // четыре цифры перед запятой
			{
				temp = ((((uint16_t)meter_rx_buffer[i+1])-0x30)*1000) + ((((uint16_t)meter_rx_buffer[i+2])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+3])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+4])-0x30);

				if( meter_rx_buffer[i+7] == 0x29 ) // если 1 разряд после запятой
				{
					temp_mil = (((uint16_t)meter_rx_buffer[i+6])-0x30)*100;
				}
				else if( meter_rx_buffer[i+8] == 0x29 ) // если 2 разряда после запятой
				{
					temp_mil = (((((uint16_t)meter_rx_buffer[i+6])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+7])-0x30))*10;
				}
				else if( meter_rx_buffer[i+9] == 0x29 ) // если 3 разряда после запятой
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+6])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+7])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+8])-0x30);
				}
				//if( meter_rx_buffer[i+9] == 0x29 ) // если 4 разряда после запятой
				//{
					//temp_mil = ((((uint16_t)meter_rx_buffer[i+5])-0x30)*1000) + ((((uint16_t)meter_rx_buffer[i+6])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+7])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+8])-0x30);
				//}
				else
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+6])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+7])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+8])-0x30);
				}

				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*CE_303_TARIF1_POWER_H_REG, ((uint8_t)(temp>>24)) );
				fm25v02_write(2*CE_303_TARIF1_POWER_H_REG+1, ((uint8_t)(temp>>16)) );
				fm25v02_write(2*CE_303_TARIF1_POWER_L_REG, ((uint8_t)(temp>>8)) );
				fm25v02_write(2*CE_303_TARIF1_POWER_L_REG+1, ((uint8_t)temp) );
				fm25v02_write(2*CE_303_TARIF1_POWER_MIL_REG, ((uint8_t)(temp_mil>>8)) );
				fm25v02_write(2*CE_303_TARIF1_POWER_MIL_REG+1, ((uint8_t)(temp_mil)) );
				osMutexRelease(Fm25v02MutexHandle);
				status_registers.ce303_tarif1_power_h_reg = (uint16_t)(temp>>16);
				status_registers.ce303_tarif1_power_l_reg = (uint16_t)temp;
				status_registers.ce303_tarif1_power_mil_reg =  temp_mil;
				tarif_1_state = 1;

			}

		}

		else if( (meter_rx_buffer[i] == 0x28) && (tarif_general_state==1) && (tarif_1_state==1) && (tarif_2_state==0) && (tarif_3_state==0) && (tarif_4_state==0) && (tarif_5_state==0) ) // 0х28 означает символ "(" в ASCII таблице
		{
			if( meter_rx_buffer[i+2] == 0x2E) // 0х2Е означает символ "." в ASCII таблице, 1 цифра перед запятой
			{
				temp = (uint32_t)(meter_rx_buffer[i+1]-0x30);

				if( meter_rx_buffer[i+4] == 0x29 ) // если 1 разряд после запятой
				{
					temp_mil = (((uint16_t)meter_rx_buffer[i+3])-0x30)*100;
				}
				else if( meter_rx_buffer[i+5] == 0x29 ) // если 2 разряда после запятой
				{
					temp_mil = (((((uint16_t)meter_rx_buffer[i+3])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+4])-0x30))*10;
				}
				else if( meter_rx_buffer[i+6] == 0x29 ) // если 3 разряда после запятой
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+3])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+4])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+5])-0x30);
				}
				//if( meter_rx_buffer[i+7] == 0x29 ) // если 4 разряда после запятой
				//{
					//temp_mil = ((((uint16_t)meter_rx_buffer[i+3])-0x30)*1000) + ((((uint16_t)meter_rx_buffer[i+4])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+5])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+6])-0x30);
				//}
				else
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+3])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+4])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+5])-0x30);
				}

				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*CE_303_TARIF2_POWER_H_REG, ((uint8_t)(temp>>24)) );
				fm25v02_write(2*CE_303_TARIF2_POWER_H_REG+1, ((uint8_t)(temp>>16)) );
				fm25v02_write(2*CE_303_TARIF2_POWER_L_REG, ((uint8_t)(temp>>8)) );
				fm25v02_write(2*CE_303_TARIF2_POWER_L_REG+1, ((uint8_t)temp) );
				fm25v02_write(2*CE_303_TARIF2_POWER_MIL_REG, ((uint8_t)(temp_mil>>8)) );
				fm25v02_write(2*CE_303_TARIF2_POWER_MIL_REG+1, ((uint8_t)(temp_mil)) );
				osMutexRelease(Fm25v02MutexHandle);
				status_registers.ce303_tarif2_power_h_reg = (uint16_t)(temp>>16);
				status_registers.ce303_tarif2_power_l_reg = (uint16_t)temp;
				status_registers.ce303_tarif2_power_mil_reg =  temp_mil;
				tarif_2_state = 1;

			}

			else if( meter_rx_buffer[i+3] == 0x2E) // 2 цифры перед запятой
			{
				temp = ((((uint16_t)meter_rx_buffer[i+1])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+2])-0x30);

				if( meter_rx_buffer[i+5] == 0x29 ) // если 1 разряд после запятой
				{
					temp_mil = (((uint16_t)meter_rx_buffer[i+4])-0x30)*100;
				}
				else if( meter_rx_buffer[i+6] == 0x29 ) // если 2 разряда после запятой
				{
					temp_mil = (((((uint16_t)meter_rx_buffer[i+4])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+5])-0x30))*10;
				}
				else if( meter_rx_buffer[i+7] == 0x29 ) // если 3 разряда после запятой
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+4])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+5])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+6])-0x30);
				}
				//if( meter_rx_buffer[i+8] == 0x29 ) // если 4 разряда после запятой
				//{
					//temp_mil = ((((uint16_t)meter_rx_buffer[i+4])-0x30)*1000) + ((((uint16_t)meter_rx_buffer[i+5])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+6])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+7])-0x30);
				//}
				else
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+4])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+5])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+6])-0x30);
				}

				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*CE_303_TARIF2_POWER_H_REG, ((uint8_t)(temp>>24)) );
				fm25v02_write(2*CE_303_TARIF2_POWER_H_REG+1, ((uint8_t)(temp>>16)) );
				fm25v02_write(2*CE_303_TARIF2_POWER_L_REG, ((uint8_t)(temp>>8)) );
				fm25v02_write(2*CE_303_TARIF2_POWER_L_REG+1, ((uint8_t)temp) );
				fm25v02_write(2*CE_303_TARIF2_POWER_MIL_REG, ((uint8_t)(temp_mil>>8)) );
				fm25v02_write(2*CE_303_TARIF2_POWER_MIL_REG+1, ((uint8_t)(temp_mil)) );
				osMutexRelease(Fm25v02MutexHandle);
				status_registers.ce303_tarif2_power_h_reg = (uint16_t)(temp>>16);
				status_registers.ce303_tarif2_power_l_reg = (uint16_t)temp;
				status_registers.ce303_tarif2_power_mil_reg =  temp_mil;
				tarif_2_state = 1;

			}

			else if( meter_rx_buffer[i+4] == 0x2E) // 3 цифры перед запятой
			{
				temp = ((((uint16_t)meter_rx_buffer[i+1])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+2])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+3])-0x30);

				if( meter_rx_buffer[i+6] == 0x29 ) // если 1 разряд после запятой
				{
					temp_mil = (((uint16_t)meter_rx_buffer[i+5])-0x30)*100;
				}
				else if( meter_rx_buffer[i+7] == 0x29 ) // если 2 разряда после запятой
				{
					temp_mil = (((((uint16_t)meter_rx_buffer[i+5])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+6])-0x30))*10;
				}
				else if( meter_rx_buffer[i+8] == 0x29 ) // если 3 разряда после запятой
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+5])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+6])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+7])-0x30);
				}
				//if( meter_rx_buffer[i+9] == 0x29 ) // если 4 разряда после запятой
				//{
					//temp_mil = ((((uint16_t)meter_rx_buffer[i+5])-0x30)*1000) + ((((uint16_t)meter_rx_buffer[i+6])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+7])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+8])-0x30);
				//}
				else
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+5])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+6])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+7])-0x30);
				}

				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*CE_303_TARIF2_POWER_H_REG, ((uint8_t)(temp>>24)) );
				fm25v02_write(2*CE_303_TARIF2_POWER_H_REG+1, ((uint8_t)(temp>>16)) );
				fm25v02_write(2*CE_303_TARIF2_POWER_L_REG, ((uint8_t)(temp>>8)) );
				fm25v02_write(2*CE_303_TARIF2_POWER_L_REG+1, ((uint8_t)temp) );
				fm25v02_write(2*CE_303_TARIF2_POWER_MIL_REG, ((uint8_t)(temp_mil>>8)) );
				fm25v02_write(2*CE_303_TARIF2_POWER_MIL_REG+1, ((uint8_t)(temp_mil)) );
				osMutexRelease(Fm25v02MutexHandle);
				status_registers.ce303_tarif2_power_h_reg = (uint16_t)(temp>>16);
				status_registers.ce303_tarif2_power_l_reg = (uint16_t)temp;
				status_registers.ce303_tarif2_power_mil_reg =  temp_mil;
				tarif_2_state = 1;

			}

			else if( meter_rx_buffer[i+5] == 0x2E) // четыре цифры перед запятой
			{
				temp = ((((uint16_t)meter_rx_buffer[i+1])-0x30)*1000) + ((((uint16_t)meter_rx_buffer[i+2])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+3])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+4])-0x30);

				if( meter_rx_buffer[i+7] == 0x29 ) // если 1 разряд после запятой
				{
					temp_mil = (((uint16_t)meter_rx_buffer[i+6])-0x30)*100;
				}
				else if( meter_rx_buffer[i+8] == 0x29 ) // если 2 разряда после запятой
				{
					temp_mil = (((((uint16_t)meter_rx_buffer[i+6])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+7])-0x30))*10;
				}
				else if( meter_rx_buffer[i+9] == 0x29 ) // если 3 разряда после запятой
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+6])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+7])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+8])-0x30);
				}
				//if( meter_rx_buffer[i+9] == 0x29 ) // если 4 разряда после запятой
				//{
					//temp_mil = ((((uint16_t)meter_rx_buffer[i+5])-0x30)*1000) + ((((uint16_t)meter_rx_buffer[i+6])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+7])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+8])-0x30);
				//}
				else
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+6])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+7])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+8])-0x30);
				}

				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*CE_303_TARIF2_POWER_H_REG, ((uint8_t)(temp>>24)) );
				fm25v02_write(2*CE_303_TARIF2_POWER_H_REG+1, ((uint8_t)(temp>>16)) );
				fm25v02_write(2*CE_303_TARIF2_POWER_L_REG, ((uint8_t)(temp>>8)) );
				fm25v02_write(2*CE_303_TARIF2_POWER_L_REG+1, ((uint8_t)temp) );
				fm25v02_write(2*CE_303_TARIF2_POWER_MIL_REG, ((uint8_t)(temp_mil>>8)) );
				fm25v02_write(2*CE_303_TARIF2_POWER_MIL_REG+1, ((uint8_t)(temp_mil)) );
				osMutexRelease(Fm25v02MutexHandle);
				status_registers.ce303_tarif2_power_h_reg = (uint16_t)(temp>>16);
				status_registers.ce303_tarif2_power_l_reg = (uint16_t)temp;
				status_registers.ce303_tarif2_power_mil_reg =  temp_mil;
				tarif_2_state = 1;

			}
		}

		else if( (meter_rx_buffer[i] == 0x28) && (tarif_general_state==1) && (tarif_1_state==1) && (tarif_2_state==1) && (tarif_3_state==0) && (tarif_4_state==0) && (tarif_5_state==0) ) // 0х28 означает символ "(" в ASCII таблице
		{
			if( meter_rx_buffer[i+2] == 0x2E) // 0х2Е означает символ "." в ASCII таблице, 1 цифра перед запятой
			{
				temp = (uint32_t)(meter_rx_buffer[i+1]-0x30);

				if( meter_rx_buffer[i+4] == 0x29 ) // если 1 разряд после запятой
				{
					temp_mil = (((uint16_t)meter_rx_buffer[i+3])-0x30)*100;
				}
				else if( meter_rx_buffer[i+5] == 0x29 ) // если 2 разряда после запятой
				{
					temp_mil = (((((uint16_t)meter_rx_buffer[i+3])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+4])-0x30))*10;
				}
				else if( meter_rx_buffer[i+6] == 0x29 ) // если 3 разряда после запятой
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+3])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+4])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+5])-0x30);
				}
				//if( meter_rx_buffer[i+7] == 0x29 ) // если 4 разряда после запятой
				//{
					//temp_mil = ((((uint16_t)meter_rx_buffer[i+3])-0x30)*1000) + ((((uint16_t)meter_rx_buffer[i+4])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+5])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+6])-0x30);
				//}
				else
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+3])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+4])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+5])-0x30);
				}

				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*CE_303_TARIF3_POWER_H_REG, ((uint8_t)(temp>>24)) );
				fm25v02_write(2*CE_303_TARIF3_POWER_H_REG+1, ((uint8_t)(temp>>16)) );
				fm25v02_write(2*CE_303_TARIF3_POWER_L_REG, ((uint8_t)(temp>>8)) );
				fm25v02_write(2*CE_303_TARIF3_POWER_L_REG+1, ((uint8_t)temp) );
				fm25v02_write(2*CE_303_TARIF3_POWER_MIL_REG, ((uint8_t)(temp_mil>>8)) );
				fm25v02_write(2*CE_303_TARIF3_POWER_MIL_REG+1, ((uint8_t)(temp_mil)) );
				osMutexRelease(Fm25v02MutexHandle);
				status_registers.ce303_tarif3_power_h_reg = (uint16_t)(temp>>16);
				status_registers.ce303_tarif3_power_l_reg = (uint16_t)temp;
				status_registers.ce303_tarif3_power_mil_reg =  temp_mil;
				tarif_3_state = 1;

			}

			else if( meter_rx_buffer[i+3] == 0x2E) // 2 цифры перед запятой
			{
				temp = ((((uint16_t)meter_rx_buffer[i+1])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+2])-0x30);

				if( meter_rx_buffer[i+5] == 0x29 ) // если 1 разряд после запятой
				{
					temp_mil = (((uint16_t)meter_rx_buffer[i+4])-0x30)*100;
				}
				else if( meter_rx_buffer[i+6] == 0x29 ) // если 2 разряда после запятой
				{
					temp_mil = (((((uint16_t)meter_rx_buffer[i+4])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+5])-0x30))*10;
				}
				else if( meter_rx_buffer[i+7] == 0x29 ) // если 3 разряда после запятой
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+4])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+5])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+6])-0x30);
				}
				//if( meter_rx_buffer[i+8] == 0x29 ) // если 4 разряда после запятой
				//{
					//temp_mil = ((((uint16_t)meter_rx_buffer[i+4])-0x30)*1000) + ((((uint16_t)meter_rx_buffer[i+5])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+6])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+7])-0x30);
				//}
				else
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+4])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+5])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+6])-0x30);
				}

				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*CE_303_TARIF3_POWER_H_REG, ((uint8_t)(temp>>24)) );
				fm25v02_write(2*CE_303_TARIF3_POWER_H_REG+1, ((uint8_t)(temp>>16)) );
				fm25v02_write(2*CE_303_TARIF3_POWER_L_REG, ((uint8_t)(temp>>8)) );
				fm25v02_write(2*CE_303_TARIF3_POWER_L_REG+1, ((uint8_t)temp) );
				fm25v02_write(2*CE_303_TARIF3_POWER_MIL_REG, ((uint8_t)(temp_mil>>8)) );
				fm25v02_write(2*CE_303_TARIF3_POWER_MIL_REG+1, ((uint8_t)(temp_mil)) );
				osMutexRelease(Fm25v02MutexHandle);
				status_registers.ce303_tarif3_power_h_reg = (uint16_t)(temp>>16);
				status_registers.ce303_tarif3_power_l_reg = (uint16_t)temp;
				status_registers.ce303_tarif3_power_mil_reg =  temp_mil;
				tarif_3_state = 1;

			}

			else if( meter_rx_buffer[i+4] == 0x2E) // 3 цифры перед запятой
			{
				temp = ((((uint16_t)meter_rx_buffer[i+1])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+2])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+3])-0x30);

				if( meter_rx_buffer[i+6] == 0x29 ) // если 1 разряд после запятой
				{
					temp_mil = (((uint16_t)meter_rx_buffer[i+5])-0x30)*100;
				}
				else if( meter_rx_buffer[i+7] == 0x29 ) // если 2 разряда после запятой
				{
					temp_mil = (((((uint16_t)meter_rx_buffer[i+5])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+6])-0x30))*10;
				}
				else if( meter_rx_buffer[i+8] == 0x29 ) // если 3 разряда после запятой
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+5])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+6])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+7])-0x30);
				}
				//if( meter_rx_buffer[i+9] == 0x29 ) // если 4 разряда после запятой
				//{
					//temp_mil = ((((uint16_t)meter_rx_buffer[i+5])-0x30)*1000) + ((((uint16_t)meter_rx_buffer[i+6])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+7])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+8])-0x30);
				//}
				else
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+5])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+6])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+7])-0x30);
				}

				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*CE_303_TARIF3_POWER_H_REG, ((uint8_t)(temp>>24)) );
				fm25v02_write(2*CE_303_TARIF3_POWER_H_REG+1, ((uint8_t)(temp>>16)) );
				fm25v02_write(2*CE_303_TARIF3_POWER_L_REG, ((uint8_t)(temp>>8)) );
				fm25v02_write(2*CE_303_TARIF3_POWER_L_REG+1, ((uint8_t)temp) );
				fm25v02_write(2*CE_303_TARIF3_POWER_MIL_REG, ((uint8_t)(temp_mil>>8)) );
				fm25v02_write(2*CE_303_TARIF3_POWER_MIL_REG+1, ((uint8_t)(temp_mil)) );
				osMutexRelease(Fm25v02MutexHandle);
				status_registers.ce303_tarif3_power_h_reg = (uint16_t)(temp>>16);
				status_registers.ce303_tarif3_power_l_reg = (uint16_t)temp;
				status_registers.ce303_tarif3_power_mil_reg =  temp_mil;
				tarif_3_state = 1;

			}

			else if( meter_rx_buffer[i+5] == 0x2E) // четыре цифры перед запятой
			{
				temp = ((((uint16_t)meter_rx_buffer[i+1])-0x30)*1000) + ((((uint16_t)meter_rx_buffer[i+2])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+3])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+4])-0x30);

				if( meter_rx_buffer[i+7] == 0x29 ) // если 1 разряд после запятой
				{
					temp_mil = (((uint16_t)meter_rx_buffer[i+6])-0x30)*100;
				}
				else if( meter_rx_buffer[i+8] == 0x29 ) // если 2 разряда после запятой
				{
					temp_mil = (((((uint16_t)meter_rx_buffer[i+6])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+7])-0x30))*10;
				}
				else if( meter_rx_buffer[i+9] == 0x29 ) // если 3 разряда после запятой
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+6])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+7])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+8])-0x30);
				}
				//if( meter_rx_buffer[i+9] == 0x29 ) // если 4 разряда после запятой
				//{
					//temp_mil = ((((uint16_t)meter_rx_buffer[i+5])-0x30)*1000) + ((((uint16_t)meter_rx_buffer[i+6])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+7])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+8])-0x30);
				//}
				else
				{
					temp_mil = ((((uint16_t)meter_rx_buffer[i+6])-0x30)*100) + ((((uint16_t)meter_rx_buffer[i+7])-0x30)*10) + (((uint16_t)meter_rx_buffer[i+8])-0x30);
				}

				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*CE_303_TARIF3_POWER_H_REG, ((uint8_t)(temp>>24)) );
				fm25v02_write(2*CE_303_TARIF3_POWER_H_REG+1, ((uint8_t)(temp>>16)) );
				fm25v02_write(2*CE_303_TARIF3_POWER_L_REG, ((uint8_t)(temp>>8)) );
				fm25v02_write(2*CE_303_TARIF3_POWER_L_REG+1, ((uint8_t)temp) );
				fm25v02_write(2*CE_303_TARIF3_POWER_MIL_REG, ((uint8_t)(temp_mil>>8)) );
				fm25v02_write(2*CE_303_TARIF3_POWER_MIL_REG+1, ((uint8_t)(temp_mil)) );
				osMutexRelease(Fm25v02MutexHandle);
				status_registers.ce303_tarif3_power_h_reg = (uint16_t)(temp>>16);
				status_registers.ce303_tarif3_power_l_reg = (uint16_t)temp;
				status_registers.ce303_tarif3_power_mil_reg =  temp_mil;
				tarif_3_state = 1;

			}
		}

		else if( (meter_rx_buffer[i] == 0x28) && (tarif_general_state==1) && (tarif_1_state==1) && (tarif_2_state==1) && (tarif_3_state==1) && (tarif_4_state==0) && (tarif_5_state==0) ) // 0х28 означает символ "(" в ASCII таблице
		{

		}

		else if( (meter_rx_buffer[i] == 0x28) && (tarif_general_state==1) && (tarif_1_state==1) && (tarif_2_state==1) && (tarif_3_state==1) && (tarif_4_state==1) && (tarif_5_state==0) ) // 0х28 означает символ "(" в ASCII таблице
		{

		}

	}

	meter_rx_number = 0;
	meter_rx_buffer_clear(); // очищаем буфер приема счетчика
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_10, GPIO_PIN_SET); // устанавливаем микросхему на передачу
	HAL_UART_Transmit_DMA(&huart6, &end_buf[0], 5); // передаем команду
	HAL_Delay(1000);

}

