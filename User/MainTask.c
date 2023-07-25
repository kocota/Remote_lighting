#include "MainTask.h"
#include "modbus.h"
#include "cmsis_os.h"
#include "gpio.h"
#include "fm25v02.h"
#include "m95.h"
#include "tsl2561.h"

extern osThreadId MainTaskHandle;
extern osThreadId EventWriteTaskHandle;
extern osTimerId Ring_Center_TimerHandle;
extern RTC_HandleTypeDef hrtc;
extern osThreadId LedTaskHandle;
extern osMutexId Fm25v02MutexHandle;
extern status_register_struct status_registers;
extern control_register_struct control_registers;

extern volatile uint8_t phase_a_control_state; // переменная статуса включения фазы А
extern volatile uint8_t phase_b_control_state; // переменная статуса включения фазы В
extern volatile uint8_t phase_c_control_state; // переменная статуса включения фазы С

extern volatile uint32_t cur_a;
extern volatile uint32_t cur_b;
extern volatile uint32_t cur_c;

extern  double data_lux;
extern bool gain;        // Gain setting, 0 = X1, 1 = X16;
extern unsigned char time;
extern unsigned int ms;
extern unsigned int data0, data2;
extern double lux;    // Resulting lux value

RTC_TimeTypeDef current_time;
RTC_DateTypeDef current_date;
RTC_TimeTypeDef set_time;
RTC_DateTypeDef set_date;

uint16_t status_registers_quantity = 58; // количество статусных регистров

volatile uint8_t security_control_temp = 0;
volatile uint8_t security_state_temp = 0;

uint8_t temp_time_on_1_hour;
uint8_t temp_time_on_1_minute;
uint8_t temp_time_on_2_hour;
uint8_t temp_time_on_2_minute;
uint8_t temp_time_on_3_hour;
uint8_t temp_time_on_3_minute;
uint8_t temp_time_on_4_hour;
uint8_t temp_time_on_4_minute;
uint8_t temp_time_off_1_hour;
uint8_t temp_time_off_1_minute;
uint8_t temp_time_off_2_hour;
uint8_t temp_time_off_2_minute;
uint8_t temp_time_off_3_hour;
uint8_t temp_time_off_3_minute;
uint8_t temp_time_off_4_hour;
uint8_t temp_time_off_4_minute;

volatile uint16_t time_on_counter = 0;
volatile uint16_t time_off_counter = 0;
volatile uint8_t on_state = 0;
volatile uint8_t off_state = 0;

bool d_state1;
bool d_state2;
bool d_state3;
bool d_state4;
unsigned char sensor_id;



