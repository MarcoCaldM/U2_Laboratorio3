 // FileName:        HVAC_Thread.c
 // Dependencies:    HVAC.h
 // Processor:       MSP432
 // Board:           MSP432P401R
 // Program version: CCS V8.3 TI
 // Company:         Texas Instruments
 // Description:     Definici�n de las funciones del thread de HVAC_Thread().
 // Authors:         Jos� Luis Chac�n M. y Jes�s Alejandro Navarro Acosta.
 // Updated:         11/2018

#include "HVAC.h"                           // Incluye definici�n del sistema.

/*****  SE DECLARARON LAS VARIABLES Y FUNCIONES PARA REALIZAR EL DALAY CON EL TIMER ******** */
bool retraso = false;
void Timer32_INT1 (void); // Funci�n de interrupci�n.
void Delay_ms (uint32_t time); // Funci�n de delay.
/***********************************************************************************************/

/*********************************THREAD******************************************
 * Function: HVAC_Thread
 * Preconditions: None.
 * Overview: Realiza la lectura de la temperatura y controla salidas actualizando
 *           a su vez entradas. Imprime estados.
 * Input:  Apuntador vac�o que puede apuntar cualquier tipo de dato.
 * Output: None.
 *
 ********************************************************************************/

void *HVAC_Thread(void *arg0)
{
    SystemInit();
    System_InicialiceTIMER();           // INICIALIZA EL TIMER
    HVAC_InicialiceIO();                // INICIALIZA ENTRADAS Y SALIDAS
    HVAC_InicialiceADC();               // INICIALIZA MODULO ADC
    HVAC_InicialiceUART();              // INICIALIZA MODULO UART

    GPIO_setOutput(BSP_LED1_PORT,  BSP_LED1,  0); //Apaga el LED rojo
    Enc_Apg = APAGADO;

    while(1)
    {
        contadorApg = 0;        //Reinicia el contador del apagado
        Select_Menu = 0;        //Reinicia la seleccion del menu
        while(Enc_Apg == ENCENDIDO){
            HVAC_ActualizarEntradas();
            HVAC_ActualizarSalidas();
            HVAC_Enc_Apg_Check();
            HVAC_PrintState();
            HVAC_Heartbeat();
        }
    }
}



/* *********  FUNCIONES PARA REALIZAR EL DALAY CON EL TIMER ********* */
void Delay_ms(uint32_t time)
{
    T32_EnableTimer1(); // Habilita timer.
    T32_EnableInterrupt1(); // Habilita interrupci�n.
    // Carga de valor en milisegundos.
    T32_SetLoadValue1(time*(__SYSTEM_CLOCK/1000));
    retraso = true;
    while(retraso); // While enclavado.
}
void Timer32_INT1(void)
{
    T32_ClearInterruptFlag1(); // Al llegar a la interrupci�n
    retraso = false; // desenclava el while.
    contadorApg = 0; // Si no se pulsa dos veces el boton ON/OFF reinicia el contador de apagado
}

