/*
 * I2C Example project for the mcb32 toolchain
 * Demonstrates the temperature sensor and display of the Basic IO Shield
 * Make sure your Uno32-board has the correct jumper settings, as can be seen
 * in the rightmost part of this picture:
 * https://reference.digilentinc.com/_media/chipkit_uno32:jp6_jp8.png?w=300&tok=dcceb2
 */


/*
	Vad ska programmet göra?
	Den ska by default visa temp, maxtemp, timer samt alarmtid.
	Om tempen visar för högt skall även en varningstext skrivas ut,
	likaså skall varannan led blinka.

	Om tiden överstiger alarmtiden ska de andra ledsen lysa samt någon typ av varningstext.

	SW1 = Visa Celcius när SW1 är = 1
	SW2 = Visa Fahrenheit när SW2 är = 1
	

	Maxtemperaturen skall gå att justera med hjälp av två av de andra switcharna.
	Alarmtiden skall gå att justera med hjälp utav två knappar, två och tre. Den fjärde knappen är till för att snooza alarmet.
*/

#include <pic32mx.h>
#include <stdint.h>
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


/* Own code starts here */


//Needed for timer
#define TMR2PERIOD ((80000000 / 256) / 10)
#if TMR2PERIOD > 0xffff
#error "Timer period is too big."
#endif

//Portinitiation
volatile int* porte = (volatile int*) 0xbf886110; //För lamporna

//Global Variables
char textstring[] = " ";
int timeoutcount = 0;
int mytime = 0x0000;
int maxtime = 0x0500;
int unit = 0; //Unitvalue, 0 is C, 32 is F, 273 is K
int maxtemp = 0x01c00;
char tempunit ='C'; //Set what unit of Temperature we are using, Celcius by default


int getsw( void ){
	/*
		de 4 msb av returvärdet innehåller värdet från
		switcharna SW4, SW3, SW2, SW1.
		SW1 är den minst signifikanta biten.
		Alla andra bitar måste vara 0.
		Funktionen kan inte kallas om PORTD inte initierats rätt
		Switcharna är connectade genom bitarna 11 till 8
		i PORTD
		
	*/
	int switches = PORTD >> 8;
	switches = switches & 0xf;
	return switches;	
}

int getbtns( void ){
	/*
		Ska returnera de 3 msb från knapparna btn4, btn3 och btn2 och btn1 från kortet.
		BTN1 är den msb.
		Alla andra bitar ska vara 0. (maska med 0xf?)
		Bitarna 4, 5, 6 och 7 i PORTD är knapparna.
	*/
	int btns = PORTD >> 4;
	btns = btns & 0xf;
	return btns;
}
void switchcheck( checksw ){
	/*
		Ändrar värden beroende på switcharnas värde
	*/
	if(checksw == 1){
		//Öka maxtemperaturen med 1 grad celcius
		//maxtemp += 0x0100;
	}
	//varför funkar inte (checksw & 0x100)
	else if(checksw == 2){
		//Minska maxtemperaturen med 1 grad celcius
		//maxtemp -= 0x0100;
	}
	else if(checksw == 4){
		//makes kelvin
		//tempunit ='K';
		//unit = 0x111; //set unit to 273
		//tempunit ='C';
		//unit = 0;
	}
	else if(checksw == 8){
		//tempunit ='F';
		//unit = 0x20; //set unit to 32
	}
}

void buttoncheck( checkbtn ){
	if(checkbtn = 1){ //btn1
		//Öka tiden med 1 sek
		maxtime += 0x0100;
	}
	else if(checkbtn = 2){ //btn2
		//Minska tiden med 1 sek
		maxtime -= 0x0100;

	}
	else if(checkbtn = 4){ //btn3
		//Ingen implementerad funktion

	}
	else if(checkbtn = 8){ //btn4
		//Nollställ timer, dvs snooze
		mytime = 0;
	}
}
bool tempcheck(temp){
		/* Maxtemp-Check */
	if(maxtemp <= temp){ 
		return true;
	}
	else{
		
		return false;
	}

}
bool timecheck(mytime){
	/*Kolla om tiden gått över*/
	if(maxtime <= mytime){
		return true;
	}
	else{
		return false;
	}
}
/* Own code ends here*/