void ThreadMainTask(void const * argument)
{
	uint8_t temp;





	osThreadSuspend(MainTaskHandle); // ждем пока не будут вычитаны регистры и не получен статус фаз А1,А2,В1,В2,С1,С2

	time_on_counter = (control_registers.delay_on_sensor_reg + 1); // выставляем значения, чтобы не происходило переключений освещения после сброса контроллера
	time_off_counter = (control_registers.delay_off_sensor_reg + 1);


	for(;;)
	{

		if( HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_0) == GPIO_PIN_SET ) // проверяем если есть наличие единицы на пине PFO микросхемы TPS3306-15
		{
			if(status_registers.power_on_reg == 0) // если основного питания до этого не было, записываем в регистр наличия питания 1
			{
				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*POWER_ON_REG, 0x00);
				fm25v02_write(2*POWER_ON_REG+1, 1);
				status_registers.power_on_reg = 1;
				osMutexRelease(Fm25v02MutexHandle);

				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*GPRS_CALL_REG, 0x00);
				fm25v02_write(2*GPRS_CALL_REG+1, CALL_ON);
				osMutexRelease(Fm25v02MutexHandle);

				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*SYSTEM_STATUS_REG, 0x00);
				fm25v02_write(2*SYSTEM_STATUS_REG+1, POWER_ON);
				status_registers.system_status_reg = POWER_ON;
				osMutexRelease(Fm25v02MutexHandle);

				osThreadResume(EventWriteTaskHandle);
			}

			if(status_registers.power_on_lighting_reg == 0) // если основного питания до этого не было, записываем в регистр наличия питания освещения 1
			{
				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*POWER_ON_LIGHTING_REG, 0x00);
				fm25v02_write(2*POWER_ON_LIGHTING_REG+1, 1);
				status_registers.power_on_lighting_reg = 1;
				osMutexRelease(Fm25v02MutexHandle);

				//osMutexWait(Fm25v02MutexHandle, osWaitForever);
				//fm25v02_write(2*GPRS_CALL_REG, 0x00);
				//fm25v02_write(2*GPRS_CALL_REG+1, CALL_ON);
				//osMutexRelease(Fm25v02MutexHandle);

				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*SYSTEM_STATUS_REG, 0x00);
				fm25v02_write(2*SYSTEM_STATUS_REG+1, POWER_ON);
				status_registers.system_status_reg = POWER_ON;
				osMutexRelease(Fm25v02MutexHandle);

				osThreadResume(EventWriteTaskHandle);
			}
		}
		else // если на пине PFO микросхемы TPS3306-15 нет наличия единицы
		{
			if(status_registers.power_on_reg == 1)
			{
				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*POWER_ON_REG, 0x00);
				fm25v02_write(2*POWER_ON_REG+1, 0x00);
				status_registers.power_on_reg = 0;
				osMutexRelease(Fm25v02MutexHandle);

				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*GPRS_CALL_REG, 0x00);
				fm25v02_write(2*GPRS_CALL_REG+1, CALL_ON);
				osMutexRelease(Fm25v02MutexHandle);

				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*SYSTEM_STATUS_REG, 0x00);
				fm25v02_write(2*SYSTEM_STATUS_REG+1, POWER_OFF);
				status_registers.system_status_reg = POWER_OFF;
				osMutexRelease(Fm25v02MutexHandle);

				osThreadResume(EventWriteTaskHandle);
			}

			if(status_registers.power_on_lighting_reg == 1)
			{
				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*POWER_ON_LIGHTING_REG, 0x00);
				fm25v02_write(2*POWER_ON_LIGHTING_REG+1, 0x00);
				status_registers.power_on_lighting_reg = 0;
				osMutexRelease(Fm25v02MutexHandle);

				//osMutexWait(Fm25v02MutexHandle, osWaitForever);
				//fm25v02_write(2*GPRS_CALL_REG, 0x00);
				//fm25v02_write(2*GPRS_CALL_REG+1, CALL_ON);
				//osMutexRelease(Fm25v02MutexHandle);

				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*SYSTEM_STATUS_REG, 0x00);
				fm25v02_write(2*SYSTEM_STATUS_REG+1, POWER_OFF);
				status_registers.system_status_reg = POWER_OFF;
				osMutexRelease(Fm25v02MutexHandle);

				osThreadResume(EventWriteTaskHandle);
			}
		}
		/*
		switch(control_registers.security_control_reg) // проверяем значение переменной включения охранной сигнализации
		{

			case(DISABLE_FROM_SERVER):

				BUZ_OFF();

				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*SECURITY_CONTROL_REG, 0x00);
				fm25v02_write(2*SECURITY_CONTROL_REG+1, SECURITY_CONTROL_DEFAULT);
				control_registers.security_control_reg = SECURITY_CONTROL_DEFAULT; // обновляем переменную
				fm25v02_write(2*SECURITY_STATUS_REG, 0x00);
				fm25v02_write(2*SECURITY_STATUS_REG+1, DISABLED_BY_SERVER);
				osMutexRelease(Fm25v02MutexHandle);

				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*GPRS_CALL_REG, 0x00);
				fm25v02_write(2*GPRS_CALL_REG+1, CALL_ON);
				osMutexRelease(Fm25v02MutexHandle);

				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*SYSTEM_STATUS_REG, 0x00);
				fm25v02_write(2*SYSTEM_STATUS_REG+1, TURN_OFF_STATE_ALARM);
				status_registers.system_status_reg = TURN_OFF_STATE_ALARM;
				osMutexRelease(Fm25v02MutexHandle);

				osThreadResume(EventWriteTaskHandle);

			break;

			case(ENABLE_FROM_SERVER): // если в регистр поступила команда включить из сервера

				BUZ_OFF();

				security_control_temp = ENABLED_BY_SERVER;

				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*SECURITY_CONTROL_REG, 0x00);
				fm25v02_write(2*SECURITY_CONTROL_REG+1, SECURITY_CONTROL_DEFAULT);
				control_registers.security_control_reg = SECURITY_CONTROL_DEFAULT; // обновляем переменную
				fm25v02_write(2*SECURITY_STATUS_REG, 0x00);
				fm25v02_write(2*SECURITY_STATUS_REG+1, ARMING_PROCESS);
				osMutexRelease(Fm25v02MutexHandle);

			break;

			case(DISABLE_FROM_IBUTTON): // если поступила команда выключить из сервера или с таблетки

				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*SECURITY_CONTROL_REG, 0x00);
				fm25v02_write(2*SECURITY_CONTROL_REG+1, SECURITY_CONTROL_DEFAULT);
				control_registers.security_control_reg = SECURITY_CONTROL_DEFAULT; // обновляем переменную
				fm25v02_write(2*SECURITY_STATUS_REG, 0x00);
				fm25v02_write(2*SECURITY_STATUS_REG+1, DISABLED_BY_IBUTTON);
				osMutexRelease(Fm25v02MutexHandle);

				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*GPRS_CALL_REG, 0x00);
				fm25v02_write(2*GPRS_CALL_REG+1, CALL_ON);
				osMutexRelease(Fm25v02MutexHandle);

				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*SYSTEM_STATUS_REG, 0x00);
				fm25v02_write(2*SYSTEM_STATUS_REG+1, TURN_OFF_STATE_ALARM);
				status_registers.system_status_reg = TURN_OFF_STATE_ALARM;
				osMutexRelease(Fm25v02MutexHandle);

				osThreadResume(EventWriteTaskHandle);

			break;

			case(ENABLE_FROM_IBUTTON): // если поступила команда включить с таблетки

				security_control_temp = ENABLED_BY_IBUTTON;

				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*SECURITY_CONTROL_REG, 0x00);
				fm25v02_write(2*SECURITY_CONTROL_REG+1, SECURITY_CONTROL_DEFAULT);
				control_registers.security_control_reg = SECURITY_CONTROL_DEFAULT; // обновляем переменную
				fm25v02_write(2*SECURITY_STATUS_REG, 0x00);
				fm25v02_write(2*SECURITY_STATUS_REG+1, ARMING_PROCESS);
				osMutexRelease(Fm25v02MutexHandle);

			break;
		}
		*/

		switch(control_registers.time_update_reg) // проверяем значение переменной обновления времени
		{
			case(SET_TIME): // Если в регистр записана команда установить время

				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*TIME_UPDATE_REG, 0x00);
				fm25v02_write(2*TIME_UPDATE_REG+1, SET_TIME_DEFAULT);
				osMutexRelease(Fm25v02MutexHandle);

				set_time.Hours = control_registers.time_hour_reg; // записываем в переменные структуры времени значения регистров управления временем
				set_time.Minutes = control_registers.time_minute_reg;
				set_time.Seconds = control_registers.time_seconds_reg;
				set_date.Date = control_registers.time_day_reg;
				set_date.Month = control_registers.time_month_reg;
				set_date.Year = control_registers.time_year_reg;
				set_date.WeekDay = control_registers.time_weekday_reg;

				set_time.Hours = RTC_ByteToBcd2(set_time.Hours); // переводим обычный формат времени в BCD
				set_time.Minutes = RTC_ByteToBcd2(set_time.Minutes);
				set_time.Seconds = RTC_ByteToBcd2(set_time.Seconds);
				set_date.Date = RTC_ByteToBcd2(set_date.Date);
				set_date.Month = RTC_ByteToBcd2(set_date.Month);
				set_date.Year = RTC_ByteToBcd2(set_date.Year);
				set_date.WeekDay = RTC_ByteToBcd2(set_date.WeekDay);

				HAL_RTC_SetTime(&hrtc, &set_time, RTC_FORMAT_BCD); // устанавливаем дату и время
				HAL_RTC_SetDate(&hrtc, &set_date, RTC_FORMAT_BCD);


			break;

			case(SET_TIME_DEFAULT): // Если в регистр не поступала команда установки времени

				HAL_RTC_GetTime(&hrtc, &current_time , RTC_FORMAT_BCD); // получаем значение даты и времени
				HAL_RTC_GetDate(&hrtc, &current_date , RTC_FORMAT_BCD);

				current_time.Hours = RTC_Bcd2ToByte(current_time.Hours); // переводим из BCD формата в обычный
				current_time.Minutes = RTC_Bcd2ToByte(current_time.Minutes);
				current_time.Seconds = RTC_Bcd2ToByte(current_time.Seconds);
				current_date.Date = RTC_Bcd2ToByte(current_date.Date);
				current_date.Month = RTC_Bcd2ToByte(current_date.Month);
				current_date.Year = RTC_Bcd2ToByte(current_date.Year);
				current_date.WeekDay = RTC_Bcd2ToByte(current_date.WeekDay);

				osMutexWait(Fm25v02MutexHandle, osWaitForever);

				fm25v02_write(2*TIME_CURRENT_HOUR_REG, 0x00); // записываем в регистры значения даты и времени
				fm25v02_write(2*TIME_CURRENT_HOUR_REG+1, current_time.Hours);
				fm25v02_write(2*TIME_CURRENT_MINUTE_REG, 0x00);
				fm25v02_write(2*TIME_CURRENT_MINUTE_REG+1, current_time.Minutes);
				fm25v02_write(2*TIME_CURRENT_SECOND_REG, 0x00);
				fm25v02_write(2*TIME_CURRENT_SECOND_REG+1, current_time.Seconds);
				fm25v02_write(2*TIME_CURRENT_DAY_REG, 0x00);
				fm25v02_write(2*TIME_CURRENT_DAY_REG+1, current_date.Date);
				fm25v02_write(2*TIME_CURRENT_MONTH_REG, 0x00);
				fm25v02_write(2*TIME_CURRENT_MONTH_REG+1, current_date.Month);
				fm25v02_write(2*TIME_CURRENT_YEAR_REG, 0x00);
				fm25v02_write(2*TIME_CURRENT_YEAR_REG+1, current_date.Year);
				fm25v02_write(2*TIME_CURRENT_WEEKDAY_REG, 0x00);
				fm25v02_write(2*TIME_CURRENT_WEEKDAY_REG+1, current_date.WeekDay);

				osMutexRelease(Fm25v02MutexHandle);

				status_registers.time_current_hour_reg = current_time.Hours;
				status_registers.time_current_minute_reg = current_time.Minutes;
				status_registers.time_current_second_reg = current_time.Seconds;
				status_registers.time_current_day_reg = current_date.Date;
				status_registers.time_current_month_reg = current_date.Month;
				status_registers.time_current_year_reg = current_date.Year;
				status_registers.time_current_weekday_reg = current_date.WeekDay;

				if( ((control_registers.light_control_reg)&0x0020) == 0x0020 ) // если включено управление по расписанию
				{

					osMutexWait(Fm25v02MutexHandle, osWaitForever);

					fm25v02_read( 2*(0x28C0+(((current_date.Month)-1)*31*16)+(((current_date.Date)-1)*16)+0)+1, &temp_time_on_1_hour);
					fm25v02_read( 2*(0x28C0+(((current_date.Month)-1)*31*16)+(((current_date.Date)-1)*16)+1)+1, &temp_time_on_1_minute);
					fm25v02_read( 2*(0x28C0+(((current_date.Month)-1)*31*16)+(((current_date.Date)-1)*16)+2)+1, &temp_time_on_2_hour);
					fm25v02_read( 2*(0x28C0+(((current_date.Month)-1)*31*16)+(((current_date.Date)-1)*16)+3)+1, &temp_time_on_2_minute);
					fm25v02_read( 2*(0x28C0+(((current_date.Month)-1)*31*16)+(((current_date.Date)-1)*16)+4)+1, &temp_time_on_3_hour);
					fm25v02_read( 2*(0x28C0+(((current_date.Month)-1)*31*16)+(((current_date.Date)-1)*16)+5)+1, &temp_time_on_3_minute);
					fm25v02_read( 2*(0x28C0+(((current_date.Month)-1)*31*16)+(((current_date.Date)-1)*16)+6)+1, &temp_time_on_4_hour);
					fm25v02_read( 2*(0x28C0+(((current_date.Month)-1)*31*16)+(((current_date.Date)-1)*16)+7)+1, &temp_time_on_4_minute);
					fm25v02_read( 2*(0x28C0+(((current_date.Month)-1)*31*16)+(((current_date.Date)-1)*16)+8)+1, &temp_time_off_1_hour);
					fm25v02_read( 2*(0x28C0+(((current_date.Month)-1)*31*16)+(((current_date.Date)-1)*16)+9)+1, &temp_time_off_1_minute);
					fm25v02_read( 2*(0x28C0+(((current_date.Month)-1)*31*16)+(((current_date.Date)-1)*16)+10)+1, &temp_time_off_2_hour);
					fm25v02_read( 2*(0x28C0+(((current_date.Month)-1)*31*16)+(((current_date.Date)-1)*16)+11)+1, &temp_time_off_2_minute);
					fm25v02_read( 2*(0x28C0+(((current_date.Month)-1)*31*16)+(((current_date.Date)-1)*16)+12)+1, &temp_time_off_3_hour);
					fm25v02_read( 2*(0x28C0+(((current_date.Month)-1)*31*16)+(((current_date.Date)-1)*16)+13)+1, &temp_time_off_3_minute);
					fm25v02_read( 2*(0x28C0+(((current_date.Month)-1)*31*16)+(((current_date.Date)-1)*16)+14)+1, &temp_time_off_4_hour);
					fm25v02_read( 2*(0x28C0+(((current_date.Month)-1)*31*16)+(((current_date.Date)-1)*16)+15)+1, &temp_time_off_4_minute);

					osMutexRelease(Fm25v02MutexHandle);

					if( (((control_registers.light_control_reg)&0x0001)==0x0000) || (((control_registers.light_control_reg)&0x0002)==0x0000) || (((control_registers.light_control_reg)&0x0004)==0x0000) )
					{

						if( (temp_time_on_1_hour==current_time.Hours)&&(temp_time_on_1_minute==current_time.Minutes)&&((temp_time_on_1_hour!=temp_time_off_1_hour)||(temp_time_on_1_minute!=temp_time_off_1_minute)) )
						{
							osMutexWait(Fm25v02MutexHandle, osWaitForever);
							fm25v02_read(2*LIGHT_CONTROL_REG+1, &temp);
							temp = temp|0x07;
							fm25v02_write(2*LIGHT_CONTROL_REG+1, temp);
							control_registers.light_control_reg = control_registers.light_control_reg|0x0007;

							fm25v02_write(2*MONTH_LIGHTING_ON_REG, 0x00);
							temp = current_date.Month;
							fm25v02_write(2*MONTH_LIGHTING_ON_REG+1, temp);
							fm25v02_write(2*DAY_LIGHTING_ON_REG, 0x00);
							temp = current_date.Date;
							fm25v02_write(2*DAY_LIGHTING_ON_REG+1, temp);
							fm25v02_write(2*HOUR_LIGHTING_ON_REG+1, 0x00);
							temp = current_time.Hours;
							fm25v02_write(2*HOUR_LIGHTING_ON_REG+1, temp);
							fm25v02_write(2*MINUTE_LIGHTING_ON_REG, 0x00);
							temp = current_time.Minutes;
							fm25v02_write(2*MINUTE_LIGHTING_ON_REG+1, temp);

							osMutexRelease(Fm25v02MutexHandle);

							//LED8_TOGGLE();
						}

						if( (temp_time_on_2_hour==current_time.Hours)&&(temp_time_on_2_minute==current_time.Minutes)&&((temp_time_on_2_hour!=temp_time_off_2_hour)||(temp_time_on_2_minute!=temp_time_off_2_minute)) )
						{
							osMutexWait(Fm25v02MutexHandle, osWaitForever);
							fm25v02_read(2*LIGHT_CONTROL_REG+1, &temp);
							temp = temp|0x07;
							fm25v02_write(2*LIGHT_CONTROL_REG+1, temp);
							control_registers.light_control_reg = control_registers.light_control_reg|0x0007;

							fm25v02_write(2*MONTH_LIGHTING_ON_REG, 0x00);
							temp = current_date.Month;
							fm25v02_write(2*MONTH_LIGHTING_ON_REG+1, temp);
							fm25v02_write(2*DAY_LIGHTING_ON_REG, 0x00);
							temp = current_date.Date;
							fm25v02_write(2*DAY_LIGHTING_ON_REG+1, temp);
							fm25v02_write(2*HOUR_LIGHTING_ON_REG+1, 0x00);
							temp = current_time.Hours;
							fm25v02_write(2*HOUR_LIGHTING_ON_REG+1, temp);
							fm25v02_write(2*MINUTE_LIGHTING_ON_REG, 0x00);
							temp = current_time.Minutes;
							fm25v02_write(2*MINUTE_LIGHTING_ON_REG+1, temp);

							osMutexRelease(Fm25v02MutexHandle);

							//LED7_TOGGLE();
						}

						if( (temp_time_on_3_hour==current_time.Hours)&&(temp_time_on_3_minute==current_time.Minutes)&&((temp_time_on_3_hour!=temp_time_off_3_hour)||(temp_time_on_3_minute!=temp_time_off_3_minute)) )
						{
							osMutexWait(Fm25v02MutexHandle, osWaitForever);
							fm25v02_read(2*LIGHT_CONTROL_REG+1, &temp);
							temp = temp|0x07;
							fm25v02_write(2*LIGHT_CONTROL_REG+1, temp);
							control_registers.light_control_reg = control_registers.light_control_reg|0x0007;

							fm25v02_write(2*MONTH_LIGHTING_ON_REG, 0x00);
							temp = current_date.Month;
							fm25v02_write(2*MONTH_LIGHTING_ON_REG+1, temp);
							fm25v02_write(2*DAY_LIGHTING_ON_REG, 0x00);
							temp = current_date.Date;
							fm25v02_write(2*DAY_LIGHTING_ON_REG+1, temp);
							fm25v02_write(2*HOUR_LIGHTING_ON_REG+1, 0x00);
							temp = current_time.Hours;
							fm25v02_write(2*HOUR_LIGHTING_ON_REG+1, temp);
							fm25v02_write(2*MINUTE_LIGHTING_ON_REG, 0x00);
							temp = current_time.Minutes;
							fm25v02_write(2*MINUTE_LIGHTING_ON_REG+1, temp);

							osMutexRelease(Fm25v02MutexHandle);

							//LED6_TOGGLE();
						}

						if( (temp_time_on_4_hour==current_time.Hours)&&(temp_time_on_4_minute==current_time.Minutes)&&((temp_time_on_4_hour!=temp_time_off_4_hour)||(temp_time_on_4_minute!=temp_time_off_4_minute)) )
						{
							osMutexWait(Fm25v02MutexHandle, osWaitForever);
							fm25v02_read(2*LIGHT_CONTROL_REG+1, &temp);
							temp = temp|0x07;
							fm25v02_write(2*LIGHT_CONTROL_REG+1, temp);
							control_registers.light_control_reg = control_registers.light_control_reg|0x0007;

							fm25v02_write(2*MONTH_LIGHTING_ON_REG, 0x00);
							temp = current_date.Month;
							fm25v02_write(2*MONTH_LIGHTING_ON_REG+1, temp);
							fm25v02_write(2*DAY_LIGHTING_ON_REG, 0x00);
							temp = current_date.Date;
							fm25v02_write(2*DAY_LIGHTING_ON_REG+1, temp);
							fm25v02_write(2*HOUR_LIGHTING_ON_REG+1, 0x00);
							temp = current_time.Hours;
							fm25v02_write(2*HOUR_LIGHTING_ON_REG+1, temp);
							fm25v02_write(2*MINUTE_LIGHTING_ON_REG, 0x00);
							temp = current_time.Minutes;
							fm25v02_write(2*MINUTE_LIGHTING_ON_REG+1, temp);

							osMutexRelease(Fm25v02MutexHandle);

							//LED5_TOGGLE();
						}


					}

					else if( (((control_registers.light_control_reg)&0x0001)==0x0001) || (((control_registers.light_control_reg)&0x0002)==0x0002) || (((control_registers.light_control_reg)&0x0004)==0x0004) )
					{

						if( (temp_time_off_1_hour==current_time.Hours)&&(temp_time_off_1_minute==current_time.Minutes)&&((temp_time_on_1_hour!=temp_time_off_1_hour)||(temp_time_on_1_minute!=temp_time_off_1_minute)) )
						{
							osMutexWait(Fm25v02MutexHandle, osWaitForever);
							fm25v02_read(2*LIGHT_CONTROL_REG+1, &temp);
							temp = temp&0xF8;
							fm25v02_write(2*LIGHT_CONTROL_REG+1, temp);
							control_registers.light_control_reg = control_registers.light_control_reg&0xFFF8;

							fm25v02_write(2*MONTH_LIGHTING_OFF_REG, 0x00);
							temp = current_date.Month;
							fm25v02_write(2*MONTH_LIGHTING_OFF_REG+1, temp);
							fm25v02_write(2*DAY_LIGHTING_OFF_REG+1, 0x00);
							temp = current_date.Date;
							fm25v02_write(2*DAY_LIGHTING_OFF_REG+1, temp);
							fm25v02_write(2*HOUR_LIGHTING_OFF_REG+1, 0x00);
							temp = current_time.Hours;
							fm25v02_write(2*HOUR_LIGHTING_OFF_REG+1, temp);
							fm25v02_write(2*MINUTE_LIGHTING_OFF_REG+1, 0x00);
							temp = current_time.Minutes;
							fm25v02_write(2*MINUTE_LIGHTING_OFF_REG+1, temp);

							osMutexRelease(Fm25v02MutexHandle);
						}

						if( (temp_time_off_2_hour==current_time.Hours)&&(temp_time_off_2_minute==current_time.Minutes)&&((temp_time_on_2_hour!=temp_time_off_2_hour)||(temp_time_on_2_minute!=temp_time_off_2_minute)) )
						{
							osMutexWait(Fm25v02MutexHandle, osWaitForever);
							fm25v02_read(2*LIGHT_CONTROL_REG+1, &temp);
							temp = temp&0xF8;
							fm25v02_write(2*LIGHT_CONTROL_REG+1, temp);
							control_registers.light_control_reg = control_registers.light_control_reg&0xFFF8;

							fm25v02_write(2*MONTH_LIGHTING_OFF_REG, 0x00);
							temp = current_date.Month;
							fm25v02_write(2*MONTH_LIGHTING_OFF_REG+1, temp);
							fm25v02_write(2*DAY_LIGHTING_OFF_REG+1, 0x00);
							temp = current_date.Date;
							fm25v02_write(2*DAY_LIGHTING_OFF_REG+1, temp);
							fm25v02_write(2*HOUR_LIGHTING_OFF_REG+1, 0x00);
							temp = current_time.Hours;
							fm25v02_write(2*HOUR_LIGHTING_OFF_REG+1, temp);
							fm25v02_write(2*MINUTE_LIGHTING_OFF_REG+1, 0x00);
							temp = current_time.Minutes;
							fm25v02_write(2*MINUTE_LIGHTING_OFF_REG+1, temp);

							osMutexRelease(Fm25v02MutexHandle);
						}

						if( (temp_time_off_3_hour==current_time.Hours)&&(temp_time_off_3_minute==current_time.Minutes)&&((temp_time_on_3_hour!=temp_time_off_3_hour)||(temp_time_on_3_minute!=temp_time_off_3_minute)) )
						{
							osMutexWait(Fm25v02MutexHandle, osWaitForever);
							fm25v02_read(2*LIGHT_CONTROL_REG+1, &temp);
							temp = temp&0xF8;
							fm25v02_write(2*LIGHT_CONTROL_REG+1, temp);
							control_registers.light_control_reg = control_registers.light_control_reg&0xFFF8;

							fm25v02_write(2*MONTH_LIGHTING_OFF_REG, 0x00);
							temp = current_date.Month;
							fm25v02_write(2*MONTH_LIGHTING_OFF_REG+1, temp);
							fm25v02_write(2*DAY_LIGHTING_OFF_REG+1, 0x00);
							temp = current_date.Date;
							fm25v02_write(2*DAY_LIGHTING_OFF_REG+1, temp);
							fm25v02_write(2*HOUR_LIGHTING_OFF_REG+1, 0x00);
							temp = current_time.Hours;
							fm25v02_write(2*HOUR_LIGHTING_OFF_REG+1, temp);
							fm25v02_write(2*MINUTE_LIGHTING_OFF_REG+1, 0x00);
							temp = current_time.Minutes;
							fm25v02_write(2*MINUTE_LIGHTING_OFF_REG+1, temp);

							osMutexRelease(Fm25v02MutexHandle);
						}

						if( (temp_time_off_4_hour==current_time.Hours)&&(temp_time_off_4_minute==current_time.Minutes)&&((temp_time_on_4_hour!=temp_time_off_4_hour)||(temp_time_on_4_minute!=temp_time_off_4_minute)) )
						{
							osMutexWait(Fm25v02MutexHandle, osWaitForever);
							fm25v02_read(2*LIGHT_CONTROL_REG+1, &temp);
							temp = temp&0xF8;
							fm25v02_write(2*LIGHT_CONTROL_REG+1, temp);
							control_registers.light_control_reg = control_registers.light_control_reg&0xFFF8;

							fm25v02_write(2*MONTH_LIGHTING_OFF_REG, 0x00);
							temp = current_date.Month;
							fm25v02_write(2*MONTH_LIGHTING_OFF_REG+1, temp);
							fm25v02_write(2*DAY_LIGHTING_OFF_REG+1, 0x00);
							temp = current_date.Date;
							fm25v02_write(2*DAY_LIGHTING_OFF_REG+1, temp);
							fm25v02_write(2*HOUR_LIGHTING_OFF_REG+1, 0x00);
							temp = current_time.Hours;
							fm25v02_write(2*HOUR_LIGHTING_OFF_REG+1, temp);
							fm25v02_write(2*MINUTE_LIGHTING_OFF_REG+1, 0x00);
							temp = current_time.Minutes;
							fm25v02_write(2*MINUTE_LIGHTING_OFF_REG+1, temp);

							osMutexRelease(Fm25v02MutexHandle);
						}


					}
				}

			break;

		}

		switch(control_registers.reset_control_reg) // удаленная перезагрузка контроллера
		{
			case(1):
				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*RESET_CONTROL_REG, 0);
				fm25v02_write(2*RESET_CONTROL_REG+1, 0);
				osMutexRelease(Fm25v02MutexHandle);
				NVIC_SystemReset();
			break;

		}

		switch(control_registers.alarm_loop_clear_reg) // сбросить сработавшие шлейфы
		{
			case(1):
				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*ALARM_LOOP_CLEAR_REG, 0);
				fm25v02_write(2*ALARM_LOOP_CLEAR_REG+1, 0);
				fm25v02_write(2*ALARM_LOOP_REG, 0);
				fm25v02_write(2*ALARM_LOOP_REG+1, 0);
				osMutexRelease(Fm25v02MutexHandle);

				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*GPRS_CALL_REG, 0x00);
				fm25v02_write(2*GPRS_CALL_REG+1, CALL_ON);
				osMutexRelease(Fm25v02MutexHandle);

				osThreadResume(EventWriteTaskHandle);

			break;

		}

		switch(control_registers.false_loop_clear_reg) // сбросить неисправные шлейфы
		{
			case(1):
				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*FALSE_LOOP_CLEAR_REG, 0);
				fm25v02_write(2*FALSE_LOOP_CLEAR_REG+1, 0);
				fm25v02_write(2*ERROR_LOOP_REG, 0);
				fm25v02_write(2*ERROR_LOOP_REG+1, 0);
				osMutexRelease(Fm25v02MutexHandle);

				//osMutexWait(Fm25v02MutexHandle, osWaitForever);
				//fm25v02_write(2*GPRS_CALL_REG, 0x00);
				//fm25v02_write(2*GPRS_CALL_REG+1, CALL_ON);
				//osMutexRelease(Fm25v02MutexHandle);

				//osThreadResume(EventWriteTaskHandle);
				//osTimerStart(Ring_Center_TimerHandle, 1);

				osMutexWait(Fm25v02MutexHandle, osWaitForever);

				fm25v02_write(2*CE_303_ERROR_REG, 0);
				fm25v02_write(2*CE_303_ERROR_REG+1, 0);
				fm25v02_write(2*CE_303_CURRENT_A_REG, 0);
				fm25v02_write(2*CE_303_CURRENT_A_REG+1, 0);
				fm25v02_write(2*CE_303_CURRENT_B_REG, 0);
				fm25v02_write(2*CE_303_CURRENT_B_REG+1, 0);
				fm25v02_write(2*CE_303_CURRENT_C_REG, 0);
				fm25v02_write(2*CE_303_CURRENT_C_REG+1, 0);
				fm25v02_write(2*CE_303_CURRENT_MIL_A_REG, 0);
				fm25v02_write(2*CE_303_CURRENT_MIL_A_REG+1, 0);
				fm25v02_write(2*CE_303_CURRENT_MIL_B_REG, 0);
				fm25v02_write(2*CE_303_CURRENT_MIL_B_REG+1, 0);
				fm25v02_write(2*CE_303_CURRENT_MIL_C_REG, 0);
				fm25v02_write(2*CE_303_CURRENT_MIL_C_REG+1, 0);
				fm25v02_write(2*CE_303_VOLT_A_REG, 0);
				fm25v02_write(2*CE_303_VOLT_A_REG+1, 0);
				fm25v02_write(2*CE_303_VOLT_B_REG, 0);
				fm25v02_write(2*CE_303_VOLT_B_REG+1, 0);
				fm25v02_write(2*CE_303_VOLT_C_REG, 0);
				fm25v02_write(2*CE_303_VOLT_C_REG+1, 0);
				fm25v02_write(2*CE_303_VOLT_MIL_A_REG, 0);
				fm25v02_write(2*CE_303_VOLT_MIL_A_REG+1, 0);
				fm25v02_write(2*CE_303_VOLT_MIL_B_REG, 0);
				fm25v02_write(2*CE_303_VOLT_MIL_B_REG+1, 0);
				fm25v02_write(2*CE_303_VOLT_MIL_C_REG, 0);
				fm25v02_write(2*CE_303_VOLT_MIL_C_REG+1, 0);
				fm25v02_write(2*CE_303_POWER_A_REG, 0);
				fm25v02_write(2*CE_303_POWER_A_REG+1, 0);
				fm25v02_write(2*CE_303_POWER_B_REG, 0);
				fm25v02_write(2*CE_303_POWER_B_REG+1, 0);
				fm25v02_write(2*CE_303_POWER_C_REG, 0);
				fm25v02_write(2*CE_303_POWER_C_REG+1, 0);
				fm25v02_write(2*CE_303_POWER_MIL_A_REG, 0);
				fm25v02_write(2*CE_303_POWER_MIL_A_REG+1, 0);
				fm25v02_write(2*CE_303_POWER_MIL_B_REG, 0);
				fm25v02_write(2*CE_303_POWER_MIL_B_REG+1, 0);
				fm25v02_write(2*CE_303_POWER_MIL_C_REG, 0);
				fm25v02_write(2*CE_303_POWER_MIL_C_REG+1, 0);

				fm25v02_write(2*LIGHTING_STATUS_REG, 0);
				fm25v02_write(2*LIGHTING_STATUS_REG+1, 0);
				fm25v02_write(2*LIGHTING_ALARM_REG, 0);
				fm25v02_write(2*LIGHTING_ALARM_REG+1, 0);

				fm25v02_write(2*MONTH_LIGHTING_OFF_REG, 0);
				fm25v02_write(2*MONTH_LIGHTING_OFF_REG+1, 0);
				fm25v02_write(2*DAY_LIGHTING_OFF_REG, 0);
				fm25v02_write(2*DAY_LIGHTING_OFF_REG+1, 0);
				fm25v02_write(2*HOUR_LIGHTING_OFF_REG, 0);
				fm25v02_write(2*HOUR_LIGHTING_OFF_REG+1, 0);
				fm25v02_write(2*MINUTE_LIGHTING_OFF_REG, 0);
				fm25v02_write(2*MINUTE_LIGHTING_OFF_REG+1, 0);
				fm25v02_write(2*MONTH_LIGHTING_ON_REG, 0);
				fm25v02_write(2*MONTH_LIGHTING_ON_REG+1, 0);
				fm25v02_write(2*DAY_LIGHTING_ON_REG, 0);
				fm25v02_write(2*DAY_LIGHTING_ON_REG+1, 0);
				fm25v02_write(2*HOUR_LIGHTING_ON_REG, 0);
				fm25v02_write(2*HOUR_LIGHTING_ON_REG+1, 0);
				fm25v02_write(2*MINUTE_LIGHTING_ON_REG, 0);
				fm25v02_write(2*MINUTE_LIGHTING_ON_REG+1, 0);
				fm25v02_write(2*CE_303_TOTAL_POWER_H_REG, 0);
				fm25v02_write(2*CE_303_TOTAL_POWER_H_REG+1, 0);
				fm25v02_write(2*CE_303_TOTAL_POWER_L_REG, 0);
				fm25v02_write(2*CE_303_TOTAL_POWER_L_REG+1, 0);
				fm25v02_write(2*CE_303_TOTAL_POWER_M_REG, 0);
				fm25v02_write(2*CE_303_TOTAL_POWER_M_REG+1, 0);
				fm25v02_write(2*CE_303_TARIF1_POWER_H_REG, 0);
				fm25v02_write(2*CE_303_TARIF1_POWER_H_REG+1, 0);
				fm25v02_write(2*CE_303_TARIF1_POWER_L_REG, 0);
				fm25v02_write(2*CE_303_TARIF1_POWER_L_REG+1, 0);
				fm25v02_write(2*CE_303_TARIF1_POWER_MIL_REG, 0);
				fm25v02_write(2*CE_303_TARIF1_POWER_MIL_REG+1, 0);
				fm25v02_write(2*CE_303_TARIF2_POWER_H_REG, 0);
				fm25v02_write(2*CE_303_TARIF2_POWER_H_REG+1, 0);
				fm25v02_write(2*CE_303_TARIF2_POWER_L_REG, 0);
				fm25v02_write(2*CE_303_TARIF2_POWER_L_REG+1, 0);
				fm25v02_write(2*CE_303_TARIF2_POWER_MIL_REG, 0);
				fm25v02_write(2*CE_303_TARIF2_POWER_MIL_REG+1, 0);
				fm25v02_write(2*CE_303_TARIF3_POWER_H_REG, 0);
				fm25v02_write(2*CE_303_TARIF3_POWER_H_REG+1, 0);
				fm25v02_write(2*CE_303_TARIF3_POWER_L_REG, 0);
				fm25v02_write(2*CE_303_TARIF3_POWER_L_REG+1, 0);
				fm25v02_write(2*CE_303_TARIF3_POWER_MIL_REG, 0);
				fm25v02_write(2*CE_303_TARIF3_POWER_MIL_REG+1, 0);
				fm25v02_write(2*CE_303_TARIF4_POWER_H_REG, 0);
				fm25v02_write(2*CE_303_TARIF4_POWER_H_REG+1, 0);
				fm25v02_write(2*CE_303_TARIF4_POWER_L_REG, 0);
				fm25v02_write(2*CE_303_TARIF4_POWER_L_REG+1, 0);
				fm25v02_write(2*CE_303_TARIF4_POWER_MIL_REG, 0);
				fm25v02_write(2*CE_303_TARIF4_POWER_MIL_REG+1, 0);
				fm25v02_write(2*CE_303_TARIF5_POWER_H_REG, 0);
				fm25v02_write(2*CE_303_TARIF5_POWER_H_REG+1, 0);
				fm25v02_write(2*CE_303_TARIF5_POWER_L_REG, 0);
				fm25v02_write(2*CE_303_TARIF5_POWER_L_REG+1, 0);
				fm25v02_write(2*CE_303_TARIF5_POWER_MIL_REG, 0);
				fm25v02_write(2*CE_303_TARIF5_POWER_MIL_REG+1, 0);

				fm25v02_write(2*LIGHT_CONTROL_REG, 0);
				fm25v02_write(2*LIGHT_CONTROL_REG+1, 0);

				osMutexRelease(Fm25v02MutexHandle);

				osThreadResume(EventWriteTaskHandle);

			break;

		}


		switch(control_registers.event_read_reg)
		{
			case(1):

				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*EVENT_READ_REG, 0);
				fm25v02_write(2*EVENT_READ_REG+1, 0);
				fm25v02_write(2*ADDRESS_LAST_EVENT_H_REG, 0x00);
				fm25v02_write(2*ADDRESS_LAST_EVENT_H_REG+1, 0x20);
				fm25v02_write(2*ADDRESS_LAST_EVENT_L_REG, 0x00);
				fm25v02_write(2*ADDRESS_LAST_EVENT_L_REG+1, 0x00);
				osMutexRelease(Fm25v02MutexHandle);

			break;
		}
