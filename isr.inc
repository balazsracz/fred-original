;*******************************************************************
; FRED/ISR.ASM
;
; Copyright: Stefan Bormann (s.bormann@tu-bs.de)
;            Martin Pischky (martin.pischky@fernuni-hagen.de)
;
; SEE COPYRGHT.TXT FOR COPYRIGHT INFO!!!
;
; This file holds the interrupt service routine and three macro
; definitions to modify the INTCON register
;*******************************************************************

; GIE =1 global interrupts on
; EEIE=0 EE write complete interrupt off
; T0IE=0 TMR0 overflow interrupt off
; T0IF=0 reset TMR0 overflow flag
; INTF=0 reset INT interrupt flag
; RBIF=0 reset Port B interrupt on change flag

INTCON_LOCONET	MACRO			
		 movlw	B'10010000'	; INTE=1 ENABLE CS INTERRUPT PIN
		 movwf	INTCON		; RBIE=0 DISABLE PORT B INTERRUPT ON CHANGE
		ENDM			
		
INTCON_ADC	MACRO			
		 movlw	B'10001000'	; INTE=0 DISABLE CS INTERRUPT PIN
		 movwf	INTCON		; RBIE=1 ENABLE PORT B INTERRUPT ON CHANGE
		ENDM

INTCON_NONE	MACRO
		 movlw	B'10000000'	; INTE=0 DISABLE CS INTERRUPT PIN
		 movwf	INTCON		; RBIE=0 DISABLE PORT B INTERRUPT ON CHANGE
		ENDM


; *****************************        
; * Interrupt Service Routine *
; *****************************        

ISR_SAVE_REGISTERS	macro ;save status and w register  
		         movwf   int_w_temp
		         swapf   STATUS, W
		         movwf   int_status_temp
			endm

ISR_RESTORE_REGISTERS	macro ;restore status and w register
		         swapf   int_status_temp, W
		         movwf   STATUS
		         swapf   int_w_temp, F
		         swapf   int_w_temp, W
			endm


ISR		MACRO
		 ISR_SAVE_REGISTERS		; 3/4/5	save W and STATUS
		 btfsc	INTCON, RBIF		; 6	int source is port B int on change?
		  goto	isr_adc			; 7	handle adc int
		 btfsc	INTCON, INTF		; 8	int source is interrupt pin?
		  goto	isr_loconet		; 9,10	 handle loconet int
	;	 SEND_DEBUG	debug_isr	; debug message
isr_freeze:	 goto 	isr_freeze		; freeze
	
isr_loconet:	 ISR_LN_RECV			; 9-...
		 goto	isr_end

isr_adc:	 ISR_ADC

isr_end:	 ISR_RESTORE_REGISTERS
	         retfie				; normal return from interrupt
		ENDM

