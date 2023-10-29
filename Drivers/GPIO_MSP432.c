 // FileName:        UART_MSP432.c
 // Dependencies:    system.h
 // Processor:       MSP432
 // Board:			 MSP432P401R
 // Program version: CCS V8.3 TI
 // Company:         Texas Instruments
 // Description:     Definici�n de funciones del m�dulo GPIO para entradas y salidas.
 // Authors:         Jos� Luis Chac�n M. y Jes�s Alejandro Navarro Acosta.
 // Updated:         11/2018

#include "HVAC.h"     // Configuraci�n del sistema.


/* Declaraci�n del arreglo GPIO_PORT_TO_BASE
 * En el cual se encuentran las direcciones en donde comienzan
 * los registros de control para cada puerto.                   */
static const uint32_t GPIO_PORT_TO_BASE[] =
{   0x00,
    0x40004C00,
    0x40004C01,
    0x40004C20,
    0x40004C21,
    0x40004C40,
    0x40004C41,
    0x40004C60,
    0x40004C61,
    0x40004C80,
    0x40004C81,
    0x40004D20
};

/*****************************************************************************
 * Function: GPIO_init
 * Preconditions: None.
 * Overview: Inicializaci�n del m�dulo GPIO. Se configuran los pines de
 *          entrada y salida que corresponden a los LEDs y los switches
 *          en la tarjeta.
 * Input: None.
 * Output: None.
 *
 *****************************************************************************/
void GPIO_init_board(void)
{
    // Para los led's de la tarjeta (4).
    GPIO_set_output_bit(P1,B0); // LED1 (rojo)
    GPIO_set_output_bit(P2,B0); // LED_RGB (rojo)
    GPIO_set_output_bit(P2,B1); // LED_RGB (verde)
    GPIO_set_output_bit(P2,B2); // LED_RGB (azul)

    // Para los switches de la tarjeta (2).
    GPIO_set_input_bit(P1,B1); // BTN ON/OFF
    GPIO_set_input_bit(P1,B4); // BTN Menu
    GPIO_set_input_bit(P2,B4); // BTN UP
    GPIO_set_input_bit(P2,B5); // BTN DOWN

    GPIO_enable_bit_pullup(P1,B1); // SW1
    GPIO_enable_bit_pullup(P1,B4); // BTN ON/OFF
    GPIO_enable_bit_pullup(P2,B4); // BTN UP
    GPIO_enable_bit_pullup(P2,B5); // BTN DOWN

    GPIO_write_bit_high(P1,B1); // Force pull-up.
    GPIO_write_bit_high(P1,B4); // Force pull-up.
    GPIO_write_bit_low(P2,B4); // Force pull-up.
    GPIO_write_bit_low(P2,B5); // Force pull-up.
}

/**********************************NEW*****************************************
 * Function: GPIO_setBitIO
 * Preconditions: None.
 * Overview: Marca un pin de un puerto como entrada o como salida.
 * Input: Puerto (i.e 2), Pin (i.e. BIT0), salida (0) o entrada (1).
 * Output: None.
 *
 *****************************************************************************/

void GPIO_setBitIO(uint_fast8_t selectedPort, uint_fast16_t selectedPins, bool I_O)  // 0: Salida, // 1: Entrada.
{
    if(selectedPort%2 == 0)  // Si el puerto es par.
    {
        if(!I_O)
            GPIO_set_output_bit(((DIO_PORT_Even_Interruptable_Type*) GPIO_PORT_TO_BASE[selectedPort-1]), selectedPins);
        else
        {
            GPIO_set_input_bit(((DIO_PORT_Even_Interruptable_Type*) GPIO_PORT_TO_BASE[selectedPort-1]), selectedPins);
            GPIO_enable_bit_pullup(((DIO_PORT_Even_Interruptable_Type*) GPIO_PORT_TO_BASE[selectedPort-1]), selectedPins);
            GPIO_write_bit_high(((DIO_PORT_Even_Interruptable_Type*) GPIO_PORT_TO_BASE[selectedPort-1]), selectedPins);
        }
    }
    else                    // Si el puerto es impar.
    {
        if(!I_O)
            GPIO_set_output_bit(((DIO_PORT_Odd_Interruptable_Type*) GPIO_PORT_TO_BASE[selectedPort]), selectedPins);
        else
        {
            GPIO_set_input_bit(((DIO_PORT_Odd_Interruptable_Type*) GPIO_PORT_TO_BASE[selectedPort]), selectedPins);
            GPIO_enable_bit_pullup(((DIO_PORT_Odd_Interruptable_Type*) GPIO_PORT_TO_BASE[selectedPort]), selectedPins);
            GPIO_write_bit_high(((DIO_PORT_Odd_Interruptable_Type*) GPIO_PORT_TO_BASE[selectedPort]), selectedPins);
        }
    }
}

