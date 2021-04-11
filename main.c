/*
 * Project 2.c
 *
 * Created: 15/04/2019 13:10:48
 * Student Name 1: Eoghan O'Connor 16110625
 */ 

/***Incude files***/
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/****Protyping Functions*****/
void adc_servo(void);
void sendmsg (char *s);
void servo_position(char ch);
void init_USART(void);
void init_ADC(void);
void init_TCCR(void);
void init_TIMSK(void);
void continueous_display(void);

/****Global Variables, definitions *****/
int servo_user;
/*timer variables and time period variables*/
volatile int Time_Period;
volatile int Time_Period_High;
volatile int Time_Period_Low;
unsigned char timecount;                      // Extends TCNT of Timer1
unsigned int start_edge, end_edge;           // globals for times.
int time_contmode;

/*Send message variables variables*/
unsigned char qcntr = 0,sndcntr = 0;   /*indexes into the que*/
unsigned char queue[50];       /*character queue*/
char message_temp[50];

/*Adc variables */
int adc_contmode;
int adc_flag;
volatile int adc_reading;   /* ADC is in Free Running Mode - you don't have to set up anything for */
volatile int adc_reading_mV;/*4.8828=5000/1023 this gives the ADC in mV*/
volatile double servo_pos;



/**************************************************************************************************************************************************/


/*MAIN FUNCTION*/
int main(void)
{
	DDRB = 0b00001000;
	
	time_contmode=0;
	adc_contmode=0;
	servo_user=1;
	char ch;  /* character variable for received character*/
	init_ADC();
	init_USART();
	init_TCCR();
	init_TIMSK();
	sei();
    /* Replace with your application code */
    while (1) 
    {
		int OCR2A_value=OCR2A;
		if (UCSR0A & (1<<RXC0)) /*check for character received*/
		{
			ch = UDR0;    /*get character sent from PC*/
			switch (ch)
			{	/*Number inputs for the servo position uses a function*/
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				servo_position(ch);//function to deal with number inputs
				break;
				
				/*Report the timer 555 to the data visualizer*/
				case 'T':
				case 't':
				sprintf(message_temp,"555 timer: %d msecs",Time_Period);
				sendmsg(message_temp);
				break;
				
				/*Report the low period of the 555 timer*/
				case 'L':
				case 'l':
				sprintf(message_temp,"555 timer: %d msecs",Time_Period_Low);
				sendmsg(message_temp);
				break;
				
				/*Report the high period of the 555 timer*/
				case 'h':
				case 'H':
				sprintf(message_temp,"555 timer: %d msecs",Time_Period_High);
				sendmsg(message_temp);
				break;
				
				/*Turn on continuous mode for reporting the 555 timer*/
				case 'C':
				case 'c':
				time_contmode=1;
				break;
				
				/*Turn off continuous mode for reporting the 555 timer*/
				case 'E':
				case 'e':
				time_contmode=0;
				break;
				
				/*Report the adc reading to the data visualizer*/
				case 'A':
				case 'a':
				sprintf(message_temp,"The ADC value is: %d ",adc_reading);
				sendmsg(message_temp);
				break;
				
				/*Report the adc reading in milli volts to the data visualizer*/
				case 'V':
				case 'v':
				sprintf(message_temp,"The ADC value is: %i mV",adc_reading_mV);
				sendmsg(message_temp);
				break;
				
				/*Turn on continuous mode for reporting the adc reading*/
				case 'w':
				case 'W':
			adc_contmode=1;
				break;
				
				/*Turn off continuous mode for reporting the adc reading*/
				case 'q':
				case 'Q':
			adc_contmode=0;
				break;
				
				/*Report the angle of the servo motor to the data visualizer*/
				case 'D':
				case 'd':
				sprintf(message_temp,"The angle of the servo is: %lf degrees",servo_pos);
				sendmsg(message_temp);
				break;
				
				/*Report the value of the 0CR2A to the data visualizer*/
				case 'S':
				case 's':
				sprintf(message_temp,"The value of the OCR2A is %i",OCR2A_value);
				sendmsg(message_temp);
				break;
				
				/*Is set by the adc reading */
				case 'R':
				case 'r':
				servo_user=0;
				break;
				
				/*Is set by the user */
				case 'Y':
				case 'y':
				servo_pos=1;
				break;
				
			}//switch end
		}//end of if
		
		/*function for continuous display of timer or adc readings */
		continueous_display();
		
    }// end of while
	
}//end main


