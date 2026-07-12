;******************************************************************************
; file:    dcc/slnthr/eeprom.inc
;
; Copyright: Martin Pischky (martin.pischky@fernuni-hagen.de)
;            Stefan Bormann (s.bormann@tu-bs.de)
;
; History: 980103pi inital version (from macros.inc)
;                   ee_write added
;          980125sb moved "movwf EEADR" from ee_* to C_EE_*
;                   added loop in C_EE_WRITE to ensure that previous write is
;                    finished before atempting a new write cycle
;                   added check if data to be written is really new
;                   added ee_write_var and ee_clear
;******************************************************************************

INLINE_EE_READ	EQU	FALSE	; FALSE = use call
INLINE_EE_WRITE EQU	FALSE	; FALSE = use call

ee_start	EQU	H'2100'

; use to define the eeprom-layout. 
; example: 	
;	org 	ee_start
;	id1:	DE 	123
;	id2:    DE	H'55'
; 		..
; This should be done before the "org 0"

; Variables:

	cblock
		ee_temp1 : 1
		ee_temp2 : 1	; variables to temporarily store
	endc			; the eeprom address and data while we are
				; waiting for the previous write cycle
				; finishes



;******************************************************************************
; read an eeprom cell into W register
; assumes 2100H <= eeprom_adr < 2140
; assumes that bank 0 is selected
;******************************************************************************

ee_read    MACRO	eeprom_adr
		movlw	eeprom_adr-ee_start
		call	p_ee_read
	   ENDM

;******************************************************************************
; write value in W register to eeprom
; assumes 2100h <= eeprom_adr < 2140h
; assumes that bank 0 is selected
;******************************************************************************

ee_write   MACRO	eeprom_adr
 		movwf	ee_temp1		; Value to write
		movlw	eeprom_adr-ee_start	; Adress to write
		call 	p_ee_write
	   ENDM
	   
;******************************************************************************
; clear value in W register to eeprom
; assumes 2100H <= eeprom_adr < 2140
; assumes that bank 0 is selected
;******************************************************************************

ee_clear   MACRO	eeprom_adr
 		clrf	ee_temp1		; Null to be written
		movlw	eeprom_adr-ee_start	; Adress to write
		call 	p_ee_write
	   ENDM
	   
;******************************************************************************
; write value in FILE register to eeprom
; assumes 2100H <= eeprom_adr < 2140
; assumes that bank 0 is selected
;******************************************************************************

ee_write_var	MACRO eeprom_adr, file_reg
			movfw	file_reg
 			movwf	ee_temp1		; Value to write
			movlw	eeprom_adr-ee_start	; Adress to write
			call 	p_ee_write
		ENDM
	   
;******************************************************************************
; set Zero-Flag if eeprom write is in progress
;******************************************************************************

EE_WRITE_PROGRS	MACRO	

		bank_1
		clrz
		btfsc	EECON1, WR
		setz
		bank_0
		
		ENDM

;******************************************************************************
; include this macro between the "org 0" and the "END")
;******************************************************************************
		
P_EEPROM   MACRO

p_ee_read:	movwf	EEADR		; Adress to read
		bank_1
		bsf	EECON1, RD 	; EE Read
		bank_0
		movfw	EEDATA		; EEPROM-Data is in W
		return



p_ee_write:	movwf	EEADR
		
		movfw	ee_temp1	; read new data into W
		bank_1
		bsf	EECON1, RD	; Read old data into EEDATA
		bank_0
		xorwf	EEDATA,	F	; compare EEDATA and W
		skpnz			; if equal than return
		return
		
		movwf	EEDATA		; EEDATA:=new data
		bank_1    	
		
		bcf	INTCON, GIE	; Disable Ints
		bsf	EECON1, WREN 	; enable write
		movlw	H'55'
		movwf	EECON2		; Write 55h
		movlw	H'AA'
		movwf	EECON2		; Write AAh
		bsf	EECON1, WR	; Set WR Bit - begin write
		bsf	INTCON, GIE	; Enable Ints
		bcf	EECON1, WREN	; disable write

ee_write_wait:	bank_0
		;call	check_timer	; keep things running (we are waiting
					;  longer than the timer overflow time)
		bank_1
		btfsc	EECON1, WR	; wait for write
		 goto	ee_write_wait	;  to finish
		bank_0
		return
	   ENDM

;end-of-file: dcc/slnthr/eeprom.inc *******************************************