/* Temperature sensor internal registers */
typedef enum TempSensorReg TempSensorReg;
enum TempSensorReg {
	TEMP_SENSOR_REG_TEMP,
	TEMP_SENSOR_REG_CONF,
	TEMP_SENSOR_REG_HYST,
	TEMP_SENSOR_REG_LIMIT,
};


char textbuffer[4][16];

static const uint8_t const font[] = {
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 2, 5, 2, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 94, 0, 0, 0, 0,
	0, 0, 4, 3, 4, 3, 0, 0,
	0, 36, 126, 36, 36, 126, 36, 0,
	0, 36, 74, 255, 82, 36, 0, 0,
	0, 70, 38, 16, 8, 100, 98, 0,
	0, 52, 74, 74, 52, 32, 80, 0,
	0, 0, 0, 4, 3, 0, 0, 0,
	0, 0, 0, 126, 129, 0, 0, 0,
	0, 0, 0, 129, 126, 0, 0, 0,
	0, 42, 28, 62, 28, 42, 0, 0,
	0, 8, 8, 62, 8, 8, 0, 0,
	0, 0, 0, 128, 96, 0, 0, 0,
	0, 8, 8, 8, 8, 8, 0, 0,
	0, 0, 0, 0, 96, 0, 0, 0,
	0, 64, 32, 16, 8, 4, 2, 0,
	0, 62, 65, 73, 65, 62, 0, 0,
	0, 0, 66, 127, 64, 0, 0, 0,
	0, 0, 98, 81, 73, 70, 0, 0,
	0, 0, 34, 73, 73, 54, 0, 0,
	0, 0, 14, 8, 127, 8, 0, 0,
	0, 0, 35, 69, 69, 57, 0, 0,
	0, 0, 62, 73, 73, 50, 0, 0,
	0, 0, 1, 97, 25, 7, 0, 0,
	0, 0, 54, 73, 73, 54, 0, 0,
	0, 0, 6, 9, 9, 126, 0, 0,
	0, 0, 0, 102, 0, 0, 0, 0,
	0, 0, 128, 102, 0, 0, 0, 0,
	0, 0, 8, 20, 34, 65, 0, 0,
	0, 0, 20, 20, 20, 20, 0, 0,
	0, 0, 65, 34, 20, 8, 0, 0,
	0, 2, 1, 81, 9, 6, 0, 0,
	0, 28, 34, 89, 89, 82, 12, 0,
	0, 0, 126, 9, 9, 126, 0, 0,
	0, 0, 127, 73, 73, 54, 0, 0,
	0, 0, 62, 65, 65, 34, 0, 0,
	0, 0, 127, 65, 65, 62, 0, 0,
	0, 0, 127, 73, 73, 65, 0, 0,
	0, 0, 127, 9, 9, 1, 0, 0,
	0, 0, 62, 65, 81, 50, 0, 0,
	0, 0, 127, 8, 8, 127, 0, 0,
	0, 0, 65, 127, 65, 0, 0, 0,
	0, 0, 32, 64, 64, 63, 0, 0,
	0, 0, 127, 8, 20, 99, 0, 0,
	0, 0, 127, 64, 64, 64, 0, 0,
	0, 127, 2, 4, 2, 127, 0, 0,
	0, 127, 6, 8, 48, 127, 0, 0,
	0, 0, 62, 65, 65, 62, 0, 0,
	0, 0, 127, 9, 9, 6, 0, 0,
	0, 0, 62, 65, 97, 126, 64, 0,
	0, 0, 127, 9, 9, 118, 0, 0,
	0, 0, 38, 73, 73, 50, 0, 0,
	0, 1, 1, 127, 1, 1, 0, 0,
	0, 0, 63, 64, 64, 63, 0, 0,
	0, 31, 32, 64, 32, 31, 0, 0,
	0, 63, 64, 48, 64, 63, 0, 0,
	0, 0, 119, 8, 8, 119, 0, 0,
	0, 3, 4, 120, 4, 3, 0, 0,
	0, 0, 113, 73, 73, 71, 0, 0,
	0, 0, 127, 65, 65, 0, 0, 0,
	0, 2, 4, 8, 16, 32, 64, 0,
	0, 0, 0, 65, 65, 127, 0, 0,
	0, 4, 2, 1, 2, 4, 0, 0,
	0, 64, 64, 64, 64, 64, 64, 0,
	0, 0, 1, 2, 4, 0, 0, 0,
	0, 0, 48, 72, 40, 120, 0, 0,
	0, 0, 127, 72, 72, 48, 0, 0,
	0, 0, 48, 72, 72, 0, 0, 0,
	0, 0, 48, 72, 72, 127, 0, 0,
	0, 0, 48, 88, 88, 16, 0, 0,
	0, 0, 126, 9, 1, 2, 0, 0,
	0, 0, 80, 152, 152, 112, 0, 0,
	0, 0, 127, 8, 8, 112, 0, 0,
	0, 0, 0, 122, 0, 0, 0, 0,
	0, 0, 64, 128, 128, 122, 0, 0,
	0, 0, 127, 16, 40, 72, 0, 0,
	0, 0, 0, 127, 0, 0, 0, 0,
	0, 120, 8, 16, 8, 112, 0, 0,
	0, 0, 120, 8, 8, 112, 0, 0,
	0, 0, 48, 72, 72, 48, 0, 0,
	0, 0, 248, 40, 40, 16, 0, 0,
	0, 0, 16, 40, 40, 248, 0, 0,
	0, 0, 112, 8, 8, 16, 0, 0,
	0, 0, 72, 84, 84, 36, 0, 0,
	0, 0, 8, 60, 72, 32, 0, 0,
	0, 0, 56, 64, 32, 120, 0, 0,
	0, 0, 56, 64, 56, 0, 0, 0,
	0, 56, 64, 32, 64, 56, 0, 0,
	0, 0, 72, 48, 48, 72, 0, 0,
	0, 0, 24, 160, 160, 120, 0, 0,
	0, 0, 100, 84, 84, 76, 0, 0,
	0, 0, 8, 28, 34, 65, 0, 0,
	0, 0, 0, 126, 0, 0, 0, 0,
	0, 0, 65, 34, 28, 8, 0, 0,
	0, 0, 4, 2, 4, 2, 0, 0,
	0, 120, 68, 66, 68, 120, 0, 0,
};

