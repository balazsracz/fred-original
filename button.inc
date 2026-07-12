;*****************************************************************************
; fred/button.inc
;
; Author: Stefan Bormann (s.bormann@tu-bs.de)
;
; History: 971230sb initial version
;          980808sb complete redesign, added debouncing
;          021129sh F3 added, distinguish between dispatch FB and FC
;          030411pi Version 1.7: use FOURKEYS - code for FD is conditional include
;
; Contains macros to access ports of switches and buttons
; The procedure "get_button_events" is the heart of this module,
; it returns an event value in W. 
;*****************************************************************************

; Variable definition
; Bitfields in button states:
;  0..3	counter
;     4	last state of button
;     5	d.c.
;     6	d.c.
;     7	d.c.

button_state_mask_counter	equ	0x0F
button_state_mask_bits		equ	0xF0
button_state_bitpos_last	equ	6
button_state_bitpos_stabile	equ	7
button_state_init_counter	equ	5	; 1..15

	cblock
		button_state		: 1 ; state of the fsm (lsb of state address)
		
		button_table		: 0 ; table of states of all binary input
					    ; devices
	;	button_state_FA		: 1 ; state of first function button
	;	button_state_FB		: 1 ; state of second function button
	;	button_state_FC 	: 1 ; state of third function button
	;	button_state_ESTOP	: 1 ; state of emergency stop button
	;	button_state_DIR	: 1 ; state of direction button
	endc

; Event values, stored as return value in W
event_none	equ	0	; no event
event_p_FA	equ	1	; Fa pressed
event_r_FA	equ	2	; Fa released
event_p_FB	equ	3	; Fb pressed
event_r_FB	equ	4	; Fb released
event_p_FC	equ	5	; Fc pressed
event_r_FC	equ	6	; Fc released
event_p_ESTOP	equ	7	; ESTOP pressed
event_r_ESTOP	equ	8	; ESTOP released
event_f_DIR	equ	9	; dir switch to forward
event_r_DIR	equ    10	; dir switch to reverse
event_dispatch	equ    11	; Dispatch combination pressed
#ifdef FOURKEYS
event_p_FD	equ    12	; Fd pressed
event_r_FD	equ    13	; Fd released
#endif

; bit positions of "button_state"
button_FA		equ	0	; state of f0 button
button_FB		equ	1	; state of f1 button
button_FC		equ	2	; state of f2 button
button_ESTOP		equ	3	; state of estop button
button_DIR		equ	4	; state of dir switch
#ifdef FOURKEYS
button_FD		equ	5	; state of f3 button
#endif
button_waitidle		equ	7	; 1=wait for idle				



; Macros to access ports for switch and buttons

SKPFA		macro
		 btfsc	f0_switch
		endm

SKPNFA		macro
		 btfss	f0_switch
		endm

SKPFB		macro
		 btfsc	f1_switch
		endm

SKPNFB		macro
		 btfss	f1_switch
		endm

SKPFC		macro
		 btfsc	f2_switch
		endm

SKPNFC		macro
		 btfss	f2_switch
		endm

#ifdef FOURKEYS

SKPFD		macro
		 btfsc	f3_switch
		endm

SKPNFD		macro
		 btfss	f3_switch
		endm

#endif

SKPDIR		macro
		 btfsc	dir_switch
		endm

SKPNDIR		macro
		 btfss	dir_switch
		endm
	
SKPESTOP	macro
		 btfsc	estop_switch
		endm

SKPNESTOP	macro
		 btfss	estop_switch
		endm

; Macro for macros :-)
		
SKPNF		macro
		 call	checkfbuttons
		 skpnc
		endm
		
SKPF		macro
		 call	checkfbuttons
		 skpc
		endm


; Macro to initialize module
	
INIT_BUTTON	macro				; pins initialized in portdefs.inc
		 clrf	button_state
		 bsf	button_state, button_waitidle
		 SKPNDIR
		 bsf	button_state, button_DIR
		endm