/*************************************INTIALIZATION FUNCTIONS******************************************/


void init_USART()
{
	UCSR0A	= 0x00;
	UCSR0B	= (1<<RXEN0) | (1<<TXEN0) | (1<<TXC0);  /*enable receiver, transmitter and transmit interrupt*/
	UBRR0	= 102;  /*baud rate This is higher than 9600 baud rate as 9600= 103 but this is 102*/
}

void init_ADC()
{
ADMUX = ((1<<REFS0) | (2<<MUX0));  /* AVCC selected for VREF, ADC0 as ADC input  */
	ADCSRA = ((1<<ADEN)|(1<<ADATE)|(1<<ADIE)|(7<<ADPS0));/* Enable ADC, Start Conversion, Auto Trigger enabled, 
																	   Interrupt enabled, Pre-scaler = 32  */
	ADCSRB = (4<<ADTS0); /* Select AutoTrigger Source to Free Running Mode 
						    Strictly speaking - this is already 0, so we could omit the write to
						    ADCSRB, but included here so the intent is clear */
}

void init_TCCR()
{
	TCCR1A=0; // Disable all o/p waveforms
	TCCR2A = ((2<<COM2A0)|(3<<WGM20)); /* Enable Fast PWM Mode, TOP = 0xff, Enable OC2A set on match while downcounting */

	TCCR0B=(5<<CS00);	// clk/1024 as TC0 clock source
	TCCR1B = ((0<<ICES1) | (2<<CS10));     // Noise Canceler on, Rising Edge, CLK/8 (2MHz) T1 source
	TCCR2B = (7<<CS20);		/* use clk/1024 as TC2 clock source */
}
	
void init_TIMSK()
{
	TIMSK1 = ((1<<ICIE1) | (1 << TOIE1));
	TCNT0=100;
}


/************************************************************************************/
/* USART sendmsg function															*/
/*this function loads the queue and													*/
/*starts the sending process														*/
/************************************************************************************/
void sendmsg (char *s)
{
	
	qcntr = 0;    /*preset indices*/
	sndcntr = 1;  /*set to one because first character already sent*/
	
	queue[qcntr++] = 0x0d;   /*put CRLF into the queue first*/
	queue[qcntr++] = 0x0a;
	while (*s)
		queue[qcntr++] = *s++;   /*put characters into queue*/
		
	UDR0 = queue[0];  /*send first character to start process*/
}

/********************************************************************************/
/* Interrupt Service Routines													*/
/********************************************************************************/

/*this interrupt occurs whenever the */
/*USART has completed sending a character*/

ISR(USART_TX_vect)
{
	/*send next character and increment index*/
	if (qcntr != sndcntr)
	UDR0 = queue[sndcntr++];
}



ISR(TIMER1_OVF_vect){
	timecount++;
	
}
ISR(TIMER0_OVF_vect){
	timecount++;
	
}

ISR(TIMER1_CAPT_vect)
{
	unsigned long clocks;                                /* count of clocks in the pulse - not needed outside the ISR, so make it local */
	end_edge = ICR1;                                                   /* The C compiler reads two 8bit regs for us  */
	clocks = ((unsigned long)timecount * 65536) + (unsigned long)end_edge - (unsigned long)start_edge;
	timecount = 0;                                    // Clear timecount for next time around
	start_edge = end_edge;                    // We're counting rising to rising, so this end = next start
	// Save its time for next time through here

	/*Calculate time period*/
	Time_Period = Time_Period_High + Time_Period_Low;
	
	
	//If its a rising clock edge take the time period
	if(TCCR1B &(1<<ICES1)){
		Time_Period_High= clocks/2;
		TCCR1B ^= 1<<ICES1;
	}
	//If its a falling  clock edge take the time period
	else {
		Time_Period_Low= clocks/2;
		TCCR1B ^= 1<<ICES1;
	}
}