void delay(int cyc) {
	int i;
	for(i = cyc; i > 0; i--);
}

uint8_t spi_send_recv(uint8_t data) {
	while(!(SPI2STAT & 0x08));
	SPI2BUF = data;
	while(!(SPI2STAT & 0x01));
	return SPI2BUF;
}

void display_init() {
	DISPLAY_COMMAND_DATA_PORT &= ~DISPLAY_COMMAND_DATA_MASK;
	delay(10);
	DISPLAY_VDD_PORT &= ~DISPLAY_VDD_MASK;
	delay(1000000);
	
	spi_send_recv(0xAE);
	DISPLAY_RESET_PORT &= ~DISPLAY_RESET_MASK;
	delay(10);
	DISPLAY_RESET_PORT |= DISPLAY_RESET_MASK;
	delay(10);
	
	spi_send_recv(0x8D);
	spi_send_recv(0x14);
	
	spi_send_recv(0xD9);
	spi_send_recv(0xF1);
	
	DISPLAY_VBATT_PORT &= ~DISPLAY_VBATT_MASK;
	delay(10000000);
	
	spi_send_recv(0xA1);
	spi_send_recv(0xC8);
	
	spi_send_recv(0xDA);
	spi_send_recv(0x20);
	
	spi_send_recv(0xAF);


	/* Own code starts */

	//initiate the green leds
	volatile int * trise = (volatile int *) 0xbf886100;
	*trise = *trise & 0xfff0;	
 	//initiate timer
	T2CON = 0x70; //prescale 256. 
  	PR2 = TMR2PERIOD; //period register bit 0-15 (=16 least significant bites)
  	T2CONSET = 0x8000; //Start the timer
	/* Own code ends*/
}

