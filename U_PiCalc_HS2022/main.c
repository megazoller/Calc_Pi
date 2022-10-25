/*
 * U_PiCalc_HS2022.c
 *
 * Created: 20.03.2018 18:32:07
 * Author : -
 */ 

#include <math.h>
#include <stdio.h>
#include <string.h>
#include "avr_compiler.h"
#include "pmic_driver.h"
#include "TC_driver.h"
#include "clksys_driver.h"
#include "sleepConfig.h"
#include "port_driver.h"
#include "math.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "event_groups.h"
#include "stack_macros.h"

#include "mem_check.h"

#include "init.h"
#include "utils.h"
#include "errorHandler.h"
#include "NHD0420Driver.h"

#include "ButtonHandler.h"


void controllerTask(void* pvParameters);
void leibnizTask(void* pvParameters);
void asinTask(void *pvParameters);
void breakTask(void* pvParameters);
void UITask(void* pvParameters);

//EventgroupButtons
EventGroupHandle_t egButtonEvents = NULL;
#define BUTTON1_SHORT	0x01 // Start Calculate
#define BUTTON2_SHORT	0x02 // Stop Calculate
#define BUTTON3_SHORT	0x04 // Reset 
#define BUTTON4_SHORT	0x08 // Switch Algorithm

//Modes for Finite State Machine
#define MODE_IDLE 0
#define MODE_Leibniz 1
#define MODE_asin 2
//#define MODE_ALARMALARM 3

uint8_t mode = 0;
float pi_calc = 0;

//EventgroupLeibniz
EventGroupHandle_t egLeibnizState = NULL;
 

//EventgroupOTHER
EventGroupHandle_t egarctanState = NULL;




int main(void)
{
	vInitClock();
	
	xTaskCreate( controllerTask, (const char *) "control_tsk", configMINIMAL_STACK_SIZE+150, NULL, 3, NULL);
	xTaskCreate( leibnizTask, (const char *) "leibniz_tsk", configMINIMAL_STACK_SIZE+150, NULL, 1, NULL);
	xTaskCreate( asinTask, (const char*) "asin_tsk", configMINIMAL_STACK_SIZE, NULL, 1, NULL); //Init ButtonTask. Medium Priority. Somehow important to time Button debouncing and timing.
	xTaskCreate( UITask, (const char *) "ui_task", configMINIMAL_STACK_SIZE, NULL, 2, NULL); //Init UITask. Lowest Priority. Least time critical.
	
	vInitDisplay();
	vDisplayClear();
	vDisplayWriteStringAtPos(0,0,"PI-Calc HS2022");
	
	vTaskStartScheduler();
	vTaskSuspend(leibnizTask);
	vTaskSuspend(asinTask);
	return 0;
}

void controllerTask(void* pvParameters)
{
	egButtonEvents = xEventGroupCreate();
	initButtons();
	while(1)
	{
		updateButtons();
		if(getButtonPress(BUTTON1) == SHORT_PRESSED)
		{
			xEventGroupSetBits(egButtonEvents, BUTTON1_SHORT);
		}
		if(getButtonPress(BUTTON2) == SHORT_PRESSED)
		{
			xEventGroupSetBits(egButtonEvents, BUTTON2_SHORT);
		}
		if(getButtonPress(BUTTON3) == SHORT_PRESSED)
		{
			xEventGroupSetBits(egButtonEvents, BUTTON3_SHORT);
		}
		if(getButtonPress(BUTTON4) == SHORT_PRESSED)
		{
			xEventGroupSetBits(egButtonEvents, BUTTON4_SHORT);
		}
		vTaskDelay(10/portTICK_RATE_MS);
	}
}
		
void UITask(void* pvParameters)
{
	while(1)
	{
		switch(mode)
		{
			case MODE_IDLE:
				if(xEventGroupGetBits(egButtonEvents) & BUTTON4_SHORT)
				{
					xEventGroupClearBits(egButtonEvents, BUTTON4_SHORT);
					vDisplayClear();
					vDisplayWriteStringAtPos(0,0, "Leibniz");
					char pistring[12];
					sprintf(&pistring[0], "PI: %.8f", pi_calc);
					vDisplayWriteStringAtPos(1,0, "%s", pistring);
					mode = MODE_Leibniz;
				}
			break;
			case MODE_Leibniz:
				if(xEventGroupGetBits(egButtonEvents) & BUTTON4_SHORT)
				{
					xEventGroupClearBits(egButtonEvents, BUTTON4_SHORT);
					vDisplayClear();
					pi_calc = 0;
					mode = MODE_asin;
				}
				if(xEventGroupGetBits(egButtonEvents) & BUTTON1_SHORT)
				{
					xEventGroupClearBits(egButtonEvents, BUTTON1_SHORT);
					char pistring[12];
					sprintf(&pistring[0], "PI: %.8f", pi_calc);
					vDisplayClear();
					vDisplayWriteStringAtPos(0,0, "Leibniz");
					vDisplayWriteStringAtPos(1,0, "%s", pistring);
					mode = MODE_Leibniz;
				}
			break;
			case MODE_asin:
				if(xEventGroupGetBits(egButtonEvents) & BUTTON1_SHORT)
				{
					xEventGroupClearBits(egButtonEvents, BUTTON1_SHORT);
					char pistring[12];
					sprintf(&pistring[0], "PI: %.8f", pi_calc);
					vDisplayClear();
					vDisplayWriteStringAtPos(0,0, "Leibniz");
					vDisplayWriteStringAtPos(1,0, "%s", pistring);
					mode = MODE_Leibniz;
				}
			break;
			default:
			break;
		}
	}
}

void leibnizTask(void* pvParameters)
{
	float piviertel = 1.0;
	uint32_t n = 3;
	double pi_calc = 0;
	while (1)
	{
		piviertel = piviertel -(1.0/n) + (1.0/(n+2));
		n = n+4;
		pi_calc = piviertel * 4;
		vTaskDelay(500);
	}
}

void asinTask(void* pvParameters)
{
	double pi_calc = 0;
	while (1)
	{
		double pi_calc = 2 * asin(1.0);
	}
}