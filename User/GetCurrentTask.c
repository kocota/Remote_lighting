#include "GetCurrentTask.h"
#include "cmsis_os.h"
#include "stm32f4xx_hal.h"
#include "gpio.h"
#include "modbus.h"
#include "fm25v02.h"

extern osThreadId GetCurrentTaskHandle;
extern ADC_HandleTypeDef hadc1;
extern osThreadId MainTaskHandle;
//extern ADC_HandleTypeDef hadc1;
//extern osThreadId LedTaskHandle;
extern osMutexId Fm25v02MutexHandle;
extern status_register_struct status_registers;
extern control_register_struct control_registers;

uint32_t data_in[3];

volatile uint8_t control_phase_a;
volatile uint8_t control_phase_b;
volatile uint8_t control_phase_c;


void ThreadGetCurrentTask(void const * argument)
{
	volatile uint16_t overcurrent_phase_a_state = 0;
	volatile uint16_t overcurrent_phase_b_state = 0;
	volatile uint16_t overcurrent_phase_c_state = 0;
	volatile uint8_t temp_h;
	volatile uint8_t temp_l;

	volatile uint8_t phase_temp;

	volatile uint8_t phase_a1_off_state=0;
	volatile uint8_t phase_a2_off_state=0;
	volatile uint8_t phase_b1_off_state=0;
	volatile uint8_t phase_b2_off_state=0;
	volatile uint8_t phase_c1_off_state=0;
	volatile uint8_t phase_c2_off_state=0;
	volatile uint8_t cascade_off_state=0;
	volatile uint8_t door_off_state=0;

	volatile uint8_t phase_a1_on_state=0;
	volatile uint8_t phase_a2_on_state=0;
	volatile uint8_t phase_b1_on_state=0;
	volatile uint8_t phase_b2_on_state=0;
	volatile uint8_t phase_c1_on_state=0;
	volatile uint8_t phase_c2_on_state=0;
	volatile uint8_t cascade_on_state=0;
	volatile uint8_t door_on_state=0;

	volatile uint16_t phase_a1_alarm_state=0;
	volatile uint16_t phase_a2_alarm_state=0;
	volatile uint16_t phase_b1_alarm_state=0;
	volatile uint16_t phase_b2_alarm_state=0;
	volatile uint16_t phase_c1_alarm_state=0;
	volatile uint16_t phase_c2_alarm_state=0;

	volatile uint8_t current_counter = 0;

	//volatile uint16_t overcurrent_a_counter = 0;
	//volatile uint16_t overcurrent_b_counter = 0;
	//volatile uint16_t overcurrent_c_counter = 0;

	uint16_t current_a_temp;
	uint16_t current_b_temp;
	uint16_t current_c_temp;

	uint16_t current_a_sum = 0;
	uint16_t current_b_sum = 0;
	uint16_t current_c_sum = 0;

	uint16_t current_a = 0;
	uint16_t current_b = 0;
	uint16_t current_c = 0;


	//HAL_Delay(10000);
	osThreadSuspend(GetCurrentTaskHandle);

	for(;;)
	{

		//switch(control_registers.lighting_switching_reg) // проверяем включена ли функция освещения
		//{
			//case(LIGHTING_ON): // если функция освещения включена, то делаем измерения тока

					HAL_ADCEx_InjectedStart(&hadc1);

					while( HAL_ADCEx_InjectedPollForConversion(&hadc1, 1) != HAL_OK )
					{

					}

					data_in[0] = HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_1);
					data_in[1] = HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_2);
					data_in[2] = HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_3);

					current_a_temp = data_in[0]*1000/4095; // 283
					current_b_temp = data_in[1]*1000/4095; // 283
					current_c_temp = data_in[2]*1000/4095; // 283

					current_a_sum = current_a_sum + current_a_temp;
					current_b_sum = current_b_sum + current_b_temp;
					current_c_sum = current_c_sum + current_c_temp;

					current_counter++;

					if(current_counter>=20)
					{
						current_a = current_a_sum/20;
						current_b = current_b_sum/20;
						current_c = current_c_sum/20;

						current_a_sum = 0;
						current_b_sum = 0;
						current_c_sum = 0;

						current_counter = 0;

						osMutexWait(Fm25v02MutexHandle, osWaitForever);
						fm25v02_write(2*CURRENT_PHASE_A_REG, (uint8_t)((current_a>>8)&0x00FF) );
						fm25v02_write(2*CURRENT_PHASE_A_REG+1, (uint8_t)(current_a&0x00FF) );
						fm25v02_write(2*CURRENT_PHASE_B_REG, (uint8_t)((current_b>>8)&0x00FF) );
						fm25v02_write(2*CURRENT_PHASE_B_REG+1, (uint8_t)(current_b&0x00FF) );
						fm25v02_write(2*CURRENT_PHASE_C_REG, (uint8_t)((current_c>>8)&0x00FF) );
						fm25v02_write(2*CURRENT_PHASE_C_REG+1, (uint8_t)(current_c&0x00FF) );
						osMutexRelease(Fm25v02MutexHandle);

						status_registers.current_phase_a_reg = current_a;
						status_registers.current_phase_b_reg = current_b;
						status_registers.current_phase_c_reg = current_c;

					}


					/*
					osMutexWait(Fm25v02MutexHandle, osWaitForever);
					fm25v02_write(2*CURRENT_PHASE_A_REG, (uint8_t)((current_a_temp>>8)&0x00FF) );
					fm25v02_write(2*CURRENT_PHASE_A_REG+1, (uint8_t)(current_a_temp&0x00FF) );
					fm25v02_write(2*CURRENT_PHASE_B_REG, (uint8_t)((current_b_temp>>8)&0x00FF) );
					fm25v02_write(2*CURRENT_PHASE_B_REG+1, (uint8_t)(current_b_temp&0x00FF) );
					fm25v02_write(2*CURRENT_PHASE_C_REG, (uint8_t)((current_c_temp>>8)&0x00FF) );
					fm25v02_write(2*CURRENT_PHASE_C_REG+1, (uint8_t)(current_c_temp&0x00FF) );
					osMutexRelease(Fm25v02MutexHandle);

					status_registers.current_phase_a_reg = current_a_temp;
					status_registers.current_phase_b_reg = current_b_temp;
					status_registers.current_phase_c_reg = current_c_temp;
					*/

				//if( ((control_registers.light_control_reg)&0x0001) == 0x0001 ) // если в управляющем регистре освещения выставлен бит включения фазы А
				//{

					if(status_registers.current_phase_a_reg > control_registers.max_current_phase_a) // проверяем если значение тока превысило максимальное значение тока фазы А
					{
						overcurrent_phase_a_state++;

						if(overcurrent_phase_a_state >= control_registers.overcurrent_timeleft_reg)
						{
							overcurrent_phase_a_state = 0;

							if( ((status_registers.lighting_alarm_reg)&0x0080) == 0x0000 ) // проверяем установлен ли бит превышения тока фазы А
							{
								osMutexWait(Fm25v02MutexHandle, osWaitForever);
								fm25v02_read(2*LIGHTING_ALARM_REG, &temp_h);
								fm25v02_read(2*LIGHTING_ALARM_REG+1, &temp_l);
								temp_l = temp_l|0x80;
								fm25v02_write(2*LIGHTING_ALARM_REG, temp_h);
								fm25v02_write(2*LIGHTING_ALARM_REG+1, temp_l);
								status_registers.lighting_alarm_reg = (((uint16_t)temp_h)<<8)|temp_l; // если превышение тока длилось 10 циклов, то выставляем бит превышения тока фазы А в регистр аварий

								fm25v02_read(2*LIGHT_CONTROL_REG, &temp_h);
								fm25v02_read(2*LIGHT_CONTROL_REG+1, &temp_l);
								temp_l = temp_l&0xFE;
								fm25v02_write(2*LIGHT_CONTROL_REG, temp_h);
								fm25v02_write(2*LIGHT_CONTROL_REG+1, temp_l);
								control_registers.light_control_reg = (((uint16_t)temp_h)<<8)|temp_l;

								osMutexRelease(Fm25v02MutexHandle);
							}
							else if( ((status_registers.lighting_alarm_reg)&0x0080) == 0x0080 ) // проверяем установлен ли бит превышения тока фазы А
							{
								if( (control_registers.light_control_reg)&0x01 )
								{
									osMutexWait(Fm25v02MutexHandle, osWaitForever);

									fm25v02_read(2*LIGHT_CONTROL_REG, &temp_h);
									fm25v02_read(2*LIGHT_CONTROL_REG+1, &temp_l);
									temp_l = temp_l&0xFE;
									fm25v02_write(2*LIGHT_CONTROL_REG, temp_h);
									fm25v02_write(2*LIGHT_CONTROL_REG+1, temp_l);
									control_registers.light_control_reg = (((uint16_t)temp_h)<<8)|temp_l;

									osMutexRelease(Fm25v02MutexHandle);
								}
							}

						}

					}

					else
					{
						overcurrent_phase_a_state = 0;
					}

				//}

				//if( ((control_registers.light_control_reg)&0x0002) == 0x0002 ) // если в управляющем регистре освещения выставлен бит включения фазы В
				//{

					if(status_registers.current_phase_b_reg > control_registers.max_current_phase_b) // проверяем если значение тока превысило максимальное значение тока фазы В
					{

						overcurrent_phase_b_state++;

						if(overcurrent_phase_b_state >= control_registers.overcurrent_timeleft_reg)
						{
							overcurrent_phase_b_state = 0;

							if( ((status_registers.lighting_alarm_reg)&0x0100) == 0x0000 ) // проверяем установлен ли бит превышения тока фазы В
							{
								osMutexWait(Fm25v02MutexHandle, osWaitForever);
								fm25v02_read(2*LIGHTING_ALARM_REG, &temp_h);
								fm25v02_read(2*LIGHTING_ALARM_REG+1, &temp_l);
								temp_h = temp_h|0x01;
								fm25v02_write(2*LIGHTING_ALARM_REG, temp_h);
								fm25v02_write(2*LIGHTING_ALARM_REG+1, temp_l);
								status_registers.lighting_alarm_reg = (((uint16_t)temp_h)<<8)|temp_l; // если превышение тока длилось 10 циклов, то выставляем бит превышения тока фазы В в регистр аварий

								fm25v02_read(2*LIGHT_CONTROL_REG, &temp_h);
								fm25v02_read(2*LIGHT_CONTROL_REG+1, &temp_l);
								temp_l = temp_l&0xFD;
								fm25v02_write(2*LIGHT_CONTROL_REG, temp_h);
								fm25v02_write(2*LIGHT_CONTROL_REG+1, temp_l);
								control_registers.light_control_reg = (((uint16_t)temp_h)<<8)|temp_l;

								osMutexRelease(Fm25v02MutexHandle);
							}

							else if( ((status_registers.lighting_alarm_reg)&0x0100) == 0x0100 ) // проверяем установлен ли бит превышения тока фазы А
							{
								if( (control_registers.light_control_reg)&0x02 )
								{
									osMutexWait(Fm25v02MutexHandle, osWaitForever);

									fm25v02_read(2*LIGHT_CONTROL_REG, &temp_h);
									fm25v02_read(2*LIGHT_CONTROL_REG+1, &temp_l);
									temp_l = temp_l&0xFD;
									fm25v02_write(2*LIGHT_CONTROL_REG, temp_h);
									fm25v02_write(2*LIGHT_CONTROL_REG+1, temp_l);
									control_registers.light_control_reg = (((uint16_t)temp_h)<<8)|temp_l;

									osMutexRelease(Fm25v02MutexHandle);
								}
							}

						}

					}

					else
					{
						overcurrent_phase_b_state = 0;
					}

				//}

				//if( ((control_registers.light_control_reg)&0x0004) == 0x0004 ) // если в управляющем регистре освещения выставлен бит включения фазы С
				//{

					if(status_registers.current_phase_c_reg > control_registers.max_current_phase_c) // проверяем если значение тока превысило максимальное значение тока фазы С
					{

						overcurrent_phase_c_state++;

						if(overcurrent_phase_c_state >= control_registers.overcurrent_timeleft_reg)
						{
							overcurrent_phase_c_state = 0;

							if( ((status_registers.lighting_alarm_reg)&0x0200) == 0x0000 ) // проверяем установлен ли бит превышения тока фазы С
							{
								osMutexWait(Fm25v02MutexHandle, osWaitForever);
								fm25v02_read(2*LIGHTING_ALARM_REG, &temp_h);
								fm25v02_read(2*LIGHTING_ALARM_REG+1, &temp_l);
								temp_h = temp_h|0x02;
								fm25v02_write(2*LIGHTING_ALARM_REG, temp_h);
								fm25v02_write(2*LIGHTING_ALARM_REG+1, temp_l);
								status_registers.lighting_alarm_reg = (((uint16_t)temp_h)<<8)|temp_l; // если превышение тока длилось 10 циклов, то выставляем бит превышения тока фазы А в регистр аварий

								fm25v02_read(2*LIGHT_CONTROL_REG, &temp_h);
								fm25v02_read(2*LIGHT_CONTROL_REG+1, &temp_l);
								temp_l = temp_l&0xFB;
								fm25v02_write(2*LIGHT_CONTROL_REG, temp_h);
								fm25v02_write(2*LIGHT_CONTROL_REG+1, temp_l);
								control_registers.light_control_reg = (((uint16_t)temp_h)<<8)|temp_l;

								osMutexRelease(Fm25v02MutexHandle);
							}

							else if( ((status_registers.lighting_alarm_reg)&0x0200) == 0x0200 ) // проверяем установлен ли бит превышения тока фазы А
							{
								if( (control_registers.light_control_reg)&0x04 )
								{
									osMutexWait(Fm25v02MutexHandle, osWaitForever);

									fm25v02_read(2*LIGHT_CONTROL_REG, &temp_h);
									fm25v02_read(2*LIGHT_CONTROL_REG+1, &temp_l);
									temp_l = temp_l&0xFB;
									fm25v02_write(2*LIGHT_CONTROL_REG, temp_h);
									fm25v02_write(2*LIGHT_CONTROL_REG+1, temp_l);
									control_registers.light_control_reg = (((uint16_t)temp_h)<<8)|temp_l;

									osMutexRelease(Fm25v02MutexHandle);
								}
							}

						}

					}

					else
					{
						overcurrent_phase_c_state = 0;
					}

				//}

