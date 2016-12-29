/*
===============================================================================
 Name        : main.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
*/


#include <cr_section_macros.h>
//#include <cstdint>

// TODO: insert other include files here
#include  "chip.h"
#include "board_configure.h"
#include "timer.h"
#include "led.h"
#include "usart.h"

// TODO: insert other definitions and declarations here



#define _8N1 0x01                 // define uart configuration parameter


/* Configuration parameter for KWP UART */
static UART_CONFIG_T KWP_UART_CFG  =
{
  14976000, // uart common clock
  10400,    // baud rate
  _8N1,     // 8 data length, no parity, 1 stop
  0,        // Asynchronous Mode
  NO_ERR_EN // Enable No Errors
};


/* Configuration parameter for bluetooth UART */
static UART_CONFIG_T BLUE_TOOTH__UART_CFG  =
{
  14976000, // uart common clock
  115200,    // baud rate
  _8N1,     // 8 data length, no parity, 1 stop
  0,        // Asynchronous Mode
  NO_ERR_EN // Enable No Errors
};

//static rx buffer values ;
const int MAX_RX_LENGTH = 64;
static char rx_buffer[MAX_RX_LENGTH];
static int rx_index = 0;
static bool rx_ready = false;
static bool rx_message_too_long = false;
static char str[] = "Message is too long, please resend in a shorter length!";
static char end = '\n';

/* bluetooth uart pointer */
UART *pBluetoothUart = UART::Instance(LPC_UART0_CHANNEL);

/* callback for bluetooth uart rx processing */
void Bluetooth_Rx_Callback(char ch);
/* callback for Longtimer */
void LongTimer_Callback();

int main(void)
{

    // TODO: insert code here

	//Board_Init();
    Board_Configure::instance()->Configure();

    /* for getting system main clock */
    uint32_t MianClock = Board_Configure::instance()->Get_MainClockRate();



    /* configure and initialize bluetooth uart instance */
    pBluetoothUart->Uart_Init(&BLUE_TOOTH__UART_CFG);

   // RingBuffer_Init(&ring_buffer,rx_buffer,1,64);
    pBluetoothUart->Set_RX_Callback(Bluetooth_Rx_Callback);




    Led * pLed = Led::instance();
    LongTimer * pLongtimer = LongTimer::Instance(LongTimer_Callback);

    pLed->Lighten_Led_TX();
    pLed->Lighten_Led_RX();

    pLed->Off_Led_TX();
    //Led::instance()->Off_Led_RX();
    pLed->Toggle_Led_TX();
    pLed->Toggle_Led_RX();

    // Force the counter to be placed into memory*/
	volatile static uint32_t i = 0 ;

    // Enter an infinite loop, just incrementing a counter

	/* the MRT timer can maximum delay the timer for 233ms */
	//led_blink_timer->Start_Millisecond(200);

	/* start the LongTimer */
	pLongtimer->Start_Millisecond(1000);
    while(1)
    {
    	/*if(rx_message_too_long || rx_ready)
    	{
    		if(rx_message_too_long)
    		{
				rx_index = 0;
				rx_message_too_long = false;
    		}
    		else
    		{

    		}
    	}*/

    	//char ch = 0xf0;
    	if(pLongtimer->IsExpired())
    	{
    		pBluetoothUart->Send("Topon-Edison!\n",14);

    		pLongtimer->Start_Millisecond(1000);
    		//pLed->Blink_Led_RX();
    		//pLed->Blink_Led_TX();
    	}
    	i++;
    }
    return 0 ;
}


void LongTimer_Callback()
{
	Led * led = Led::instance();
	//led->Toggle_Led_TX();
	//led->Toggle_Led_RX();
}


void Bluetooth_Rx_Callback(char ch)
{
	if(rx_index == MAX_RX_LENGTH)// received message is longer than rx_buffer length
	{
		//rx_message_too_long = true;
		pBluetoothUart->Send("Message is too long, please resend in a shorter length!",55);
		return;
	}
	rx_buffer[rx_index++] = ch;
	if(ch == 't')
	{
		// echo back message
		pBluetoothUart->Send(rx_buffer,rx_index);
		rx_index = 0;
	}


}
