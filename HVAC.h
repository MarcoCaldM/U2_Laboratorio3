 // FileName:        HVAC.h
 // Dependencies:    None.
 // Processor:       MSP432
 // Board:           MSP432P401R
 // Program version: CCS V8.3 TI
 // Company:         Texas Instruments
 // Description:     Incluye librer�as, define ciertas macros y significados as� como llevar un control de versiones.
 // Authors:         Jos� Luis Chac�n M. y Jes�s Alejandro Navarro Acosta.
 // Updated:         11/2018

#ifndef _hvac_h_
#define _hvac_h_

#pragma once

#define __MSP432P401R__
#define  __SYSTEM_CLOCK    48000000 // Frecuencias funcionales recomendadas: 12, 24 y 48 Mhz.

/* Archivos de cabecera importantes. */
#include <unistd.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Archivos de cabecera POSIX. */
#include <pthread.h>
#include <semaphore.h>
#include <ti/posix/tirtos/_pthread.h>
#include <ti/sysbios/knl/Task.h>

/* Archivos de cabecera para RTOS. */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Event.h>

/* Board Support Package. */
#include "Drivers/BSP.h"

/* Enumeradores y variable para la seleccion del menu. */

enum MENU{      // Para las funciones del menu
    DEFAULT,        // 0
    P1_SELECTED,    // 1
    P2_SELECTED,    // 2
    SL_SELECTED,    // 3
};

enum UP_DOWN{   // Para el estado de las persianas y la SL
    Down,   //0
    Up,     //1
};

struct Estado_PSL{
    uint8_t Estado;
}   Persiana1,Persiana2,SecuenciaLED;

uint8_t  Select_Menu;   //Estado del boton menu

/* Enumeradores para la descripcion del sistema. */
enum FAN        // Para el fan (abanico).
{
    On,
    Auto,
};

enum SYSTEM     // Para el sistema cuando FAN est� en auto (cool, off y heat, o no considerar ninguno y usar fan only).
{
    Cool,
    Off,
    Heat,
    FanOnly,
};

struct EstadoEntradas
{
    uint8_t  SystemState;
    uint8_t     FanState;
}   EstadoEntradas;

// Definiciones B�sicas.
#define ENTRADA 1
#define SALIDA 0

// Re-definici�n de los bits y puertos de entrada a utilizar.
#define ON_OFF      B1
#define MENU_BTN    B4
#define UP_BTN      B4
#define DOWN_BTN    B5

#define FAN_ON      B3
#define FAN_AUTO    B4
#define SYSTEM_COOL B5
#define SYSTEM_OFF  B6
#define SYSTEM_HEAT B7

#define ON_OFF_PORT     1
#define MENU_PORT       1
#define UP_DOWN_PORT    2

#define FAN_PORT        2
#define SYSTEM_PORT     2
#define FAN_PORTT        P2
#define SYSTEM_PORTT     P2
#define SETPOINT_PORTT   P1

#define TEMP_CH         CH0
#define HEARTBEAT_CH    CH1
#define POT_PIN         AN1

//Canales para los potenciometros
#define LUM1    CH8
#define LUM2    CH9
#define LUM3    CH10

// Re-definici�n de los bits y puertos de salida a utilizar.
#define FAN_LED     BSP_LED1
#define HEAT_LED    BSP_LED2
#define HBeatLED    BSP_LED3
#define COOL_LED    BSP_LED4

#define FAN_LED_PORT    BSP_LED1_PORT
#define HEAT_LED_PORT   BSP_LED2_PORT
#define HB_LED_PORT     BSP_LED3_PORT
#define COOL_LED_PORT   BSP_LED4_PORT

// Definiciones del estado 'normal' de los botones externos a la tarjeta (solo hay dos botones).
#define GND 0
#define VCC 1
#define NORMAL_STATE_EXTRA_BUTTONS GND  // Aqui se coloca GND o VCC.

// Definiciones del sistema.
#define MAX_MSG_SIZE 64
#define MAX_ADC_VALUE 16383             // (2 ^14 bits) es la resoluci�n default.
#define MAIN_UART (uint32_t)(EUSCI_A0)
#define DELAY 20000
#define ITERATIONS_TO_PRINT 49

// Definiciones para el encendido y apagado del sistema
uint8_t Enc_Apg;
uint32_t contadorApg;
#define ENCENDIDO 1
#define APAGADO 0

// Definici�n para el RTOS.
#define THREADSTACKSIZE1 1500

/* Funciones. */

/* Funci�n de interrupci�n para botones de ON/OFF, menu y UP/DOWN. */
extern void INT_SWI(void);
extern void INT_UP_DOWN(void);

/* Funciones de inicializaci�n. */
extern void HVAC_InicialiceIO   (void);
extern void HVAC_InicialiceADC  (void);
extern void HVAC_InicialiceUART (void);
extern void System_InicialiceTIMER (void); // ESTO LO A�ADI PARA UTILIZAR EL "TIMER32"

/* Funciones principales. */
extern void HVAC_ActualizarEntradas(void);
extern void HVAC_ActualizarSalidas(void);
extern void HVAC_Heartbeat(void);
extern void HVAC_PrintState(void);

extern void HVAC_Enc_Apg_Check(void);

// Funciones para los estados Heat y Cool.
extern void HVAC_Heat(void);
extern void HVAC_Cool(void);

// Funciones para controlar el sistema.
extern void HVAC_Enc_Apg_Ctrl(void);
extern void HVAC_Menu(void);

#endif