; Macro used in "get_button_event"

button_count=0

BUTTON_HANDLE	macro	name
button_count++

button_handle_endif_#v(button_count)
		endm


; Procedures

P_BUTTON	MACRO

; used by macros SKPF and SKPNF
checkfbuttons:	clrc
		SKPNFA
		setc
		SKPNFB
		setc
		SKPNFC
		setc
		#ifdef FOURKEYS
		  SKPNFD
		  setc
                #endif
		return


button_update_dir:
		bcf	loco_dirf, SL_DIR
		bcf	button_state, button_DIR
		SKPDIR
		return
		bsf	loco_dirf, SL_DIR
		bsf	button_state, button_DIR
		return

		
button_handle_dispatchA:
		clrf	functionconfig
		bsf	button_state, button_waitidle
		retlw	event_dispatch

button_handle_dispatchB:
		movlw	0x03
		movwf	functionconfig
		bsf	button_state, button_waitidle
		retlw	event_dispatch

button_handle_dispatchC
		clrf	functionconfig
		incf	functionconfig,F
		bsf	button_state, button_waitidle
		retlw	event_dispatch
		

; Task to keep track of button states and report transitions
get_button_event:
		btfsc	button_state, button_waitidle
		goto	t_button_waitidle


DIR_if:		btfss	button_state, button_DIR		; if remembered state is pressed
		 goto	DIR_else
DIR_than:	SKPNDIR					; if still pressed
		 goto	DIR_endif			;  than skip to end of macro
		bcf	button_state, button_DIR		; remember released state of button
		retlw	event_r_DIR			; return button-released-event
DIR_else:	SKPDIR					; if still not pressed
		 goto	DIR_endif			;  than skip to end of macro
		bsf	button_state, button_DIR		; remember state of button
		retlw	event_f_DIR
DIR_endif:

ESTOP_if:	btfss	button_state, button_ESTOP		; if remembered state is pressed
		 goto	ESTOP_else
ESTOP_than:	SKPNESTOP					; if still pressed
		 goto	ESTOP_endif			;  than skip to end of macro
		bcf	button_state, button_ESTOP		; remember released state of button
		retlw	event_r_ESTOP			; return button-released-event
ESTOP_else:	SKPESTOP					; if still not pressed
		 goto	ESTOP_endif			;  than skip to end of macro
		bsf	button_state, button_ESTOP		; remember state of button
		retlw	event_p_ESTOP
ESTOP_endif:

FA_if:		btfss	button_state, button_FA		; if remembered state is pressed
		 goto	FA_else
FA_than:	SKPNFA					; if still pressed
		 goto	FA_endif			;  than skip to end of macro
		bcf	button_state, button_FA		; remember released state of button
		retlw	event_r_FA			; return button-released-event
FA_else:	SKPFA					; if still not pressed
		 goto	FA_endif			;  than skip to end of macro
		btfsc	button_state, button_ESTOP	; if estop is pressed and we are an F-button
		 goto	button_handle_dispatchA		;  than we haven encountered an dispatch combination
		bsf	button_state, button_FA		; remember state of button
		retlw	event_p_FA
FA_endif:

FB_if:		btfss	button_state, button_FB		; if remembered state is pressed
		 goto	FB_else
FB_than:	SKPNFB					; if still pressed
		 goto	FB_endif			;  than skip to end of macro
		bcf	button_state, button_FB		; remember released state of button
		retlw	event_r_FB			; return button-released-event
FB_else:	SKPFB					; if still not pressed
		 goto	FB_endif			;  than skip to end of macro
		btfsc	button_state, button_ESTOP	; if estop is pressed and we are an F-button
		 goto	button_handle_dispatchB		;  than we haven encountered an dispatch combination
		bsf	button_state, button_FB		; remember state of button
		retlw	event_p_FB
FB_endif:

FC_if:		btfss	button_state, button_FC		; if remembered state is pressed
		 goto	FC_else
FC_than:	SKPNFC					; if still pressed
		 goto	FC_endif			;  than skip to end of macro
		bcf	button_state, button_FC		; remember released state of button
		retlw	event_r_FC			; return button-released-event
FC_else:	SKPFC					; if still not pressed
		 goto	FC_endif			;  than skip to end of macro
		btfsc	button_state, button_ESTOP	; if estop is pressed and we are an F-button
		 goto	button_handle_dispatchC		;  than we haven encountered an dispatch combination
		bsf	button_state, button_FC		; remember state of button
		retlw	event_p_FC
FC_endif:

#ifdef FOURKEYS

FD_if:		btfss	button_state, button_FD		; if remembered state is pressed
		 goto	FD_else
FD_than:	SKPNFD					; if still pressed
		 goto	FD_endif			;  than skip to end of macro
		bcf	button_state, button_FD		; remember released state of button
		retlw	event_r_FD			; return button-released-event
FD_else:	SKPFD					; if still not pressed
		 goto	FD_endif			;  than skip to end of macro
		bsf	button_state, button_FD		; remember state of button
		retlw	event_p_FD
FD_endif:

#endif

		retlw	event_none
		

t_button_waitidle:
		btfss	button_state, button_FA
		goto	t_button_waitidle_no_FA
		bcf	button_state, button_FA
		retlw	event_r_FA
t_button_waitidle_no_FA:
		btfss	button_state, button_FB
		goto	t_button_waitidle_no_FB
		bcf	button_state, button_FB
		retlw	event_r_FB
t_button_waitidle_no_FB:
		btfss	button_state, button_FC
		goto	t_button_waitidle_no_FC
		bcf	button_state, button_FC
		retlw	event_r_FC
t_button_waitidle_no_FC:
		btfss	button_state, button_ESTOP
		goto	t_button_waitidle_no_ESTOP
		bcf	button_state, button_ESTOP
		retlw	event_r_ESTOP
t_button_waitidle_no_ESTOP:

		SKPNFA
		retlw	event_none
		SKPNFB
		retlw	event_none
		SKPNFC
		retlw	event_none
		SKPNESTOP
		retlw	event_none

		bcf	button_state, button_waitidle
		retlw	event_none

		ENDM
		
		
P_BUTTON_TESTRANGE macro

;----------- test range:

		movlw	button_table
		movwf	FSR

		SKPNFA
		call	button_compute_on
		SKPFA
		call	button_compute_off
		addlw	event_p_FA
		skpc
		goto	event_found



event_found
		
		
		

; button_compute_on and button_compute_off work out how to handle
; different button states
; Input:	FSR	pointer to button state
;		INDF	previous state
; Output:	Carry	0=new event was generated and wants to be delivered
;			1=no event generated 
;		W	1, if (button ist 'on')
;			0, if (button ist 'off')
;		FSR	incremented

button_compute_on:
		btfsc	INDF, button_state_bitpos_last
		 goto	button_compute_same
		bsf	INDF, button_state_bitpos_last
		goto	button_compute_different
		
button_compute_off:
		btfss	INDF, button_state_bitpos_last
		 goto	button_compute_same
		bsf	INDF, button_state_bitpos_last
button_compute_different:
		movfw	INDF
		andlw	button_state_mask_bits
		iorlw	button_state_init_counter
		movwf	INDF
		incf	FSR,F
		retlw	0



button_compute_same:
		movlw	button_state_mask_counter	; W:=state & mask_counter
		andwf	INDF,W
		addlw	-1				; W--
		skpnc					; if it was not 0:
		 decf	INDF,F	; update state counter; precondition: counter is in LSBs	
		
		movlw	0
		btfsc	INDF, button_state_bitpos_last
		movlw	1
		incf	FSR,F
		return						


		ENDM
