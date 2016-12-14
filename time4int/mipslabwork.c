/* mipslabwork.c

   This file written 2015 by F Lundevall

   This file should be changed by YOU! So add something here:

   This file modified 2015-12-24 by Ture Teknolog 

   Latest update 2015-08-28 by F Lundevall

   For copyright and licensing, see file COPYING */

#include <stdint.h>   /* Declarations of uint_32 and the like */
#include <pic32mx.h>  /* Declarations of system-specific addresses etc */
#include "mipslab.h"  /* Declatations for these labs */
#include <stdbool.h>

#define DISPLAY_VDD_PORT PORTF
#define DISPLAY_VDD_MASK 0x40
#define DISPLAY_VBATT_PORT PORTF
#define DISPLAY_VBATT_MASK 0x20
#define DISPLAY_COMMAND_DATA_PORT PORTF
#define DISPLAY_COMMAND_DATA_MASK 0x10
#define DISPLAY_RESET_PORT PORTG
#define DISPLAY_RESET_MASK 0x200

/* Address of the temperature sensor on the I2C bus */
#define TEMP_SENSOR_ADDR 0x48

//Portinitiation
volatile int* porte = (volatile int*) 0xbf886110; //För lamporna

#define TMR2PERIOD ((80000000 / 256) / 10)
#if TMR2PERIOD > 0xffff
#error "Timer period is too big."
#endif

int timeoutcount=0;
int prime = 1234567;
int mytime = 0x0000;
int maxtime = 0x0200;
char textstring[] = "text, more text, and even more text!";
char buf[32], *s, *t, *mte, *mti, *ti;
int unit = 0; //Unitvalue, 0 is C, 32 is F, 273 is K
int maxtemp = 0x01c00;
char tempunit ='C'; //Set what unit of Temperature we are using, Celcius by default
uint16_t temp;


/* Temperature sensor internal registers */
typedef enum TempSensorReg TempSensorReg;
enum TempSensorReg {
  TEMP_SENSOR_REG_TEMP,
  TEMP_SENSOR_REG_CONF,
  TEMP_SENSOR_REG_HYST,
  TEMP_SENSOR_REG_LIMIT,
};



/* Interrupt Service Routine */
void user_isr( void )
{
  if(IFS(0) & 0x100){
    timeoutcount++;
    IFS(0) = 0;
  }
  if(timeoutcount == 10){
    time2string( textstring, mytime );
    display_string( 3, textstring );
    display_update();
    tick( &mytime );
    timeoutcount = 0;

    volatile int * porte = (volatile int *) 0xbf886110;
    *porte += 0x1;
  }
  int choice=getbtns();
  if(choice){
    int swi = getsw();
    if(choice & 0x1){
      mytime = mytime & 0xff0f;
      swi = swi << 4;
      mytime = mytime | swi;
    }
    if(choice & 0x2){
       mytime = mytime & 0xf0ff;
       swi = swi << 8;
       mytime = mytime | swi;
    }
    if(choice & 0x4){
      mytime = mytime & 0xfff;
      swi = swi << 12;
      mytime = mytime | swi;
      }
    }
}


/* Lab-specific initialization goes here */
void labinit( uint16_t tempe )
{
  temp = tempe;
  IEC(0) = 0x100;
  IPC(2) = 4;
  volatile int * trise = (volatile int *) 0xbf886100;
  *trise = *trise & 0xfff0;
  TRISD = 0xf80f;
  TRISDSET = (0x7f << 5);  
  T2CON = 0x70;
  PR2 = TMR2PERIOD;
  TMR2 = 0;
  T2CONSET = 0x8000; /* Start the timer */


  display_init();
  display_string(0, "Temp:");
  
  //mte = fixed_to_string(maxtemp, buf);
  display_string(1, "Maxtemp: ");
  //display_string(1, mte);
  
  display_string(2, "Time: ");
  
  display_string(3, "Maxtime: ");
  
  display_update();
  /* Own code ends here */

  
  /* Send start condition and address of the temperature sensor with
  write mode (lowest bit = 0) until the temperature sensor sends
  acknowledge condition */
  do {
    i2c_start();
  } while(!i2c_send(TEMP_SENSOR_ADDR << 1));
  /* Send register number we want to access */
  i2c_send(TEMP_SENSOR_REG_CONF);
  /* Set the config register to 0 */
  i2c_send(0x0);
  /* Send stop condition */
  i2c_stop();  
  enable_interrupt();
}

