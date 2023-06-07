#include "ReadRegistersTask.h"
#include "cmsis_os.h"
#include "modbus.h"
#include "fm25v02.h"

extern osThreadId M95TaskHandle;
extern osThreadId GetCurrentTaskHandle;
extern osThreadId MainTaskHandle;
extern ADC_HandleTypeDef hadc1;
//extern osThreadId LedTaskHandle;
extern osMutexId Fm25v02MutexHandle;
extern status_register_struct status_registers;
extern control_register_struct control_registers;


void ThreadReadRegistersTask(void const * argument)
{
	read_status_registers(); // вычитываем регистры
	read_control_registers();
	osDelay(2000); // ждем пока будет получен статус фаз А1,А2,В1,В2,С1,С2
	osThreadResume(MainTaskHandle); // запускаем основной процесс
	osThreadResume(M95TaskHandle);  // запускаем процесс модема
	osThreadResume(GetCurrentTaskHandle);  // запускаем процесс модема
	osDelay(1000); //ждем 1 секунду


	for(;;)
	{

		read_status_registers(); // вычитываем регистры
		read_control_registers();

		osDelay(1000); // ждем 1 секунду
	}
}
