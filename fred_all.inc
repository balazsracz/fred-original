;*******************************************************************
; FRED\FRED_ALL.ASM
;
; Copyright: Stefan Bormann (s.bormann@tu-bs.de)
;            Martin Pischky (martin.pischky@fernuni-hagen.de)
;
; SEE COPYRGHT.TXT FOR COPYRIGHT INFO!!!
;
; History:  9804xxsb  Split slnthr.asm into fred_c.asm, fred_f.asm
;                     and fred_all.inc
;           980815sb  Added ee_functionconfig
;
; This is the root include file for the SLNTHR/FRED project
;*******************************************************************

#ifndef MODE_TEST
#ifndef MODE_NORMAL
  error "Must be either MODE_TEST or MODE_NORMAL"
#endif
#endif

#ifdef MODE_TEST
#ifdef MODE_NORMAL
  error "May not be MODE_TEST *and* MODE_NORMAL"
#endif
#endif


; ********************************************************
; * RAM Variable storage above special-purpose registers *
; ********************************************************

        CBLOCK  H'0C'
		; data of loco ------------------------------------
		loco_spd	: 1 ; speed 0..127
		loco_dirf	: 1 ; direction&lower functions
		loco_snd	: 1 ; upper functions
		loco_slot	: 1 ; loco handle in chief (0 means not assigned)
		loco_stat1	: 1 ; stat1 of slot
		loco_stat2	: 1 ; stat2 of slot
		loco_adr_hi	: 1 ; high byte of address (0 if short)
		loco_adr_lo	: 1 ; low byte of address
		loco_id1	: 1 ; slot id1
		loco_id2	: 1 ; slot id2
		; other global stuff
		trk		: 1 ; byte is part of slot????!??
		temp		: 1 ; variable for anything anywhere outside the ISR
		flags		: 1 ; general purpose bit array
                ; values saved during interrupt -------------------
                int_w_temp 	: 1
                int_status_temp : 1
        ENDC

#define	flag_adc_invalid		flags, 0	; adc got interrupted by packet
#define	flag_update_led			flags, 1	; led mode "leds" has to be updated
#define flag_timer_overflow		flags, 2	; local flag in "led.inc"
#define flag_temp			flags, 3	; local temporary flag
#define flag_do_adc			flags, 4	; Message from TIMER_ADC to MAIN
#define flag_reconnect			flags, 5	; Message from TIMER_RECONNECT to MAIN
#define flag_request_speedupdate	flags, 6	; adc value changed -> do speed update
#define flag_7				flags, 7	; free


; ************************************************
; * Definition of error Codes for Debug Puropoes *
; ************************************************

;  #define debug_isr			1	; error code for entry of interrupt routine without legal int source
;  #define debug_dispatch_get_send	2	; could not send dispatch_get
;  #define debug_dispatch_get_no_slot	3	; did not receive valid read slot message after dispatch get
;  #define debug_reconnect_loco_adr	4	; could not send loco_adr
;  #define debug_reconnect_no_slot	5	; did not receive valid read slot message after loco_adr
;  #define debug_reconnect_wrong_id	6	; slot is taken by other throttle
;  #define debug_connect_not_written	7	; could not send wr_sl_data
;  #define debug_connect_no_lack	8	; did not receive long acknoledge after writing slot data
;  #define debug_reconnect_give_up	9	; give up reconnection after 3 attempts.
;  #define debug_wait_reply		10	; general receiption errors

; *********************
; * including modules *
; *********************

        include "macros.inc"
	include "eeprom.inc"
  ;	include "error.inc"
	include "isr.inc"
        include "portdefs.inc"
        include "function.inc"
        include "adc.inc"
	include "loconet.inc"
	include "swtimer.inc"
	include "led.inc"
	include "button.inc"
	include "refresh.inc"
	include "connect.inc"
#ifdef MODE_NORMAL
	include "main.inc"
#endif
#ifdef MODE_TEST
	include "test.inc"
#endif


; ***************************
; * EEPROM Variable storage *
; ***************************

#define adc_min_value		
#define adc_factor	

		org 	ee_start
;------------------- USER DATA -----------------------------------------------
ee_adr_lo	DE	0x00			; loco address low byte

ee_adr_hi	DE	0x00			; loco address high byte
ee_status1	DE	0x00			; remembered speed mode 
						; (3 lsb of status1)
;------------------- CONFIGURATION CONSTANTS ---------------------------------
ee_id1		DE	H'24'          		; throttle ID1
ee_id2		DE	H'42'			; throttle ID2 (must be 0x04..0x7E)
ee_adc_min_low	DE	D'80'			; offset for A/D conversion
ee_dummy1	DE	0			; was high byte of offset
ee_adc_factor   DE      D'144'			; scaling factor for A/D conversion
;------------------- ETC -----------------------------------------------------
ee_error_code	DE	0x00			; id of unrecoverable error
ee_functionconfig DE    0x00			; mapping buttons->functionbits
		FILL	H'FF',H'2140'-$ ; fill rest of eeprom (bug in mpasm?)

; ***********************************
; * ROM origin to begin of programm *
; ***********************************

        	org     0
        	bank_0
		clrf	flags
		clrf    loco_slot	; invalidate loco data ( loco_* )
        	goto    init


; *****************************        
; * Interrupt Service Routine *
; *****************************        

	        org     4		; entry point for (I)nterrupt (S)ervice (R)outine
		ISR	

; *************************************
; * Timer routine for software timers *
; *************************************


check_timer:	TIMER_CHECK_START
		TIMER_CHECK_VAR	led_timer
		 TIMER_LED
		TIMER_CHECK_VAR refresh_low
		 TIMER_REFRESH
		TIMER_CHECK_VAR adc_timer
		 TIMER_ADC
		TIMER_CHECK_VAR reconnect_timer
		 TIMER_CONNECT
		TIMER_CHECK_END


; *************************        
; * Global Initialization *
; *************************        

init:		INIT_PORTDEFS

		clrwdt			; reset WDT and
		clrf	TMR0		;   TMR0 before changing assignment
		bank_1			; *RBPU  =0 enable weak pull-up for PORTB
		movlw	B'00000100'	; INTEDG =0 cs interrupt on falling edge
		movwf	OPTION_REG	; T0CS   =0 TMR0 clock source: internal instr. cycle
					; T0SE   =0 don't care
					; PSA    =0 prescaler for TMR0
					; PS2:PS0=100 Prescaler rate 1:16 (WDT) or 1:32 (TMR0)
		

		bank_0
		
	;	INIT_ERROR
		INIT_ADC
		INIT_LED
		INIT_LOCONET
		INIT_BUTTON
	;	INIT_TIMER
		INIT_REFRESH
		INIT_CONNECT


; **********************        
; * Main Programm Area *
; **********************        


#ifdef MODE_NORMAL
main		MAIN
#endif
#ifdef MODE_TEST
main		TEST_MAIN
#endif


; ********************
; * subroutines used *
; ********************

	P_LN_RECV
	P_LN_SEND
	P_LN_SEND_LOCO_ADR
	P_LN_SEND_NULL_MOVE
	P_LN_SEND_LOCO_SPD
	P_LN_SEND_LOCO_DIRF
	P_LN_SEND_WR_SL_DATA
	P_EEPROM
	P_LED
	P_ADC
	P_BUTTON
;	P_SW_TIMER
	P_DELAY
	P_REFRESH
	P_CONNECT

	end