/******************************************************************************
 * Function: GPIO_setOutput
 * Preconditions: None.
 * Overview: Establece el valor de una salida.
 * Input: Puerto (i.e 2), Pin (i.e BIT0), estado.
 * Output: None.
 *
 *****************************************************************************/

void GPIO_setOutput(uint_fast8_t selectedPort, uint_fast16_t selectedPins, bool state)
{
    if(selectedPort%2 == 0)  // Si el puerto es par.
    {
        if(!state)
            GPIO_write_bit_low(((DIO_PORT_Even_Interruptable_Type*) GPIO_PORT_TO_BASE[selectedPort-1]), selectedPins);
        else
            GPIO_write_bit_high(((DIO_PORT_Even_Interruptable_Type*) GPIO_PORT_TO_BASE[selectedPort-1]), selectedPins);
    }
    else                    // Si el puerto es impar.
    {
        if(!state)
            GPIO_write_bit_low(((DIO_PORT_Odd_Interruptable_Type*) GPIO_PORT_TO_BASE[selectedPort]), selectedPins);
        else
            GPIO_write_bit_high(((DIO_PORT_Odd_Interruptable_Type*) GPIO_PORT_TO_BASE[selectedPort]), selectedPins);
    }
}

/*****************************************************************************
 * Function: GPIO_enableInterrupt
 * Preconditions: None.
 * Overview: Habilita la interrupci�n para un pin de un puerto en espec�fico.
 * Input: uint_fast8_t selectedPort, uint_fast16_t selectedPins.
 * Output: None.
 *
 *****************************************************************************/
void GPIO_enableInterrupt(uint_fast8_t selectedPort, uint_fast16_t selectedPins)
{
    // Identifica el puerto seleccionado.
    uint32_t baseAddress = GPIO_PORT_TO_BASE[selectedPort];
    // Modifica el bit del registro correspondiente para habilitar la interrupci�n.
    HWREG16(baseAddress + OFS_PAIE) |= selectedPins;
}

/*****************************************************************************
 * Function: GPIO_disableInterrupt
 * Preconditions: None.
 * Overview: Deshabilita la interrupci�n para un pin de un puerto en espec�fico.
 * Input: uint_fast8_t selectedPort, uint_fast16_t selectedPins.
 * Output: None.
 *
 *****************************************************************************/
void GPIO_disableInterrupt(uint_fast8_t selectedPort, uint_fast16_t selectedPins)
{
    //Identifica el puerto seleccionado.
    uint32_t baseAddress = GPIO_PORT_TO_BASE[selectedPort];
    //Modifica el bit del registro correspondiente para deshabilitar la interrupci�n.
    HWREG16(baseAddress + OFS_PAIE) &= ~selectedPins;
}

/*****************************************************************************
 * Function: GPIO_clearInterruptFlag
 * Preconditions: None..
 * Overview: Limpia el bit que indica que hubo una interrupci�n en el bit del puerto espec�ficado.
 * Input:  uint_fast8_t selectedPort, uint_fast16_t selectedPins.
 * Output: None.
 *
 *****************************************************************************/
void GPIO_clearInterruptFlag(uint_fast8_t selectedPort, uint_fast16_t selectedPins)
{
    // Identifica el puerto seleccionado.
    uint32_t baseAddress = GPIO_PORT_TO_BASE[selectedPort];
    // Modifica el bit del registro correspondiente para limpiar la bandera de interrupcion.
    HWREG16(baseAddress + OFS_PAIFG) &= ~selectedPins;
}

/*****************************************************************************
 * Function: GPIO_getInterruptStatus
 * Preconditions: GPIO_init().
 * Overview: Retorna el valor que indica el estado de la interrupci�n de un bit del puerto en espec�fico.
 * Input: uint_fast8_t selectedPort, uint_fast16_t selectedPins.
 * Output: None.
 *
 *****************************************************************************/
uint_fast16_t GPIO_getInterruptStatus(uint_fast8_t selectedPort, uint_fast16_t selectedPins)
{
    // Identifica el puerto seleccionado.
    uint32_t baseAddress = GPIO_PORT_TO_BASE[selectedPort];
    // Retorna el valor del registro que indica el estado de una interrupci�n.
    return HWREG16(baseAddress + OFS_PAIFG) & selectedPins;
}

/*****************************************************************************
 * Function: GPIO_interruptEdgeSelect
 * Preconditions: None.
 * Overview: Selecciona el evento en que se dar� la interrupci�n para un bit de un puerto en espec�fico.
 * Input: uint_fast8_t selectedPort, uint_fast16_t selectedPins, uint_fast8_t edgeSelect.
 * Output: None.
 *
 *****************************************************************************/
