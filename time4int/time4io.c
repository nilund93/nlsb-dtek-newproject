#include <stdint.h>
#include <pic32mx.h>
#include "mipslab.h"

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
		Ska returnera de 3 msb från knapparna btn4, btn3 och btn2 från kortet.
		BTN2 är den msb.
		Alla andra bitar ska vara 0. (maska med 0x7?)
		Bitarna 5, 6 och 7 i PORTD är knapparna.
	*/
	int btns = PORTD >> 5;
	btns = btns & 0x7;
	return btns;
}