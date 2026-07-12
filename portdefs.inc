;*******************************************************************
; PORTDEFS.INC
;
; Copyright: Stefan Bormann (s.bormann@tu-bs.de)
;            Martin Pischky (martin.pischky@fernuni-hagen.de)
;
; SEE COPYRGHT.TXT FOR COPYRIGHT INFO!!!
;
; History: 971205pi  Initial version
;          980214sb  Added INIT_PORTDEFS macro
;          021129sh  Added oprtdefs for f3_switch
;          030411pi  Version 1.7: use FOURKEYS - code for FD is conditional include
;
; This file contains the definition of the IO pins
; and the macro to initialize port and tris registers
;*******************************************************************

	; definition of i/o-pins ------------------------------------------------

	#define 	green_led 	PORTA,0
	#define		red_led		PORTA,1
        #ifdef FOURKEYS
	  #define	f3_switch	PORTA,2
        #endif
        #ifndef FOURKEYS
	  #define	nc_1		PORTA,2
        #endif
	#define		dir_switch	PORTA,3
	#define		adc_discharge	PORTA,4

	#define		loconet_cs	PORTB,0
	#define		f0_switch	PORTB,1
	#define		f1_switch	PORTB,2
	#define		loconet_rx	PORTB,3
	#define		adc_monitor	PORTB,4
	#define		loconet_tx	PORTB,5
	#define		f2_switch	PORTB,6
	#define		estop_switch	PORTB,7

	; initialization -------------------------------------------------------

INIT_PORTDEFS	MACRO			; PORTA[0]=0	green_led off
		 clrf	PORTA		; PORTA[1]=0	red_led off
					; PORTA[2]=0	nc_1 off
					; PORTA[3]=0	dir switch is input - don't care
					; PORTA[4]=0	adc_discharge - discharge C
					; PORTA[7:5]=000  not existent - don't care
					
					; PORTB[4:0]=00000	inputs - don't care
		 clrf	PORTB		; PORTB[5]=0	loconet_tx - idle means off
					; PORTB[7:6]=0	inputs - don't care
		 bank_1
		 #ifdef FOURKEYS
		  movlw	B'11101100'	; TRISA[0]=0	green_led is output
		  movwf	TRISA		; TRISA[1]=0	red_led is output
					; TRISA[2]=1	f3_switch is input
					; TRISA[3]=1	dir_switch is input
					; TRISA[4]=0	adc_discharge is output
					; TRISA[7:5]=1	non existant - handle as inputs
		 #endif
                 #ifndef FOURKEYS
		  movlw	B'11101000'	; TRISA[0]=0	green_led is output
		  movwf	TRISA		; TRISA[1]=0	red_led is output
					; TRISA[2]=0	nc_1 is debug output
					; TRISA[3]=1	dir_switch is input
					; TRISA[4]=0	adc_discharge is output
					; TRISA[7:5]=1	non existant - handle as inputs
                 #endif			
		 movlw	B'11011111'	; TRISB[0]=1	loconet_cs is input
		 movwf	TRISB		; TRISB[1]=1	f0_switch is input
					; TRISB[2]=1	f1_switch is input
		 bank_0			; TRISB[3]=1	loconet_rx is input
		ENDM			; TRISB[4]=1	adc_monitor is input
					; TRISB[5]=0	loconet_tx is output
					; TRISB[6]=1	f2_switch is input
					; TRISB[7]=1	estop_switch is input

;end-of-file: dcc/slnthr/portdefs.inc ******************************
