;*******************************************************************
;  dcc/slnthr/macros.inc
;
; Authors: Martin Pischky
;          Stefan Bormann
;
; History: 971205pi initial version
;          980102sb modified ee_read
;          980103pi eeprom handling moved to eeprom.inc
;*******************************************************************


bank_0	MACRO
	bcf	STATUS,RP0
	ENDM

bank_1	MACRO
	bsf	STATUS,RP0
	ENDM

;-----------------------------------------------------------------------------

FALSE	EQU	0
TRUE	EQU	! FALSE

delay7	=	FALSE
delay6	=	FALSE
delay5	=	FALSE
delay4	=	FALSE
delay3	=	FALSE
delay2	= 	FALSE
delay1	=	FALSE
delay0	=	FALSE


DELAY		macro cycles
	if cycles==0
	endif
	if cycles==1
		nop
	endif
	if cycles==2
		goto $+1
	endif
	if cycles==3
		nop
		goto $+1
	endif
	if (cycles>3) && (cycles<8)
delay#v(cycles) =	TRUE
		call	delay_#v(cycles)
	endif
	if cycles>7
		local	loops
		local	offset
loops		= ((cycles)/4) - 1
offset		= (cycles) - ((loops+1)*4)
		movlw	loops
delay#v(offset) =	TRUE
		call	delay_#v(offset)
	endif
		endm


P_DELAY		macro
		if	delay3 
delay_3:		nop
		endif
		if	delay2 || delay3
delay_2:		nop
		endif
		if	delay1 || delay2 || delay3
delay_1:		nop
		endif
		if 	delay0 || delay1 || delay2 || delay3
delay_0:		addlw	0xff
			skpz
			goto delay_0
			return
		endif

		if 	delay7
delay_7:		nop
		endif
		if 	delay7 || delay6
delay_6:		nop	
		endif
		if 	delay7 || delay6 || delay5
delay_5:		nop
		endif
		if 	delay7 || delay6 || delay5 || delay4
delay_4:		return
		endif
		endm		


;end-of-file: dcc/slnthr/macros.inc ********************************