//----контроль фаз, двери и каскада---------------------------------------------------------------------------------------------------------------------

				if( HAL_GPIO_ReadPin(GPIOH, GPIO_PIN_4) == GPIO_PIN_RESET ) // если нет наличия фазы А1
				{
					if( ((control_registers.light_control_reg)&0x0001) == 0x0001 ) // если в управляющем регистре освещения выставлен бит включения фазы А
					{
						if(phase_a1_alarm_state<3000)
						{
							phase_a1_alarm_state++;

							if(phase_a1_alarm_state==3000)
							{
								phase_a1_alarm_state = 0;

								if( ((status_registers.lighting_alarm_reg)&0x0001) == 0x0000 ) // проверяем наличие бита аварии фазы А1, если бит не выставлен, устанавливаем бит
								{
									osMutexWait(Fm25v02MutexHandle, osWaitForever);
									fm25v02_read(2*LIGHTING_ALARM_REG+1, &phase_temp); // читаем значение из старшего байта регистра аварий освещения
									phase_temp = phase_temp|0x01; // устанавливаем 0-й бит статуса фазы А1
									fm25v02_write(2*LIGHTING_ALARM_REG+1, phase_temp); // записываем младший байт регистра аварий освещения
									status_registers.lighting_alarm_reg = status_registers.lighting_alarm_reg|0x0001; // выставляем бит аварии фазы А1 в переменной регистра аварий освещения
									osMutexRelease(Fm25v02MutexHandle);
								}

							}
						}
					}

					phase_a1_on_state = 0;

					if(phase_a1_off_state<200)
					{
						phase_a1_off_state++;

						if(phase_a1_off_state >= 200)
						{
							phase_a1_off_state = 0;
							if( ((status_registers.lighting_status_reg)&0x0001) == 0x0001 )
							{
								osMutexWait(Fm25v02MutexHandle, osWaitForever);
								fm25v02_read(2*LIGHTING_STATUS_REG+1, &phase_temp);
								phase_temp = phase_temp&0xFE;
								fm25v02_write(2*LIGHTING_STATUS_REG+1, phase_temp);
								status_registers.lighting_status_reg = status_registers.lighting_status_reg&0xFFFE;
								osMutexRelease(Fm25v02MutexHandle);
								LED3_OFF();
							}
						}
					}

				}

				else if(HAL_GPIO_ReadPin(GPIOH, GPIO_PIN_4) == GPIO_PIN_SET) // если есть наличие фазы А1
				{
					phase_a1_alarm_state = 0;
					phase_a1_off_state = 0;

					if(phase_a1_on_state<5)
					{
						phase_a1_on_state++;
						if(phase_a1_on_state>=5)
						{

							phase_a1_on_state = 0; // выставляем среднее значение между 0 и 10

							if( ((status_registers.lighting_status_reg)&0x0001) == 0x0000 )
							{
								osMutexWait(Fm25v02MutexHandle, osWaitForever);
								fm25v02_read(2*LIGHTING_STATUS_REG+1, &phase_temp); // читаем значение регистра статуса освещения
								phase_temp = phase_temp|0x01; // устанавливаем 0-й бит статуса фазы А1
								fm25v02_write(2*LIGHTING_STATUS_REG+1, phase_temp); // записываем младший байт регистра статуса освещения
								status_registers.lighting_status_reg = status_registers.lighting_status_reg|0x0001; // выставляем бит фазы А1 в переменной регистра статуса освещения
								osMutexRelease(Fm25v02MutexHandle);
								LED3_ON();
							}

						}


					}

				}

				if( HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_11) == GPIO_PIN_RESET ) // если нет наличия фазы А2
				{
					if( (((control_registers.light_control_reg)&0x0010) == 0x0010) && (((control_registers.light_control_reg)&0x0001) == 0x0001) ) // если контроль второй линии включен и если в управляющем регистре освещения выставлен бит включения фазы А
					{
						if(phase_a2_alarm_state<3000)
						{
							phase_a2_alarm_state++;
							if(phase_a2_alarm_state==3000)
							{
								phase_a2_alarm_state = 0; // выставляем среднее значение между 0 и 10

								if( ((status_registers.lighting_alarm_reg)&0x0008) == 0x0000 ) // проверяем наличие бита аварии фазы А1, если бит не выставлен, устанавливаем бит
								{
									osMutexWait(Fm25v02MutexHandle, osWaitForever);
									fm25v02_read(2*LIGHTING_ALARM_REG+1, &phase_temp); // читаем значение из старшего байта регистра аварий освещения
									phase_temp = phase_temp|0x08; // устанавливаем 0-й бит статуса фазы А1
									fm25v02_write(2*LIGHTING_ALARM_REG+1, phase_temp); // записываем младший байт регистра аварий освещения
									status_registers.lighting_alarm_reg = status_registers.lighting_alarm_reg|0x0008; // выставляем бит аварии фазы А1 в переменной регистра аварий освещения
									osMutexRelease(Fm25v02MutexHandle);
								}

							}
						}
					}

					phase_a2_on_state = 0;

					if(phase_a2_off_state<200)
					{
						phase_a2_off_state++;
						if(phase_a2_off_state >= 200)
						{
							phase_a2_off_state = 0;
							if( ((status_registers.lighting_status_reg)&0x0008) == 0x0008 )
							{
								osMutexWait(Fm25v02MutexHandle, osWaitForever);
								fm25v02_read(2*LIGHTING_STATUS_REG+1, &phase_temp);
								phase_temp = phase_temp&0xF7;
								fm25v02_write(2*LIGHTING_STATUS_REG+1, phase_temp);
								status_registers.lighting_status_reg = status_registers.lighting_status_reg&0xFFF7;
								osMutexRelease(Fm25v02MutexHandle);
								LED6_OFF();
							}
						}
					}

				}
				else if(HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_11) == GPIO_PIN_SET) // если есть наличие фазы А2
				{

					phase_a2_alarm_state = 0;
					phase_a2_off_state = 0;

					if(phase_a2_on_state<5)
					{
						phase_a2_on_state++;
						if(phase_a2_on_state>=5)
						{

							phase_a2_on_state = 0; // выставляем среднее значение между 0 и 10

							if( ((status_registers.lighting_status_reg)&0x0008) == 0x0000 )
							{
								osMutexWait(Fm25v02MutexHandle, osWaitForever);
								fm25v02_read(2*LIGHTING_STATUS_REG+1, &phase_temp); // читаем значение регистра статуса освещения
								phase_temp = phase_temp|0x08; // устанавливаем 0-й бит статуса фазы А1
								fm25v02_write(2*LIGHTING_STATUS_REG+1, phase_temp); // записываем младший байт регистра статуса освещения
								status_registers.lighting_status_reg = status_registers.lighting_status_reg|0x0008; // выставляем бит фазы А1 в переменной регистра статуса освещения
								osMutexRelease(Fm25v02MutexHandle);
								LED6_ON();
							}

						}
					}

				}


				if( HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_12) == GPIO_PIN_RESET ) // если нет наличия фазы В1
				{
					if( ((control_registers.light_control_reg)&0x0002) == 0x0002 ) // если в управляющем регистре освещения выставлен бит включения фазы В
					{
						if(phase_b1_alarm_state<3000)
						{
							phase_b1_alarm_state++;
							if(phase_b1_alarm_state==3000)
							{
								phase_b1_alarm_state = 0; // выставляем среднее значение между 0 и 10

								if( ((status_registers.lighting_alarm_reg)&0x0002) == 0x0000 ) // проверяем наличие бита аварии фазы А1, если бит не выставлен, устанавливаем бит
								{
									osMutexWait(Fm25v02MutexHandle, osWaitForever);
									fm25v02_read(2*LIGHTING_ALARM_REG+1, &phase_temp); // читаем значение из старшего байта регистра аварий освещения
									phase_temp = phase_temp|0x02; // устанавливаем 0-й бит статуса фазы А1
									fm25v02_write(2*LIGHTING_ALARM_REG+1, phase_temp); // записываем младший байт регистра аварий освещения
									status_registers.lighting_alarm_reg = status_registers.lighting_alarm_reg|0x0002; // выставляем бит аварии фазы А1 в переменной регистра аварий освещения
									osMutexRelease(Fm25v02MutexHandle);
								}

							}
						}
					}

					phase_b1_on_state = 0;

					if(phase_b1_off_state<200)
					{
						phase_b1_off_state++;
						if(phase_b1_off_state >= 200)
						{
							phase_b1_off_state = 0;
							if( ((status_registers.lighting_status_reg)&0x0002) == 0x0002 )
							{
								osMutexWait(Fm25v02MutexHandle, osWaitForever);
								fm25v02_read(2*LIGHTING_STATUS_REG+1, &phase_temp);
								phase_temp = phase_temp&0xFD;
								fm25v02_write(2*LIGHTING_STATUS_REG+1, phase_temp);
								status_registers.lighting_status_reg = status_registers.lighting_status_reg&0xFFFD;
								osMutexRelease(Fm25v02MutexHandle);
								LED4_OFF();
							}
						}
					}
				}
				else if(HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_12) == GPIO_PIN_SET) // если есть наличие фазы В1
				{
					phase_b1_alarm_state = 0;
					phase_b1_off_state = 0;

					if(phase_b1_on_state<5)
					{
						phase_b1_on_state++;
						if(phase_b1_on_state>=5)
						{

							phase_b1_on_state = 0; // выставляем среднее значение между 0 и 10

							if( ((status_registers.lighting_status_reg)&0x0002) == 0x0000 )
							{
								osMutexWait(Fm25v02MutexHandle, osWaitForever);
								fm25v02_read(2*LIGHTING_STATUS_REG+1, &phase_temp); // читаем значение регистра статуса освещения
								phase_temp = phase_temp|0x02; // устанавливаем 0-й бит статуса фазы А1
								fm25v02_write(2*LIGHTING_STATUS_REG+1, phase_temp); // записываем младший байт регистра статуса освещения
								status_registers.lighting_status_reg = status_registers.lighting_status_reg|0x0002; // выставляем бит фазы А1 в переменной регистра статуса освещения
								osMutexRelease(Fm25v02MutexHandle);
								LED4_ON();
							}

						}
					}
				}

				if( HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_13) == GPIO_PIN_RESET ) // если нет наличия фазы В2
				{
					if( (((control_registers.light_control_reg)&0x0010) == 0x0010) && (((control_registers.light_control_reg)&0x0002) == 0x0002) ) // если контроль второй линии включен и если в управляющем регистре освещения выставлен бит включения фазы В
					{
						if(phase_b2_alarm_state<3000)
						{
							phase_b2_alarm_state++;
							if(phase_b2_alarm_state==3000)
							{
								phase_b2_alarm_state = 0; // выставляем среднее значение между 0 и 10

								if( ((status_registers.lighting_alarm_reg)&0x0010) == 0x0000 ) // проверяем наличие бита аварии фазы А1, если бит не выставлен, устанавливаем бит
								{
									osMutexWait(Fm25v02MutexHandle, osWaitForever);
									fm25v02_read(2*LIGHTING_ALARM_REG+1, &phase_temp); // читаем значение из старшего байта регистра аварий освещения
									phase_temp = phase_temp|0x10; // устанавливаем 0-й бит статуса фазы А1
									fm25v02_write(2*LIGHTING_ALARM_REG+1, phase_temp); // записываем младший байт регистра аварий освещения
									status_registers.lighting_alarm_reg = status_registers.lighting_alarm_reg|0x0010; // выставляем бит аварии фазы А1 в переменной регистра аварий освещения
									osMutexRelease(Fm25v02MutexHandle);
								}

							}
						}
					}

					phase_b2_on_state = 0;

					if(phase_b2_off_state<200)
					{
						phase_b2_off_state++;
						if(phase_b2_off_state >= 200)
						{
							phase_b2_off_state = 0;
							if( ((status_registers.lighting_status_reg)&0x0010) == 0x0010 )
							{
								osMutexWait(Fm25v02MutexHandle, osWaitForever);
								fm25v02_read(2*LIGHTING_STATUS_REG+1, &phase_temp);
								phase_temp = phase_temp&0xEF;
								fm25v02_write(2*LIGHTING_STATUS_REG+1, phase_temp);
								status_registers.lighting_status_reg = status_registers.lighting_status_reg&0xFFEF;
								osMutexRelease(Fm25v02MutexHandle);
								LED7_OFF();
							}
						}
					}
				}
				else if(HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_13) == GPIO_PIN_SET) // если есть наличие фазы В2
				{
					phase_b2_alarm_state = 0;
					phase_b2_off_state = 0;

					if(phase_b2_on_state<5)
					{
						phase_b2_on_state++;
						if(phase_b2_on_state>=5)
						{

							phase_b2_on_state = 0; // выставляем среднее значение между 0 и 10

							if( ((status_registers.lighting_status_reg)&0x0010) == 0x0000 )
							{
								osMutexWait(Fm25v02MutexHandle, osWaitForever);
								fm25v02_read(2*LIGHTING_STATUS_REG+1, &phase_temp); // читаем значение регистра статуса освещения
								phase_temp = phase_temp|0x10; // устанавливаем 0-й бит статуса фазы А1
								fm25v02_write(2*LIGHTING_STATUS_REG+1, phase_temp); // записываем младший байт регистра статуса освещения
								status_registers.lighting_status_reg = status_registers.lighting_status_reg|0x0010; // выставляем бит фазы А1 в переменной регистра статуса освещения
								osMutexRelease(Fm25v02MutexHandle);
								LED7_ON();
							}

						}
					}
				}


				if( HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_14) == GPIO_PIN_RESET ) // если нет наличия фазы С1
				{
					if( ((control_registers.light_control_reg)&0x0004) == 0x0004 ) // если в управляющем регистре освещения выставлен бит включения фазы С
					{
						if(phase_c1_alarm_state<3000)
						{
							phase_c1_alarm_state++;
							if(phase_c1_alarm_state==3000)
							{
								phase_c1_alarm_state = 0; // выставляем среднее значение между 0 и 10

								if( ((status_registers.lighting_alarm_reg)&0x0004) == 0x0000 ) // проверяем наличие бита аварии фазы А1, если бит не выставлен, устанавливаем бит
								{
									osMutexWait(Fm25v02MutexHandle, osWaitForever);
									fm25v02_read(2*LIGHTING_ALARM_REG+1, &phase_temp); // читаем значение из старшего байта регистра аварий освещения
									phase_temp = phase_temp|0x04; // устанавливаем 0-й бит статуса фазы А1
									fm25v02_write(2*LIGHTING_ALARM_REG+1, phase_temp); // записываем младший байт регистра аварий освещения
									status_registers.lighting_alarm_reg = status_registers.lighting_alarm_reg|0x0004; // выставляем бит аварии фазы А1 в переменной регистра аварий освещения
									osMutexRelease(Fm25v02MutexHandle);
								}

							}
						}
					}

					phase_c1_on_state = 0;

					if(phase_c1_off_state<200)
					{
						phase_c1_off_state++;
						if(phase_c1_off_state >= 200)
						{
							phase_c1_off_state = 0;
							if( ((status_registers.lighting_status_reg)&0x0004) == 0x0004 )
							{
								osMutexWait(Fm25v02MutexHandle, osWaitForever);
								fm25v02_read(2*LIGHTING_STATUS_REG+1, &phase_temp);
								phase_temp = phase_temp&0xFB;
								fm25v02_write(2*LIGHTING_STATUS_REG+1, phase_temp);
								status_registers.lighting_status_reg = status_registers.lighting_status_reg&0xFFFB;
								osMutexRelease(Fm25v02MutexHandle);
								LED5_OFF();
							}
						}
					}
				}

				else if(HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_14) == GPIO_PIN_SET) // если есть наличие фазы С1
				{
					phase_c1_alarm_state = 0;
					phase_c1_off_state = 0;

					if(phase_c1_on_state<5)
					{
						phase_c1_on_state++;
						if(phase_c1_on_state>=5)
						{

							phase_c1_on_state = 0; // выставляем среднее значение между 0 и 10

							if( ((status_registers.lighting_status_reg)&0x0004) == 0x0000 )
							{
								osMutexWait(Fm25v02MutexHandle, osWaitForever);
								fm25v02_read(2*LIGHTING_STATUS_REG+1, &phase_temp); // читаем значение регистра статуса освещения
								phase_temp = phase_temp|0x04; // устанавливаем 0-й бит статуса фазы А1
								fm25v02_write(2*LIGHTING_STATUS_REG+1, phase_temp); // записываем младший байт регистра статуса освещения
								status_registers.lighting_status_reg = status_registers.lighting_status_reg|0x0004; // выставляем бит фазы А1 в переменной регистра статуса освещения
								osMutexRelease(Fm25v02MutexHandle);
								LED5_ON();
							}

						}
					}
				}


				if( HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_15) == GPIO_PIN_RESET ) // если нет наличия фазы С2
				{
					if( (((control_registers.light_control_reg)&0x0010) == 0x0010) && (((control_registers.light_control_reg)&0x0004) == 0x0004) ) // если контроль второй линии включен и если в управляющем регистре освещения выставлен бит включения фазы С
					{
						if(phase_c2_alarm_state<3000)
						{
							phase_c2_alarm_state++;
							if(phase_c2_alarm_state==3000)
							{
								phase_c2_alarm_state = 0; // выставляем среднее значение между 0 и 10

								if( ((status_registers.lighting_alarm_reg)&0x0020) == 0x0000 ) // проверяем наличие бита аварии фазы А1, если бит не выставлен, устанавливаем бит
								{
									osMutexWait(Fm25v02MutexHandle, osWaitForever);
									fm25v02_read(2*LIGHTING_ALARM_REG+1, &phase_temp); // читаем значение из старшего байта регистра аварий освещения
									phase_temp = phase_temp|0x20; // устанавливаем 0-й бит статуса фазы А1
									fm25v02_write(2*LIGHTING_ALARM_REG+1, phase_temp); // записываем младший байт регистра аварий освещения
									status_registers.lighting_alarm_reg = status_registers.lighting_alarm_reg|0x0020; // выставляем бит аварии фазы А1 в переменной регистра аварий освещения
									osMutexRelease(Fm25v02MutexHandle);
								}

							}
						}
					}

					phase_c2_on_state = 0;

					if(phase_c2_off_state<200)
					{
						phase_c2_off_state++;
						if(phase_c2_off_state >= 200)
						{
							phase_c2_off_state = 0;
							if( ((status_registers.lighting_status_reg)&0x0020) == 0x0020 )
							{
								osMutexWait(Fm25v02MutexHandle, osWaitForever);
								fm25v02_read(2*LIGHTING_STATUS_REG+1, &phase_temp);
								phase_temp = phase_temp&0xDF;
								fm25v02_write(2*LIGHTING_STATUS_REG+1, phase_temp);
								status_registers.lighting_status_reg = status_registers.lighting_status_reg&0xFFDF;
								osMutexRelease(Fm25v02MutexHandle);
								LED8_OFF();
							}
						}
					}
				}
				else if(HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_15) == GPIO_PIN_SET) // если нет наличия фазы С2
				{
					phase_c2_alarm_state = 0;
					phase_c2_off_state = 0;

					if(phase_c2_on_state<5)
					{
						phase_c2_on_state++;
						if(phase_c2_on_state>=5)
						{

							phase_c2_on_state = 0; // выставляем среднее значение между 0 и 10

							if( ((status_registers.lighting_status_reg)&0x0020) == 0x0000 )
							{
								osMutexWait(Fm25v02MutexHandle, osWaitForever);
								fm25v02_read(2*LIGHTING_STATUS_REG+1, &phase_temp); // читаем значение регистра статуса освещения
								phase_temp = phase_temp|0x20; // устанавливаем 0-й бит статуса фазы А1
								fm25v02_write(2*LIGHTING_STATUS_REG+1, phase_temp); // записываем младший байт регистра статуса освещения
								status_registers.lighting_status_reg = status_registers.lighting_status_reg|0x0020; // выставляем бит фазы А1 в переменной регистра статуса освещения
								osMutexRelease(Fm25v02MutexHandle);
								LED8_ON();
							}

						}
					}
				}



				if( HAL_GPIO_ReadPin(GPIOG, GPIO_PIN_0) == GPIO_PIN_RESET ) // если пин каскада сброшен
				{

					//cascade_alarm_state = 0;

					if(cascade_off_state<50)
					{
						cascade_off_state++;
						if(cascade_off_state == 50)
						{
							cascade_off_state = 0;
							if( ((status_registers.lighting_status_reg)&0x0040) == 0x0040 )
							{
								osMutexWait(Fm25v02MutexHandle, osWaitForever);
								fm25v02_read(2*LIGHTING_STATUS_REG+1, &phase_temp);
								phase_temp = phase_temp&0xBF;
								fm25v02_write(2*LIGHTING_STATUS_REG+1, phase_temp);
								status_registers.lighting_status_reg = (status_registers.lighting_status_reg)&0xFFBF;
								osMutexRelease(Fm25v02MutexHandle);
							}

							if( ((control_registers.light_control_reg)&0x0008) == 0x0008 ) // если контроль каскада включен
							{
								if( ((control_registers.light_control_reg)&0x0001) == 0x0001 )
								{
									osMutexWait(Fm25v02MutexHandle, osWaitForever);
									fm25v02_read(2*LIGHT_CONTROL_REG+1, &phase_temp); // читаем значение из старшего байта регистра управления освещением
									phase_temp = phase_temp&0xFE;
									fm25v02_write(2*LIGHT_CONTROL_REG+1, phase_temp); // записываем младший байт регистра управления освещением
									control_registers.light_control_reg = (control_registers.light_control_reg)&0xFFFE;
									osMutexRelease(Fm25v02MutexHandle);
								}
								if( ((control_registers.light_control_reg)&0x0002) == 0x0002 )
								{
									osMutexWait(Fm25v02MutexHandle, osWaitForever);
									fm25v02_read(2*LIGHT_CONTROL_REG+1, &phase_temp); // читаем значение из старшего байта регистра управления освещением
									phase_temp = phase_temp&0xFD;
									fm25v02_write(2*LIGHT_CONTROL_REG+1, phase_temp); // записываем младший байт регистра управления освещением
									control_registers.light_control_reg = (control_registers.light_control_reg)&0xFFFD;
									osMutexRelease(Fm25v02MutexHandle);
								}
								if( ((control_registers.light_control_reg)&0x0004) == 0x0004 )
								{
									osMutexWait(Fm25v02MutexHandle, osWaitForever);
									fm25v02_read(2*LIGHT_CONTROL_REG+1, &phase_temp); // читаем значение из старшего байта регистра управления освещением
									phase_temp = phase_temp&0xFB;
									fm25v02_write(2*LIGHT_CONTROL_REG+1, phase_temp); // записываем младший байт регистра управления освещением
									control_registers.light_control_reg = (control_registers.light_control_reg)&0xFFFB;
									osMutexRelease(Fm25v02MutexHandle);
								}
							}

						}
					}
				}
				else if(HAL_GPIO_ReadPin(GPIOG, GPIO_PIN_0) == GPIO_PIN_SET) // если пин каскада установлен
				{

					cascade_off_state = 0;

					if(cascade_on_state<10)
					{
						cascade_on_state++;
						if(cascade_on_state==10)
						{
							cascade_on_state = 0;

							if( ((status_registers.lighting_status_reg)&0x0040) == 0x0000 )
							{
								osMutexWait(Fm25v02MutexHandle, osWaitForever);
								fm25v02_read(2*LIGHTING_STATUS_REG+1, &phase_temp); // читаем значение регистра статуса освещения
								phase_temp = phase_temp|0x40;
								fm25v02_write(2*LIGHTING_STATUS_REG+1, phase_temp); // записываем младший байт регистра статуса освещения
								status_registers.lighting_status_reg = status_registers.lighting_status_reg|0x0040;
								osMutexRelease(Fm25v02MutexHandle);
							}

							if( ((control_registers.light_control_reg)&0x0008) == 0x0008 ) // если контроль каскада включен
							{
								if( ((control_registers.light_control_reg)&0x0001) == 0x0000 )
								{
									osMutexWait(Fm25v02MutexHandle, osWaitForever);
									fm25v02_read(2*LIGHT_CONTROL_REG+1, &phase_temp); // читаем значение из старшего байта регистра управления освещением
									phase_temp = phase_temp|0x01;
									fm25v02_write(2*LIGHT_CONTROL_REG+1, phase_temp); // записываем младший байт регистра управления освещением
									control_registers.light_control_reg = control_registers.light_control_reg|0x01;
									osMutexRelease(Fm25v02MutexHandle);
								}
								if( ((control_registers.light_control_reg)&0x0002) == 0x0000 )
								{
									osMutexWait(Fm25v02MutexHandle, osWaitForever);
									fm25v02_read(2*LIGHT_CONTROL_REG+1, &phase_temp); // читаем значение из старшего байта регистра управления освещением
									phase_temp = phase_temp|0x02;
									fm25v02_write(2*LIGHT_CONTROL_REG+1, phase_temp); // записываем младший байт регистра управления освещением
									control_registers.light_control_reg = control_registers.light_control_reg|0x02;
									osMutexRelease(Fm25v02MutexHandle);
								}
								if( ((control_registers.light_control_reg)&0x0004) == 0x0000 )
								{
									osMutexWait(Fm25v02MutexHandle, osWaitForever);
									fm25v02_read(2*LIGHT_CONTROL_REG+1, &phase_temp); // читаем значение из старшего байта регистра управления освещением
									phase_temp = phase_temp|0x04;
									fm25v02_write(2*LIGHT_CONTROL_REG+1, phase_temp); // записываем младший байт регистра управления освещением
									control_registers.light_control_reg = control_registers.light_control_reg|0x04;
									osMutexRelease(Fm25v02MutexHandle);
								}
							}

						}
					}
				}

				if( HAL_GPIO_ReadPin(GPIOG, GPIO_PIN_1) == GPIO_PIN_RESET ) // если сброшен пин двери освещения
				{

					if(door_off_state<50)
					{
						door_off_state++;
						if(door_off_state == 50)
						{
							door_off_state = 0;
							if( ((status_registers.lighting_status_reg)&0x0080) == 0x0080 )
							{
								osMutexWait(Fm25v02MutexHandle, osWaitForever);
								fm25v02_read(2*LIGHTING_STATUS_REG+1, &phase_temp);
								phase_temp = phase_temp&0x7F;
								fm25v02_write(2*LIGHTING_STATUS_REG+1, phase_temp);
								status_registers.lighting_status_reg = status_registers.lighting_status_reg&0xFF7F;
								osMutexRelease(Fm25v02MutexHandle);
							}
						}
					}
				}
				else if(HAL_GPIO_ReadPin(GPIOG, GPIO_PIN_1) == GPIO_PIN_SET) // если установлен пин двери освещения
				{

					door_off_state = 0;

					if(door_on_state<10)
					{
						door_on_state++;

						if(door_on_state==10)
						{
							door_on_state = 0;

							if( ((status_registers.lighting_status_reg)&0x0080) == 0x0000 )
							{
								osMutexWait(Fm25v02MutexHandle, osWaitForever);
								fm25v02_read(2*LIGHTING_STATUS_REG+1, &phase_temp);
								phase_temp = phase_temp|0x80;
								fm25v02_write(2*LIGHTING_STATUS_REG+1, phase_temp);
								status_registers.lighting_status_reg = status_registers.lighting_status_reg|0x0080;
								osMutexRelease(Fm25v02MutexHandle);
							}

						}
					}
				}
//---------------------------------------------------------------------------------------------------------------------------------------------------------


			//break;

		//}

		osDelay(1);
	}
}
