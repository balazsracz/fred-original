;*******************************************************************
; FRED\REFRESH.INC
;
; Copyright: Stefan Bormann (s.bormann@tu-bs.de)
;            Martin Pischky (martin.pischky@fernuni-hagen.de)
;
; SEE COPYRGHT.TXT FOR COPYRIGHT INFO!!!
;
; History:  98xxxxsb  Created
;
; This module contains the timer that reminds the chief of
; our throttle to avoid getting purged away
;*******************************************************************


	cblock
		refresh_low : 1	; timer variable low byte
		refresh_high: 1 ; timer variable high byte
	endc
	
	
refresh_timer_value	equ	1000 ms	; count slave timer in seconds


INIT_REFRESH	MACRO
		 TIMER_FREEZE	refresh_low	; don't fire!
		ENDM


START_REFRESH	MACRO
		 call	start_refresh_timer
		ENDM
		
STOP_REFRESH	MACRO
		 clrf	refresh_low
		ENDM


TIMER_REFRESH	MACRO
		TIMER_START	refresh_low, refresh_timer_value
		decfsz	refresh_high,F
		goto	timer_refresh_end
		call	ln_speedupdate
timer_refresh_end:
		ENDM


P_REFRESH	MACRO

ln_speedupdate:
		call	ln_send_loco_spd
		TIMER_START	adc_timer, 150 ms	; OWF: 200ms
start_refresh_timer:
		movlw	25				; 25s timer period
		movwf	refresh_high
		TIMER_START	refresh_low, refresh_timer_value
		return
	
		ENDM