/* This function is called repetitively from the main program */
void labwork( void )
{
  do {
      i2c_start();
    } while(!i2c_send(TEMP_SENSOR_ADDR << 1));
    /* Send register number we want to access */
    i2c_send(TEMP_SENSOR_REG_TEMP);
    
    /* Now send another start condition and address of the temperature sensor with
    read mode (lowest bit = 1) until the temperature sensor sends
    acknowledge condition */
    do {
      i2c_start();
    } 
    while(!i2c_send((TEMP_SENSOR_ADDR << 1) | 1));
    
    /* Now we can start receiving data from the sensor data register */
    temp = i2c_recv() << 8;
    i2c_ack();
    temp |= i2c_recv();
    /* To stop receiving, send nack and stop */
    i2c_nack();
    i2c_stop();
    
    /* Temperatursutskrift börjar */ //RAD 0
    s = fixed_to_string(temp, buf);
    t = s + strlen(s);
    *t++ = ' ';
    *t++ = tempunit;
    *t++ = 0;
    display_string(0, s);
    /* Temperatursutskrift slutar */

    /* Maxtemperatursrelevans börjar*/
   /* mte = fixed_to_string(maxtemp, buf);
    t = mte + strlen(mte);
    *t++ = ' ';
    *t++ = tempunit;
    *t++ = 0;*/
/*    if(tempcheck(temp)){
      //Skriv Warning till skärmen.
      display_string(1, "WARNING  WARNING ");
      *porte |= 0x55; //Täänd varannan LED, från position 0
    }
    else{
      display_string(1, "Maxtemp: ");
      //display_string(1, mte, 10);
      *porte &= ~0x55; //Invertera de tända LEDsen
    }*/
    if(IFS(0) & 0x80){ //Lyssnar på interrupt från SW1
      maxtemp += 0x0100;
      }
      if(IFS(0) & 0x800){ //Lyssnar på interrupt från SW2
      maxtemp -= 0x0100;
      }
      if(IFS(0) & 0x8000){
        if(tempunit !='C'){
          tempunit ='C';
          unit = 0;
        }
      }
      if(IFS(0) & 0x80000){ //Lyssnar på interrupt från SW4
                  //Ändrar till Celcius
        if(tempunit =='C'){
          tempunit ='F';
          unit = 0x20; //set unit to 32
        }
      }  
    /* Maxtemperatursrelevans slutar*/

    /* Tidsutskrift börjar */
    //display_string(2, "Time: ", 0);
    if(IFS(0) & 0x100){ //Lyssnar på interrupts från timern.
        timeoutcount++;
        IFS(0) = 0;
        if(timeoutcount == 10){
          time2string( textstring, mytime );
          //ti = fixed_to_string(mytime, buf);
          display_string(2, textstring);
          display_update();
          mytime += 0x100;
          timeoutcount = 0;
        }
    }
  
    /* Tidsutskrift slutar */

    /* Maxtidsutskrift börjar */
/*    mti = fixed_to_string(maxtime, buf);
    t = mti + strlen(mti);
    *t++ = ' ';
    *t++ = tempunit;
    *t++ = 0;*/
    /*if (timecheck(mytime)){
      display_string(3, "TIME OVER");
      *porte |= 0xAA; //Tänd de leds som inte tänds för det andra alarmet
    }
    else{
      display_string(3, "Maxtime: ");
      //display_string(3, nått, 8);
      //display_string(3, mti, 10);
      *porte &= ~0xAA; //Släck
    }*/
    
    /* Maxtidsutskrift slutar*/

    display_update();
    delay(1000000);





/*  prime = nextprime( prime );
  display_string( 0, itoaconv( prime ) );
  display_update();*/
}
