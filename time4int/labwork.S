  # labwork.S
  # Written 2015 by F Lundevall
  # Skeleton file for IS1200/IS1500 lab 1 2015-2016.
  # The macros PUSH and POP are in the public domain.
  # edited by Niclas Lund & Sofie Borck Janeheim
.macro	PUSH reg
	addi	$sp,$sp,-4
	sw	\reg,0($sp)
.endm

.macro	POP reg
	lw	\reg,0($sp)
	addi	$sp,$sp,4
.endm
  # You can add your own code here
  #
.global hexasc, delay, time2string, enable_interrupt
.data
.text
hexasc:
	#one register in $a0 with the 4 lsb
	#will return in $v0 the 7 LSB, all other bits must be 0.
	PUSH	$s0
	PUSH	$s1
	PUSH	$ra
	andi	$t0, $a0, 0xF 		#maska fram 4 lsb
	
	ble	$t0, 0x09, numbers 	#iom bokst�ver och siffror �r p� olika delar av ASCII-tabellen
	nop			   	#s� g�r vi en till subrutin att g� till f�r att l�sa det
	
	addi	$t0, $t0, 0x37		#l�gg till 0x37 (allts� 55 dec) f�r att komma till de
					#bokst�verna i ascii-tabellen
	andi	$t0, $t0, 0x7f		#maska fram 7 lsb
	move	$v0, $t0		#flytta v�rdet i t0 till v0
	POP	$ra
	POP	$s1
	POP	$s0
	jr	$ra			#�terg� till main
	nop
numbers:

	addi	$t0, $t0, 0x30		#l�gg till 0x30 (48 dec) f�r att komma till de numeriska
					#v�rdena i ascii-tabellen
	andi	$t0, $t0, 0x7f		#maska fram 7 lsb
	move	$v0, $t0		#flytta v�rdet i t0 till v0
	POP	$ra
	POP	$s1
	POP	$s0

	jr	$ra			#�terg� till main
	nop
	
delay:				#Uppgift 4
	li	$a0, 3000	#s�tt $t0 = ms = 1000
	bltz 	$a0, done	#kolla om ms �r >0
	nop
while:	
	li	$t1, 0		#int i;
for:
	bgt	$t1, 3800, forend	#Konstant som skall �ndras.
	nop
	addi	$t1, $t1, 1	#i++
	j	for
	nop
forend:
	addi	$a0, $a0, -1
	bgtz	$a0, while	#om ms > 0, skicka tillbaka till while.
	nop
done:
	jr	$ra
	nop 


time2string:			#Uppgift 3
	#sb data, imm(destination)
	PUSH	$ra
	PUSH	$s0
	PUSH	$s1
	move	$s0, $a0	#spara minnesadressen fr�n a0 i s0
	move	$s1, $a1	#spara timestampen fr�n a1 i s1
	
	srl	$a0, $a1, 12	#shifta h�ger 12 bitar f�r att f� minut tiotal
	jal	hexasc
	nop		
	sb	$v0, 0($s0)	#lagrar hexasc returnv�rde i s0.
	
	srl	$a0, $s1, 8	#shifta h�ger 8 bitar f�r att f� minut ental
	jal	hexasc
	nop		
	sb	$v0, 1($s0)	
	
	addi	$t1, $zero, 0x3A	
	sb	$t1, 2($s0)	#lagra v�rdet f�r :

	srl	$a0, $s1, 4	#shifta h�ger 4 bitar f�r att f� sekund tiotal 
	jal	hexasc
	nop
	sb	$v0, 3($s0)
	
	move	$a0, $s1	#tar fram sekund ental
	jal	hexasc
	nop	
	sb	$v0, 4($s0)
	sb	$zero, 5($s0)
	POP	$s1
	POP	$s0		
	POP	$ra
	jr	$ra
	nop
enable_interrupt:
	ei
	jr	$ra
	nop