void GPIO_interruptEdgeSelect(uint_fast8_t selectedPort, uint_fast16_t selectedPins, uint_fast8_t edgeSelect)
{
    // Identifica el puerto seleccionado.
    uint32_t baseAddress = GPIO_PORT_TO_BASE[selectedPort];

    // Modifica el bit del registro correspondiente para seleccionar el flanco que provoca la interrupci�n.
    if (GPIO_LOW_TO_HIGH_TRANSITION == edgeSelect)
        HWREG16(baseAddress + OFS_PAIES) &= ~selectedPins;
    else
        HWREG16(baseAddress + OFS_PAIES) |= selectedPins;
}

/*****************************************************************************
 * Function: GPIO_getInputPinValue
 * Preconditions: None.
 * Overview: Retorna el valor de entrada para un bit de un puerto en espec�fico.
 * Input: uint_fast8_t selectedPort, uint_fast16_t selectedPins.
 * Output: None.
 *
 *****************************************************************************/
uint8_t GPIO_getInputPinValue(uint_fast8_t selectedPort, uint_fast16_t selectedPins)
{
    uint_fast16_t inputPinValue;

    // Identifica el puerto seleccionado.
    uint32_t baseAddress = GPIO_PORT_TO_BASE[selectedPort];

    // Lee el registro que indica el valor de entrada de un pin.
    inputPinValue = HWREG16(baseAddress + OFS_PAIN) & (selectedPins);

    // Retorna el valor.
    if (inputPinValue > 0)
        return GPIO_INPUT_PIN_HIGH;
    return GPIO_INPUT_PIN_LOW;
}

/**********************************************************************************
 * Function: GPIO_setAsPeripheralModuleFunctionOutputPin
 * Preconditions: GPIO_init().
 * Overview: Especifica la funci�n de salida de un bit de un puerto en espec�fico.
 * Input: uint_fast8_t selectedPort, uint_fast16_t selectedPins, uint_fast8_t mode.
 * Output: None.
 *
 **********************************************************************************/
void GPIO_setAsPeripheralModuleFunctionOutputPin(uint_fast8_t selectedPort, uint_fast16_t selectedPins, uint_fast8_t mode)
{
    // Identifica el puerto seleccionado.
    uint32_t baseAddress = GPIO_PORT_TO_BASE[selectedPort];

    HWREG16(baseAddress + OFS_PADIR) |= selectedPins;
    switch (mode)
    {
    case GPIO_PRIMARY_MODULE_FUNCTION:
        HWREG16(baseAddress + OFS_PASEL0) |= selectedPins;
        HWREG16(baseAddress + OFS_PASEL1) &= ~selectedPins;
        break;
    case GPIO_SECONDARY_MODULE_FUNCTION:
        HWREG16(baseAddress + OFS_PASEL0) &= ~selectedPins;
        HWREG16(baseAddress + OFS_PASEL1) |= selectedPins;
        break;
    case GPIO_TERTIARY_MODULE_FUNCTION:
        HWREG16(baseAddress + OFS_PASEL0) |= selectedPins;
        HWREG16(baseAddress + OFS_PASEL1) |= selectedPins;
        break;
    }
}

/**********************************************************************************
 * Function: setAsPeripheralModuleFunctionInputPin
 * Preconditions: GPIO_init().
 * Overview: Especifica la funci�n de entrada de un bit de un puerto en espec�fico.
 * Input: uint_fast8_t selectedPort, uint_fast16_t selectedPins, uint_fast8_t mode.
 * Output: None.
 *
 **********************************************************************************/
void GPIO_setAsPeripheralModuleFunctionInputPin(uint_fast8_t selectedPort, uint_fast16_t selectedPins, uint_fast8_t mode)
{
    // Identifica el puerto seleccionado.
    uint32_t baseAddress = GPIO_PORT_TO_BASE[selectedPort];

    HWREG16(baseAddress + OFS_PADIR) &= ~selectedPins;
    switch (mode)
    {
    case GPIO_PRIMARY_MODULE_FUNCTION:
        HWREG16(baseAddress + OFS_PASEL0) |= selectedPins;
        HWREG16(baseAddress + OFS_PASEL1) &= ~selectedPins;
        break;
    case GPIO_SECONDARY_MODULE_FUNCTION:
        HWREG16(baseAddress + OFS_PASEL0) &= ~selectedPins;
        HWREG16(baseAddress + OFS_PASEL1) |= selectedPins;
        break;
    case GPIO_TERTIARY_MODULE_FUNCTION:
        HWREG16(baseAddress + OFS_PASEL0) |= selectedPins;
        HWREG16(baseAddress + OFS_PASEL1) |= selectedPins;
        break;
    }
}
