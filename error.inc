;*******************************************************************
; FRED\ERROR.ASM
;
; Copyright: Stefan Bormann (s.bormann@tu-bs.de)
;            Martin Pischky (martin.pischky@fernuni-hagen.de)
;
; SEE COPYRGHT.TXT FOR COPYRIGHT INFO!!!
;
; History:  980407sb  created
;
; This file contains the handling of unrecoverable errors.
;*******************************************************************


INIT_ERROR		MACRO
			 ee_clear ee_error_code			; we don't want deductions
			ENDM					; from stale error codes!


UNRECOVERABLE_ERROR	MACRO error_code
			 movlw error_code			; error code to W
			 goto handle_unrecoverable_error	; jump to error handler
			ENDM


P_ERROR	MACRO

handle_unrecoverable_error:
	bank_0				; switch to bank 0, no assumptions!
	ee_write ee_error_code		; write error code into eeprom
	movlw	led_unrecoverable_error	; flash red to indicate error situation
	movwf	leds			
error_loop:
	call	check_timer		; led flashing is timer driven
	goto	error_loop		; loop until infinity
	
	ENDM