void display_string(int line, char *s, int j) {
	int i;
	if(line < 0 || line >= 4)
		return;
	if(!s)
		return;
	
	for(i = j; i < 16; i++)
		if(*s) {
			textbuffer[line][i] = *s;
			s++;
		} 
		else{
			textbuffer[line][i] = ' ';
		}
}

void display_update() {
	int i, j, k;
	int c;
	for(i = 0; i < 4; i++) {
		DISPLAY_COMMAND_DATA_PORT &= ~DISPLAY_COMMAND_DATA_MASK;
		spi_send_recv(0x22);
		spi_send_recv(i);
		
		spi_send_recv(0x0);
		spi_send_recv(0x10);
		
		DISPLAY_COMMAND_DATA_PORT |= DISPLAY_COMMAND_DATA_MASK;
		
		for(j = 0; j < 16; j++) {
			c = textbuffer[i][j];
			if(c & 0x80)
				continue;
			
			for(k = 0; k < 8; k++)
				spi_send_recv(font[c*8 + k]);
		}
	}
}

/* Wait for I2C bus to become idle */
void i2c_idle() {
	while(I2C1CON & 0x1F || I2C1STAT & (1 << 14)); //TRSTAT
}

/* Send one byte on I2C bus, return ack/nack status of transaction */
bool i2c_send(uint8_t data) {
	i2c_idle();
	I2C1TRN = data;
	i2c_idle();
	return !(I2C1STAT & (1 << 15)); //ACKSTAT
}

/* Receive one byte from I2C bus */
uint8_t i2c_recv() {
	i2c_idle();
	I2C1CONSET = 1 << 3; //RCEN = 1
	i2c_idle();
	I2C1STATCLR = 1 << 6; //I2COV = 0
	return I2C1RCV;
}

/* Send acknowledge conditon on the bus */
void i2c_ack() {
	i2c_idle();
	I2C1CONCLR = 1 << 5; //ACKDT = 0
	I2C1CONSET = 1 << 4; //ACKEN = 1
}

/* Send not-acknowledge conditon on the bus */
void i2c_nack() {
	i2c_idle();
	I2C1CONSET = 1 << 5; //ACKDT = 1
	I2C1CONSET = 1 << 4; //ACKEN = 1
}

/* Send start conditon on the bus */
void i2c_start() {
	i2c_idle();
	I2C1CONSET = 1 << 0; //SEN
	i2c_idle();
}

/* Send restart conditon on the bus */
void i2c_restart() {
	i2c_idle();
	I2C1CONSET = 1 << 1; //RSEN
	i2c_idle();
}

/* Send stop conditon on the bus */
void i2c_stop() {
	i2c_idle();
	I2C1CONSET = 1 << 2; //PEN
	i2c_idle();
}

/* Convert 8.8 bit fixed point to string representation*/
char *fixed_to_string(uint16_t num, char *buf) {
	bool neg = false;
	uint32_t n;
	char *tmp;
	
	//2-talskomplement
	if(num & 0x8000) {
		num = ~num + 1;
		neg = true;
	}
	
	buf += 4;
	n = num >> 8;

	/* Own code starts here */

	//Kontrollera switcharna
	int checksw = getsw();
	switchcheck(checksw);

	//Fahrenheit conversion
	if(unit==0x20){
		n=n*9/5 + unit; //konvertering från C till F
	}
	else{
		n+=unit;
	}

	int checkbtn = getbtns();
	buttoncheck(checkbtn);


	/* Own code ends here*/
	


	tmp = buf;
	do {
		*--tmp = (n  % 10) + '0';
		n /= 10;
	} while(n);
	if(neg)
		*--tmp = '-';
	
	n = num;
	if(!(n & 0xFF)) {
		*buf = 0;
		return tmp;
	}
	*buf++ = '.';
	while((n &= 0xFF)) {
		n *= 10;
		*buf++ = (n >> 8) + '0';
	}
	*buf = 0;
	
	return tmp;
}