/*
		if(control_registers.lighting_switching_reg == LIGHTING_ON) // если функция освещения включена
		{
			switch(control_registers.light_control_reg&0x01) // проверяем бит фазы А
			{
				case(PHASE_A_SWITCH_OFF): // если выставлен бит на выключение фазы А

					PHASE_A_OFF(); // выключаем фазу А

				break;
				case(PHASE_A_SWITCH_ON): // если выставлен бит на включение фазы А

					if( ((status_registers.lighting_status_reg)&0x0001) == 0x0000 ) // если на фазе А1 нет напряжения
					{
						PHASE_A_ON(); // включаем фазу А
					}

				break;
			}
			switch(control_registers.light_control_reg&0x02) // проверяем бит фазы В
			{

				case(PHASE_B_SWITCH_OFF): // если выставлен бит на выключение фазы В

					PHASE_B_OFF(); // выключаем фазу А

				break;

				case(PHASE_B_SWITCH_ON): //если выставлен бит на включение фазы В

					if( ((status_registers.lighting_status_reg)&0x0002) == 0x0000 ) // если на фазе В1 нет напряжения
					{
						PHASE_B_ON(); // включаем фазу А
					}

				break;

			}
			switch(control_registers.light_control_reg&0x04) // проверяяем бит фазы С
			{

				case(PHASE_C_SWITCH_OFF): // если выставлен бит на выключение фазы С

					PHASE_C_OFF(); // выключаем фазу А

				break;

				case(PHASE_C_SWITCH_ON): // если выставлен бит на включение фазы С

					if( ((status_registers.lighting_status_reg)&0x0004) == 0x0000 ) // если на фазе В1 нет напряжения
					{
						PHASE_C_ON(); // включаем фазу А
					}

				break;

			}
		}
		else if(control_registers.lighting_switching_reg == LIGHTING_OFF) // если функция освещения выключена
		{

			PHASE_A_OFF(); // отключаем фазу А

			PHASE_B_OFF(); // отключаем фазу В

			PHASE_C_OFF(); // отключаем фазу С

		}

*/

		if(control_registers.lighting_switching_reg == LIGHTING_ON) // если функция освещения включена
		{
			switch(control_registers.lighting_alarm_reset_reg)
			{
				case(1):
					osMutexWait(Fm25v02MutexHandle, osWaitForever);
					fm25v02_write(2*LIGHTING_ALARM_RESET_REG, 0x00);
					fm25v02_write(2*LIGHTING_ALARM_RESET_REG+1, 0x00);
					control_registers.lighting_alarm_reset_reg = 0x00;
					fm25v02_write(2*LIGHTING_ALARM_REG, 0x00);
					fm25v02_write(2*LIGHTING_ALARM_REG+1, 0x00);
					status_registers.lighting_alarm_reg = 0x00;
					osMutexRelease(Fm25v02MutexHandle);
				break;
			}
		}

		if(1)
		{
			osMutexWait(Fm25v02MutexHandle, osWaitForever);
			fm25v02_write(2*CURRENT_PHASE_A_REG, (uint8_t)((cur_a>>8)&0x00FF) );
			fm25v02_write(2*CURRENT_PHASE_A_REG+1, (uint8_t)(cur_a&0x00FF) );
			fm25v02_write(2*CURRENT_PHASE_B_REG, (uint8_t)((cur_b>>8)&0x00FF) );
			fm25v02_write(2*CURRENT_PHASE_B_REG+1, (uint8_t)(cur_b&0x00FF) );
			fm25v02_write(2*CURRENT_PHASE_C_REG, (uint8_t)((cur_c>>8)&0x00FF) );
			fm25v02_write(2*CURRENT_PHASE_C_REG+1, (uint8_t)(cur_c&0x00FF) );
			osMutexRelease(Fm25v02MutexHandle);

			status_registers.current_phase_a_reg = cur_a;
			status_registers.current_phase_b_reg = cur_b;
			status_registers.current_phase_c_reg = cur_c;
		}

		if(1)
		{
			if( TSL2561_getID(&sensor_id) == true)
			{
				//TSL2561_setPowerDown();
				//HAL_Delay(1000);
				//while( TSL2561_getID(&sensor_id) == true){}
				d_state2 = TSL2561_setTiming_ms(gain, time,&ms);
				TSL2561_setPowerUp();
				//HAL_Delay(1000);
				//while( TSL2561_getID(&sensor_id) == false){}
				d_state1 = TSL2561_getData(&data0, &data2);
				if( TSL2561_getLux(gain, ms, data0, data2, &lux) == 1)
				{
					data_lux = lux/1.8;
					//data_lux = 2000;
				}
				else
				{
					data_lux = 40000;
				}

			}
			else
			{
				HAL_I2C_DeInit(&hi2c1);
				HAL_I2C_Init(&hi2c1);
				//d_state4 = TSL2561_setPowerDown();
				//while( TSL2561_getID(&sensor_id) == true){}
				//d_state2 = TSL2561_setTiming_ms(gain, time,&ms);
				//d_state3 = TSL2561_setPowerUp();
				//while( TSL2561_getID(&sensor_id) == false){}
				//HAL_Delay(2000);
			}


		}

		if( ((control_registers.light_control_reg)&0x0040) == 0x0040 ) // если включено управление по датчику освещенности
		{
			if( data_lux > control_registers.lighting_threshold_off_reg )
			{
				//time_on_counter = 0;

				if(time_off_counter < control_registers.delay_off_sensor_reg)
				{
					time_off_counter++;
				}
				else if(time_off_counter == control_registers.delay_off_sensor_reg)
				{
					time_off_counter++;
					//time_on_counter = 0;

					if(((control_registers.light_control_reg)&0x0001)==0x0001) // если включена фаза А, выключаем фазу А
					{
						osMutexWait(Fm25v02MutexHandle, osWaitForever);
						fm25v02_read(2*LIGHT_CONTROL_REG+1, &temp);
						temp = temp&0xFE;
						fm25v02_write(2*LIGHT_CONTROL_REG+1, temp);
						control_registers.light_control_reg = control_registers.light_control_reg&0xFFFE;
						osMutexRelease(Fm25v02MutexHandle);
					}
					else
					{
						time_on_counter = (control_registers.delay_on_sensor_reg + 1);
					}
					if(((control_registers.light_control_reg)&0x0002)==0x0002) // если включена фаза В, выключаем фазу В
					{
						osMutexWait(Fm25v02MutexHandle, osWaitForever);
						fm25v02_read(2*LIGHT_CONTROL_REG+1, &temp);
						temp = temp&0xFD;
						fm25v02_write(2*LIGHT_CONTROL_REG+1, temp);
						control_registers.light_control_reg = control_registers.light_control_reg&0xFFFD;
						osMutexRelease(Fm25v02MutexHandle);
					}
					else
					{
						time_on_counter = (control_registers.delay_on_sensor_reg + 1);
					}
					if(((control_registers.light_control_reg)&0x0004)==0x0004) // если включена фаза С, выключаем фазу С
					{
						osMutexWait(Fm25v02MutexHandle, osWaitForever);
						fm25v02_read(2*LIGHT_CONTROL_REG+1, &temp);
						temp = temp&0xFB;
						fm25v02_write(2*LIGHT_CONTROL_REG+1, temp);
						control_registers.light_control_reg = control_registers.light_control_reg|0xFFFB;
						osMutexRelease(Fm25v02MutexHandle);
					}
					else
					{
						time_on_counter = (control_registers.delay_on_sensor_reg + 1);
					}
				}

			}

			else if( data_lux <= control_registers.lighting_threshold_off_reg )
			{
				time_off_counter = 0;
				//time_on_counter = (control_registers.delay_on_sensor_reg + 1);
				//if(time_on_counter == (control_registers.delay_on_sensor_reg + 1))
				//{
					//time_on_counter = 0;
				//}
			}

			if( data_lux < control_registers.lighting_threshold_on_reg )
			{
				//time_off_counter = 0;

				if(time_on_counter < control_registers.delay_on_sensor_reg)
				{
					time_on_counter++;
				}
				else if( time_on_counter == control_registers.delay_on_sensor_reg )
				{
					time_on_counter++;
					//time_off_counter = 0;

					if(((control_registers.light_control_reg)&0x0001)==0x0000) // если выключена фаза А, включаем фазу А
					{
						osMutexWait(Fm25v02MutexHandle, osWaitForever);
						fm25v02_read(2*LIGHT_CONTROL_REG+1, &temp);
						temp = temp|0x01;
						fm25v02_write(2*LIGHT_CONTROL_REG+1, temp);
						control_registers.light_control_reg = control_registers.light_control_reg|0x0001;
						osMutexRelease(Fm25v02MutexHandle);
					}
					else
					{
						time_off_counter = (control_registers.delay_off_sensor_reg + 1);
					}
					if(((control_registers.light_control_reg)&0x0002)==0x0000) // если выключена фаза В, включаем фазу В
					{
						osMutexWait(Fm25v02MutexHandle, osWaitForever);
						fm25v02_read(2*LIGHT_CONTROL_REG+1, &temp);
						temp = temp|0x02;
						fm25v02_write(2*LIGHT_CONTROL_REG+1, temp);
						control_registers.light_control_reg = control_registers.light_control_reg|0x0002;
						osMutexRelease(Fm25v02MutexHandle);
					}
					else
					{
						time_off_counter = (control_registers.delay_off_sensor_reg + 1);
					}
					if(((control_registers.light_control_reg)&0x0004)==0x0000) // если выключена фаза С, включаем фазу С
					{
						osMutexWait(Fm25v02MutexHandle, osWaitForever);
						fm25v02_read(2*LIGHT_CONTROL_REG+1, &temp);
						temp = temp|0x04;
						fm25v02_write(2*LIGHT_CONTROL_REG+1, temp);
						control_registers.light_control_reg = control_registers.light_control_reg|0x0004;
						osMutexRelease(Fm25v02MutexHandle);
					}
					else
					{
						time_off_counter = (control_registers.delay_off_sensor_reg + 1);
					}
				}

			}
			else if( data_lux >= control_registers.lighting_threshold_on_reg )
			{
				time_on_counter = 0;
				//time_off_counter = (control_registers.delay_off_sensor_reg + 1);
				//if(time_off_counter == (control_registers.delay_off_sensor_reg + 1) )
				//{
					//time_off_counter = 0;
				//}
			}


		}


		osDelay(500);
	}
}
