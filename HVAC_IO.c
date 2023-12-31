 // FileName:        HVAC_IO.c
 // Dependencies:    HVAC.h
 // Processor:       MSP432
 // Board:           MSP432P401R
 // Program version: CCS V8.3 TI
 // Company:         Texas Instruments
 // Description:     Funciones de control a traves de estados.

#include "HVAC.h"

/* Variables sobre las cuales se maneja el sistema. */
char state[MAX_MSG_SIZE];                       // Cadena a imprimir.
bool toggle = 0;

uint32_t delay;                                 // Delay

// Variables de estado de los botones principales
bool Enc_Apg_push = FALSE;                      // Pulsacion del boton ON/OFF
bool Menu_Push = FALSE;                         // Pulsacion del boton Menu
bool UP_DOWN_Push = FALSE;                      // Pulsacion del boton UP/DOWN


/* **** SE DECLARARON LAS VARIABLES Y FUNCIONES PARA REALIZAR EL DALAY CON EL TIMER ******** */
extern void Timer32_INT1 (void);                // Funcion de interrupcion.
extern void Delay_ms (uint32_t time);           // Funcion de delay.
float lum[3];

/*FUNCTION******************************************************************************
*
* Function Name    : System_InicialiceTIMER
* Returned Value   : None.
* Comments         : Controla los preparativos para poder usar TIMER32
*
*END***********************************************************************************/
void System_InicialiceTIMER (void)
{
    T32_Init1();
    Int_registerInterrupt(INT_T32_INT1, Timer32_INT1);
    Int_enableInterrupt(INT_T32_INT1);
}

/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_InicialiceADC
* Returned Value   : None.
* Comments         :
*    Inicializa las configuraciones deseadas para
*    el modulo general ADC y los tres canales a utilizar
*
*END***********************************************************************************/
void HVAC_InicialiceADC(void)
{
    // Iniciando ADC y canales.
    ADC_Initialize(ADC_14bitResolution, ADC_CLKDiv8);
    ADC_SetConvertionMode(ADC_SequenceOfChannels);
    ADC_ConfigurePinChannel(LUM1, AN8, ADC_VCC_VSS);            /*pin 4.5. se enlaza el canal a la entrada analoga de la tarjeta para realizar la lectura*/
    ADC_ConfigurePinChannel(LUM2, AN9, ADC_VCC_VSS);            /*pin 4.4.*/
    ADC_ConfigurePinChannel(LUM3, AN10, ADC_VCC_VSS);           /*Pin 4.3*/
    ADC_SetStartOfSequenceChannel(AN8);                         /*Empieza la secuencia de lectura en el analogo 8*/
    ADC_SetEndOfSequenceChannel(AN10);                          /*Termina la secuencia de lectura en el analogo 10*/
}

/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_InicialiceUART
* Returned Value   : None.
* Comments         :
*    Inicializa las configuraciones deseadas para
*    configurar el modulo UART (comunicacion asincrona).
*
*END***********************************************************************************/
void HVAC_InicialiceUART (void)
{
    UART_init();
}

/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_InicialiceIO
* Returned Value   : None.
* Comments         :
*    Controla los preparativos para poder usar las entradas y salidas GPIO.
*
*END***********************************************************************************/
void HVAC_InicialiceIO(void)
{
    // Para entradas y salidas ya definidas en la tarjeta.
    GPIO_init_board();

    // Modo de interrupcion de los botones principales.
    GPIO_interruptEdgeSelect(ON_OFF_PORT,BIT(ON_OFF),  GPIO_HIGH_TO_LOW_TRANSITION);
    GPIO_interruptEdgeSelect(MENU_PORT,BIT(MENU_BTN), GPIO_HIGH_TO_LOW_TRANSITION);
    GPIO_interruptEdgeSelect(UP_DOWN_PORT,BIT(UP_BTN), GPIO_LOW_TO_HIGH_TRANSITION);
    GPIO_interruptEdgeSelect(UP_DOWN_PORT,BIT(DOWN_BTN), GPIO_LOW_TO_HIGH_TRANSITION);

    // Preparativos de interrupcion.
    GPIO_clear_interrupt_flag(P1,B1);
    GPIO_clear_interrupt_flag(P1,B4);
    GPIO_clear_interrupt_flag(P2,B4);
    GPIO_clear_interrupt_flag(P2,B5);

    GPIO_enable_bit_interrupt(P1,B1);
    GPIO_enable_bit_interrupt(P1,B4);
    GPIO_enable_bit_interrupt(P2,B4);
    GPIO_enable_bit_interrupt(P2,B5);


    /* Uso del modulo Interrupt para generar la interrupcion general y registro de esta en una funcion
    *  que se llame cuando la interrupcion se active. */
    Int_registerInterrupt(INT_PORT1, INT_SWI);
    Int_registerInterrupt(INT_PORT2, INT_UP_DOWN);
    Int_enableInterrupt(INT_PORT1);
    Int_enableInterrupt(INT_PORT2);
}

