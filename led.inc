;*******************************************************************************
;
; dcc/slnthr/led.inc [971219pi]
; 
; Copyright: Stefan Bormann (s.bormann@tu-bs.de)
;            Martin Pischky (martin.pischky@fernuni-hagen.de)
;
; history:
;    971223pi initial version
;    980125sb prepared for led_update
;    980130sb inserted led_update
;    980215sb FRED-Version
;
;*******************************************************************************
;
; VARS:

	cblock
		leds		: 1 ; array [0..3] of record red,green:boolean; end;
				    ; write into this byte the led mode
				    ; use equs led_* below
		led_timer	: 1 ; timer value for counting time between
				    ; led updates
	endc

;
; PORTS:
;       green_led       ;define as output
;       red_led         ;define as output
;
; PRECOND:
;	#include "swtimer.inc"
;       OSC_CLK	   	   = frequency-of-xtal-used
;
; INIT:
;	led_timer 	   := 1
;

; ------ definition of some usefull led modes ------

;			   phase: 33221100
;		             led: grgrgrgr

led_off			EQU	B'00000000'
led_red			EQU	B'01010101'
led_green		EQU	B'10101010'
led_yellow		EQU	B'11111111'
led_slowblink_red	EQU	B'01010000'
led_fastblink_red	EQU	B'01000100'
led_slowblink_green	EQU	B'10100000'
led_fastblink_green	EQU	B'10001000'
led_slowblink_yellow	EQU	B'11110000'
led_fastblink_yellow	EQU	B'11001100'
led_blink_yellow_red	EQU     B'11011101'
led_shortblink_red	EQU	B'00000001'
led_shortblink_green	EQU	B'00000010'
;				etc

off	equ	0
red	equ	1
green	equ	2
yellow	equ	3


#define	phase0(color)	(color*1)
#define	phase1(color)	(color*4)
#define	phase2(color)	(color*16)
#define	phase3(color)	(color*64)

#define steady(color)			( phase0(color)  + phase1(color)  + phase2(color)  + phase3(color)  )
#define slowflash(color1, color2)	( phase0(color1) + phase1(color1) + phase2(color2) + phase3(color2) )
#define fastflash(color1, color2)	( phase0(color1) + phase1(color2) + phase2(color1) + phase3(color2) )

led_notassigned		equ	steady(red)
led_estop		equ	slowflash(green, off)
led_upandrunning	equ	steady(green)

led_connect		equ	fastflash(yellow, off)

led_unrecoverable_error	equ	slowflash(red, off)
	

led_fastblink_red_green	EQU	fastflash(red, green)

; ----- definition of the duration of one led phase ------ 

led_timer_value		EQU	D'100' ms

; **************************************************
; * timer event handler - update led if req.       *
; **************************************************
;

TIMER_LED	MACRO
		; ----- change the led outputs

		bcf	red_led		;copy bit 0 of leds to port "red_led"

		TIMER_START	led_timer, led_timer_value

		bcf	green_led	;copy bit 1 of ldes to port "green_led"

		; prepare next led phase
		rlf	leds,W		;rotate left "leds" [grrr]
		rlf	leds,F
		rlf	leds,W		;rotate left "leds"
		rlf	leds,F

		; copy current led phase to port
		btfsc	leds, 0
		bsf	red_led
		btfsc	leds, 1
		bsf	green_led

		btfss	flag_update_led
		goto	timer_led_end
		
		call	do_led_update
		movwf	leds
		
		bcf	flag_update_led
timer_led_end:		
		ENDM
		

P_LED		MACRO

do_led_update:	tstf	loco_slot
		skpnz				; no loco assigned to throttle?
		retlw	led_notassigned
		movfw	loco_spd
		xorlw	SPD_ESTOP
		skpnz				; speed is estop?
		retlw	led_estop
		retlw	led_upandrunning	; if all answers are 'no',
						;  we are up and running.

		ENDM


; ***************************************************************
; * include this macro after the "org 0" and before CHECK_TIMER *
; * (but before the "END")				        *
; ***************************************************************

INIT_LED	MACRO
		movlw	led_fastblink_red_green
		movwf	leds		; init led mode with weird flashing to demonstrate operation
		TIMER_FORCE_FIRE led_timer
		ENDM

; ---- end-of-file led.inc ----

