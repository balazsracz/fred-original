;*******************************************************************
; FRED\CONNECT.INC
;
; Copyright: Stefan Bormann (s.bormann@tu-bs.de)
;            Martin Pischky (martin.pischky@fernuni-hagen.de)
;
; SEE COPYRGHT.TXT FOR COPYRIGHT INFO!!!
;
; History: 980303sb  Created
;
; This is the reconnect timer of the SLNTHR/FRED project
;*******************************************************************

	cblock
		reconnect_timer   : 1	; timer to wait between reconnect attempts
	endc
	
reconnect_time	equ	500 ms		; half a second between reconnect attempts
	
	
INIT_CONNECT	MACRO
		 TIMER_FREEZE reconnect_timer
		ENDM
		
		
TIMER_CONNECT	MACRO
		bsf	flag_reconnect
		ENDM
		

P_CONNECT	MACRO
		

		ENDM