/*******************************************************************************************/

/**********************************************************************************
 * Function: INT_SWI
 * Preconditions: Interrupcion habilitada, registrada e inicializacion de modulos.
 * Overview: Funcion que es llamada cuando se genera
 *           la interrupcion del boton ON/OFF o menu.
 * Input: None.
 * Output: None.
 **********************************************************************************/
void INT_SWI(void)
{
    GPIO_clear_interrupt_flag(P1,B1);                           // Limpia la bandera de la interrupcion.
    GPIO_clear_interrupt_flag(P1,B4);                           // Limpia la bandera de la interrupcion.

    if(!GPIO_getInputPinValue(ON_OFF_PORT,BIT(ON_OFF)))         // Si se pulsa el boton de encendido y apagado.
        HVAC_Enc_Apg_Ctrl();
    if(!GPIO_getInputPinValue(MENU_PORT,BIT(MENU_BTN))){        // Si se pulsa el boton de menu.
        Select_Menu += 0x01;                                    // Cambia la seleccion
        if(Select_Menu > 0x03)                                  // Reinicia la seleccion cuando se pasa de 3
            Select_Menu = 0x01;
        while(!GPIO_getInputPinValue(MENU_PORT,BIT(MENU_BTN))); //Para evitar el rebote
        HVAC_Menu();                                            // Imprime el menu
    }
    return;
}

/**********************************************************************************
 * Function: INT_UP_DOWN
 * Preconditions: Interrupcion habilitada, registrada e inicializacion de modulos.
 * Overview: Funcion que es llamada cuando se genera la interrupcion del
 *           boton UP/DOWN.
 *
 **********************************************************************************/
void INT_UP_DOWN(void)
{
    GPIO_clear_interrupt_flag(P2,B4);                       // Limpia la bandera de la interrupcion.
    GPIO_clear_interrupt_flag(P2,B5);                       // Limpia la bandera de la interrupcion.
    if(Enc_Apg != APAGADO){                                 //No imprime si el sistema esta apagado

        // Si se pulsa el boton UP
        if(GPIO_getInputPinValue(UP_DOWN_PORT,BIT(UP_BTN)) != NORMAL_STATE_EXTRA_BUTTONS){
            switch(Select_Menu){
                case P1_SELECTED:  Persiana1.Estado = Up; break;
                case P2_SELECTED:  Persiana2.Estado = Up; break;
                case SL_SELECTED:  SecuenciaLED.Estado = Up; break;
                case DEFAULT: UART_putsf(MAIN_UART,"\n\r Selecciona una opcion con el boton MENU \n\r");
            }
        }

        // Si se pulsa el boton DOWN
        if(GPIO_getInputPinValue(UP_DOWN_PORT,BIT(DOWN_BTN)) != NORMAL_STATE_EXTRA_BUTTONS){
            switch(Select_Menu){
                case P1_SELECTED:  Persiana1.Estado = Down; break;
                case P2_SELECTED:  Persiana2.Estado = Down; break;
                case SL_SELECTED:  SecuenciaLED.Estado = Down; break;
                case DEFAULT: UART_putsf(MAIN_UART,"\n\r Selecciona una opcion con el boton MENU \n\r");
            }
        }
        UP_DOWN_Push = TRUE;
    }
    return;
}

/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_ActualizarEntradas
* Returned Value   : None.
* Comments         :
*    Actualiza los variables indicadores de las entradas sobre las cuales surgiran
*    las salidas.
*
*END***********************************************************************************/
void HVAC_ActualizarEntradas(void)
{
    //Lectura de los potenciometros
    //Se mapean los valores dados por el ADC para entregar valores de 0 a 10 luxes
    //La formula usada es: (valor del ADC/Valor maximo del ADC) * valor maximo deseado
    ADC_trigger();
    while(ADC_is_busy());
    lum[0] = (ADC_result(LUM1) * 10) / MAX_ADC_VALUE;

    ADC_trigger();
    while(ADC_is_busy());
    lum[1] = (ADC_result(LUM2) * 10) / MAX_ADC_VALUE;

    ADC_trigger();
    while(ADC_is_busy());
    lum[2] = (ADC_result(LUM3) * 10) / MAX_ADC_VALUE;

}

