  #Uppgift 2
  # hexmain.asm
  # Written 2015-09-04 by F Lundevall
  # Copyright abandonded - this file is in the public domain.
  # edited by Niclas Lund & Sofie Borck Janeheim
	.text
main:
	li	$a0,5		# change this to test different values

	jal	hexasc		# call hexasc
	nop			# delay slot filler (just in case)	

	move	$a0,$v0		# copy return value to argument register

	li	$v0,11		# syscall with v0 = 11 will print out
	syscall			# one byte from a0 to the Run I/O window
	
stop:	j	stop		# stop after one run
	nop			# delay slot filler (just in case)

  # You can write your own code for hexasc here
  #
hexasc:	#från uppgift 2
	#one register in $a0 with the 4 lsb
	#will return in $v0 the 7 LSB, all other bits must be 0.
	andi	$t0, $a0, 0xF 		#maska fram 4 lsb
	
	ble	$t0, 0x09, numbers 	#iom bokst�ver och siffror �r p� olika delar av ASCII-tabellen
	nop			   	#s� g�r vi en till subrutin att g� till f�r att l�sa det
	
	addi	$t0, $t0, 0x37		#l�gg till 0x37 (allts� 55 dec) f�r att komma till de
					#bokst�verna i ascii-tabellen
	andi	$t0, $t0, 0x7f		#maska fram 7 lsb
	move	$v0, $t0		#flytta v�rdet i t0 till v0

	jr	$ra			#�terg� till main
	nop
numbers:
	addi	$t0, $t0, 0x30		#l�gg till 0x30 (48 dec) f�r att komma till de numeriska
					#v�rdena i ascii-tabellen
	andi	$t0, $t0, 0x7f		#maska fram 7 lsb
	move	$v0, $t0		#flytta v�rdet i t0 till v0

	jr	$ra			#�terg� till main
	nop
	