uint32_t strlen(char *str) {
	uint32_t n = 0;
	while(*str++)
		n++;
	return n;
}




int main(void) {
	uint16_t temp;
	char buf[32], *s, *t, *mte, *mti, *ti;

	/* Own code - Set up buttons */
	PORTD |= 0xfe0;
	/* Own code ends */

	/* Set up peripheral bus clock */
	OSCCON &= ~0x180000;
	OSCCON |= 0x080000;
	
	/* Set up output pins */
	AD1PCFG = 0xFFFF;
	ODCE = 0x0;
	TRISECLR = 0xFF;
	PORTE = 0x0;
	
	/* Output pins for display signals */
	PORTF = 0xFFFF;
	PORTG = (1 << 9);
	ODCF = 0x0;
	ODCG = 0x0;
	TRISFCLR = 0x70;
	TRISGCLR = 0x200;
	
	/* Set up input pins */
	TRISDSET = (1 << 8);
	TRISFSET = (1 << 1);
	
	/* Set up SPI as master */
	SPI2CON = 0;
	SPI2BRG = 4;
	
	/* Clear SPIROV*/
	SPI2STATCLR &= ~0x40;
	/* Set CKP = 1, MSTEN = 1; */
        SPI2CON |= 0x60;
	
	/* Turn on SPI */
	SPI2CONSET = 0x8000;
	
	/* Set up i2c */
	I2C1CON = 0x0;
	/* I2C Baud rate should be less than 400 kHz, is generated by dividing
	the 40 MHz peripheral bus clock down */
	I2C1BRG = 0x0C2;
	I2C1STAT = 0x0;
	I2C1CONSET = 1 << 13; //SIDL = 1
	I2C1CONSET = 1 << 15; // ON = 1
	temp = I2C1RCV; //Clear receive buffer
	
	/* Set up input pins */
	TRISDSET = (1 << 8);
	TRISFSET = (1 << 1);
	
	
	/* Own code begins here */
	display_init();
	display_string(0, "Temp:", 0);
	
	mte = fixed_to_string(maxtemp, buf);
	display_string(1, "Maxtemp: ", 0);
	display_string(1, mte, 10);
	
	display_string(2, "Time: ", 0);
	
	display_string(3, "Maxtime: ", 0);
	
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
	

	for(;;) {
		/* Send start condition and address of the temperature sensor with
		write flag (lowest bit = 0) until the temperature sensor sends
		acknowledge condition */
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
		display_string(0, s, 5);
		/* Temperatursutskrift slutar */

		/* Maxtemperatursrelevans börjar*/
		mte = fixed_to_string(maxtemp, buf);
		t = mte + strlen(mte);
		*t++ = ' ';
		*t++ = tempunit;
		*t++ = 0;
		if(tempcheck(temp)){
			//Skriv Warning till skärmen.
			display_string(1, "WARNING  WARNING ", 0);
			*porte |= 0x55; //Täänd varannan LED, från position 0
		}
		else{
			display_string(1, "Maxtemp: ", 0);
			display_string(1, mte, 10);
			*porte &= ~0x55; //Invertera de tända LEDsen
		}
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
    			mytime += 0x100;
    			timeoutcount = 0;
    		}
		}
	
		/* Tidsutskrift slutar */

		/* Maxtidsutskrift börjar */
		mti = fixed_to_string(maxtime, buf);
		t = mti + strlen(mti);
		*t++ = ' ';
		//*t++ = tempunit;
		*t++ = 0;
		if (timecheck(mytime)){
			display_string(3, "TIME OVER", 0);
			*porte |= 0xAA; //Tänd de leds som inte tänds för det andra alarmet
		}
		else{
			display_string(3, "Maxtime: ", 0);
			//display_string(3, nått, 8);
			display_string(3, mti, 10);
			*porte &= ~0xAA; //Släck
		}
		
		/* Maxtidsutskrift slutar*/

		display_update();
		delay(1000000);
	}
	
	return 0;
}