/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_PrintState
* Returned Value   : None.
* Comments         :
*    Imprime via UART la situacion actual del sistema en terminos de luminicencia
*    actual y el estado de las persianas.
*    Imprime cada cierto numero de iteraciones y justo despues de recibir un cambio
*    en las entradas, produciendose un inicio en las iteraciones.
*END***********************************************************************************/
void HVAC_PrintState(void)
{
    static char iterations = 0;
    iterations++;

    if(iterations >= ITERATIONS_TO_PRINT)
    {
        sprintf(state,"LUZ_1: %0.2f, LUZ_2: %0.2f  LUZ_3: %0.2f \n\r",lum[0], lum[1], lum[2]);
        UART_putsf(MAIN_UART,state);

        sprintf(state,"Persiana 1: %s, Persiana 2: %s, Secuencia LEDs: %s\n\r\n\r",
                    (Persiana1.Estado == Up? "UP":"DOWN"),
                    (Persiana2.Estado == Up? "UP":"DOWN"),
                    (SecuenciaLED.Estado == Up? "ON":"OFF"));
        UART_putsf(MAIN_UART,state);

        iterations = 0;

        //Si se activa la secuencia...
        if(SecuenciaLED.Estado == Up){
            //Intercambia entre LED Rojo y Azul
            toggle ^= 1;
            GPIO_setOutput(LED_RGB_PORT, LED_Rojo, toggle);
            GPIO_setOutput(LED_RGB_PORT, LED_Azul, !toggle);
        }
        //Si no se activa o se desactiva permanece apagado
        else{
            GPIO_setOutput(LED_RGB_PORT, LED_Rojo, 0);
            GPIO_setOutput(LED_RGB_PORT, LED_Azul, 0);
        }
    }

    usleep(DELAY);
}

/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_Enc_Apg_Check
* Returned Value   : None.
* Comments         : Verifica el estado de la pulsacion del boton ON/OFF y controla
*                    el delay para el encendido y apagado del programa.
*                    Tambien controla el delay del menu
*
*END***********************************************************************************/
void HVAC_Enc_Apg_Check(void)
{
    // Al pulsar el boton ON/OFF o el boton de menu...
    if (Enc_Apg_push == TRUE || Menu_Push == TRUE){

        // Si se pulsa el boton para encender...
        if(contadorApg == 0x00)
            Delay_ms(1000);                             // Espera 1 segundo

        // Si se pulsa el boton para apagar...
        else if(contadorApg > 0x00)
            Delay_ms(5000);                             // Espera 5 segundos
    }
    else if(UP_DOWN_Push == TRUE){

        //Si no hay seleccion o esta seleccionado SL no hay espera
        if(Select_Menu != DEFAULT && Select_Menu != 0x03){
            UART_putsf(MAIN_UART,"\n\rEspere 5 segundos... ");
            Delay_ms(4000);
        }

        Delay_ms(1000);
        HVAC_Menu();
    }

    UP_DOWN_Push = FALSE;
    Menu_Push = FALSE;
    Enc_Apg_push = FALSE;
}

/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_Enc_Apg_Ctrl
* Returned Value   : None.
* Comments         : Controla el encendido y apagado del programa.
*
*END***********************************************************************************/
void HVAC_Enc_Apg_Ctrl(void)
{
    //Si se pulsa el boton con el sistema apagado...
    if(Enc_Apg == APAGADO){
        Enc_Apg = ENCENDIDO;                                //Se enciende el sistema
        UART_putsf(MAIN_UART,"SISTEMA ENCENDIDO\n\r");      //Se informa al usuario
        GPIO_setOutput(BSP_LED1_PORT,  BSP_LED1,  1);       //Enciende el LED rojo
    }

    //Si se pulsa el boton con el sistema encendido...
    else if (Enc_Apg == ENCENDIDO){
        contadorApg = contadorApg + 1;                      //Aumenta el contador de apagado

        //Si se pulsa dos veces el boton de apagado...
        if(contadorApg == 0x02){
            Enc_Apg = APAGADO;                              //Se apaga el sistema
            UART_putsf(MAIN_UART,"SISTEMA APAGADO\n\r");    //Se informa al usuario
            GPIO_setOutput(BSP_LED1_PORT,  BSP_LED1,  0);   //Apaga el LED rojo
        }

        //Si se pulsa menos de dos veces el boton de apagado...
        else if(contadorApg < 0x02){
            UART_putsf(MAIN_UART,"Para apagar vuelva a presionar el boton\n\r");    //Imprime instrucciones
        }
    }
    Enc_Apg_push = TRUE;   //Controla los segundos de "delay" de cada estado (ON/OFF)
}

/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_Menu
* Returned Value   : None.
* Comments         : Imprime la seleccion actual del menu y su estado.
*
*END***********************************************************************************/
void HVAC_Menu(void){

    //No imprime si el sistema esta apagado
    if(Enc_Apg != APAGADO){
        switch(Select_Menu){
            case P1_SELECTED:  sprintf(state, "\n\rP1_SELECTED  STATUS: %s\n\r\n\r"
                               ,(Persiana1.Estado == Up? "UP":"DOWN"));
                               UART_putsf(MAIN_UART, state);
                               break;
            case P2_SELECTED:  sprintf(state, "\n\rP2_SELECTED  STATUS: %s\n\r\n\r"
                               ,(Persiana2.Estado == Up? "UP":"DOWN"));
                               UART_putsf(MAIN_UART, state);
                               break;
            case SL_SELECTED:  sprintf(state, "\n\rSL_SELECTED  STATUS: %s\n\r\n\r"
                               ,(SecuenciaLED.Estado == Up? "ON":"OFF"));
                               UART_putsf(MAIN_UART, state);
                               break;
        }
        Menu_Push = TRUE;
    }
}