/*** ADC interrupt***/
/*
*This updates for new adc value
* This can then be displayed on the data visualizer or update the servo motor.
*/
ISR (ADC_vect)//handles ADC interrupts
{
	adc_flag=1;
	adc_reading = ADC;   /* ADC is in Free Running Mode - you don't have to set up anything for */
	adc_reading_mV= ADC*4.8876;/*4.8828=5000/1023 this gives the ADC in mV*/
	TCNT0 = 100;
	TIFR0=(1<<TOV0);//clear Timer0 overflow flag
	if(servo_user==0){
		if(adc_reading<63){
			OCR2A=21;
			servo_pos=90;
		}
		else if(adc_reading>63 &&adc_reading<=126){
			OCR2A=22;
			servo_pos=90 +(90/16);
		}
		else if(adc_reading>126 &&adc_reading<=189){
			OCR2A=23;
			servo_pos=90 +((90*2)/16);
		}
		else if(adc_reading>189 &&adc_reading<=252){
			OCR2A=24;
			servo_pos=90 +((90*3)/16);
		}
		else if(adc_reading>252 &&adc_reading<=315){
			OCR2A=25;
			servo_pos=90 +((90*4)/16);
		}
		else if(adc_reading>315 &&adc_reading<=378){
			OCR2A=26;
			servo_pos=90 +((90*5)/16);
		}
		else if(adc_reading>378 &&adc_reading<=441){
			OCR2A=27;
			servo_pos=90 +((90*6)/16);
		}
		else if(adc_reading>441 &&adc_reading<=504){
			OCR2A=28;
			servo_pos=90 +((90*7)/16);
		}
		else if(adc_reading>504 &&adc_reading<=567){
			OCR2A=29;
			servo_pos=90 +((90*8)/16);
		}
		else if(adc_reading>567 &&adc_reading<=630){
			OCR2A=30;
			servo_pos=90 +((90*9)/16);
		}
		else if(adc_reading>630 &&adc_reading<=693){
			OCR2A=31;
			servo_pos=90 +((90*10)/16);
		}
		else if(adc_reading>693 &&adc_reading<=756){
			OCR2A=32;
			servo_pos=90 +((90*11)/16);
		}
		else if(adc_reading>756 &&adc_reading<=819){
			OCR2A=33;
			servo_pos=90 +((90*12)/16);
		}
		else if(adc_reading>819 &&adc_reading<=882){
			OCR2A=34;
			servo_pos=90 +((90*13)/16);
		}
		else if(adc_reading>882 &&adc_reading<=945){
			OCR2A=35;
			servo_pos=90 +((90*14)/16);
		}
		else{
			OCR2A=37;
			servo_pos=180;
		}
	}//end of servo_user if statement
}// end of function




/*********************************************************************************************/

/**FUNCTIONS**/

/***Servo Position***/
/*
*If the servo user is selected i.e that the user controls the angle of the servo using
* inputs 1 to 8
*/
void servo_position(char ch)
{
	if(servo_user){
	if(ch=='1'){
		OCR2A=21;
		servo_pos= 90;
	}
	if(ch=='2'){
		OCR2A=23;
		servo_pos= 90+12.86;
	}
	if(ch=='3'){
		OCR2A=25;
		servo_pos= 90 +(12.86*2);
	}
	if(ch=='4'){
		OCR2A=27;
		servo_pos= 90 +(12.86*3);
	}
	if(ch=='5'){
		OCR2A=29;
		servo_pos= 90 +(12.86*4);
	}
	if(ch=='6'){
		OCR2A=31;
		servo_pos= 90 +(12.86*5);
	}
	if(ch=='7'){
		OCR2A=33;
		servo_pos= 90 +(12.86*6);
	}
	if(ch=='8'){
		OCR2A=37;
		servo_pos= 180;
	}
	}//end of servo_user if statement
}//end of function


/*
* Continuous function
* This displays either the adc reading or the 555 timer if either are selected 
*/
void continueous_display(void)
{
	if(qcntr == sndcntr){
	if(time_contmode)
	{
		sprintf(message_temp,"555 timer: %d msecs",Time_Period);
		sendmsg(message_temp);
	}
	else if(adc_contmode){
		if(adc_flag){
		sprintf(message_temp,"The ADC value is: %i mV",adc_reading_mV);
		sendmsg(message_temp);
		adc_flag=0;
		}//end of flag if
		
	}//end of adc contmode if
	
	}//end of 1st if
	
}//end of function


