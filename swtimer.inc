;******************************************************************************
; FRED\SWTIMER.INC
;
; Copyright: Martin Pischky (martin.pischky@fernuni-hagen.de)
;            Stefan Bormann (stefan.bormann@gmx.de)
;
; SEE COPYRGHT.TXT FOR COPYRIGHT INFO!!!
; 
; History:
;    971219pi initial version
;    971223pi redesign
;    971230sb added INIT_TIMER
;    980103pi bugfix (second try)
;    980217sb redesign (made it simpler but more efficient)
;    000305sb bug-fix: no more overflow with 'ms' define for large numbers,
;             reordered calculations with '()'
;
; PRECOND:
;	OPTION_REG,PS2:PS0 = B'100'
;	OPTION_REG,PSA     = 0	
;       OPTION_REG,T0CS    = 0
;       INTCON, T0IE       = 0
;       OSC_CLK	   	   = frequency-of-xtal-used
;
; This file supplies some macros that make it easier and more readable
; to use a software timer. The idea is to use TMR0 overflows without interrupt.
; An application specific routine 'check_timer' has to be declared like
; explained further below. 
; Each timer needs one general perpose register to hold its timer value.
; The value 0 means, the  timer is off. Each time, check_timer is called, 
; it checks for for the TMR0 overflow bit and exits immediately if it is 
; not set. Otherwise it decrements non null timer values and executes
; the timer code if the timer value was decremented to zero.
;******************************************************************************


tmr0_prescaler 		EQU	D'32'		; 8 ms between timer overflows (for 4MHz Xtal)
software_prescaler	EQU	(4 * tmr0_prescaler * H'100')
#define			ms 	* (OSC_CLK / D'1000') / software_prescaler	; milliseconds


;************************************************************
; Activates a Timer for a given time out.
; Example:
;
;   TIMER_START my_timer, 42 ms
;
;************************************************************

TIMER_START		MACRO	timer_var, value
			 movlw	value
			 movwf	timer_var
			ENDM


;************************************************************
; Activates a Timer with the smalest possible time out.
; Example:
; 
;   TIMER_FORCE_FIRE my_timer
;
;************************************************************

TIMER_FORCE_FIRE	MACRO	timer_var
			 TIMER_START timer_var, 1
			ENDM


;************************************************************
; Deactivates a Timer.
; Example:
;
;   TIMER_FREEZE my_timer
;
;************************************************************

TIMER_FREEZE		MACRO	timer_var
			 clrf	timer_var
			ENDM


;************************************************************
; The following three macros are ment to build up a timer
; routine. 
; Example:
;
;   check_timer:	TIMER_CHECK_START
;			TIMER_CHECK_VAR my_first_timer
;			; Place here code to be executed
;			; if first timer times out
;			TIMER_CHECK_VAR my_second_timer
;			; Place here code to be executed
;			; if second timer times out
;			TIMER_CHECK_END
;
;************************************************************

timer_cont_cntr	= 0

TIMER_CHECK_START	MACRO
			 btfss	INTCON, T0IF	;check TMR0 overflow interrupt flag
			 return			;  if flag is 0 we are ready
			 bcf	INTCON, T0IF	;reset TMR0 overflow interrupt flag
			ENDM


TIMER_CHECK_VAR		MACRO timer_var
timer_continue_#v(timer_cont_cntr)	; label to jump to from previous instanciation of this macro
timer_cont_cntr++			; increment count of instantations
			 tstf	timer_var
			 bz	timer_continue_#v(timer_cont_cntr)
			 decfsz	timer_var,F
			 goto	timer_continue_#v(timer_cont_cntr)
			ENDM
		
		
TIMER_CHECK_END		MACRO
timer_continue_#v(timer_cont_cntr)	; label to jump to from last instantiation of TIMER_CHECK_VAR
			 return		; return from routine 'check_timer'
			ENDM



; ---- end-of-file swtimer.inc ----

