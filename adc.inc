;*****************************************************************************
; FRED\ADC.INC
;
; Copyright: Martin Pischky (martin.pischky@fernuni-hagen.de)
;            Stefan Bormann (s.bormann@tu-bs.de)
;
; History: 9712XXpi  initial version
;          980201sb  inserted timer readjustment,
;                    inserted abort on Loconet trafic
;          980215sb  Created FRED-Version
;          980421sb  removed high byte of ee_adc_min
;
; This module does the a/d conversion of the poti resistance and calls
; 'ln_speedupdate' if apropriate.
;*****************************************************************************

	cblock
                ; vars for adc ------------------------------------
                adc_raw       : 0 ; value from adc:
                adc_raw_low   : 1 ;    lsb		
                adc_raw_high  : 1 ;    msb
                adc_8bit      : 1 ; saves last 8 bit value
                adc_timer	: 1
	endc


ISR_ADC		macro			; freeze timer for 32 cycles
		 btfsc	adc_monitor		; interrupt caused by adc-conversion?
		  bsf	flag_adc_invalid	; if not (ESTOP or FC pressed?) abort
		  
		 clrwdt				; reset prescaler
		 bank_1				; 
		 bcf	OPTION_REG, PSA		; switch prescaler to TMR0
		 bank_0				; 
		 
		 bcf	INTCON,RBIE		; disable interrupts
		 movfw	PORTB			; 	
		 bcf	INTCON,RBIF
		endm



INIT_ADC	macro		; only pins to initialize, but
				; that is done in portdefs.inc
		 TIMER_FORCE_FIRE   adc_timer
		endm


TIMER_ADC	MACRO
		bsf	flag_do_adc
		TIMER_START  adc_timer, 30 ms	; OWF: 42ms
		ENDM


P_ADC	macro

adc:		; saving timer >>>>>>
		incf	TMR0,W
		movwf	temp			; temp=TMR0+1
		; Warning: Assumption is that adc is called shortly after
		; timer overflow. If timer overflow occured while ad conversion,
		; the oveflow would be lost!
		;              <<<<<<
		
		bcf     adc_discharge   	; start discharge
		
        	;switch prescaler to WDT >>>>>
        	clrwdt				; clear timer and prescaler
        	bank_1	
        	bcf	adc_discharge		; make it output
        	clrwdt                 		; clear WDT
        	bsf	OPTION_REG, PSA 	; assign prescaler to WDT
        	bank_0
        	;			 <<<<<
        	
        	bcf	flag_adc_invalid	; this will get set if the conversion
        					; got screwed up by some other intr
        	clrf    adc_raw_high    	; clear high byte of adc_raw
        	
        	clrf    TMR0            	; clear the timer 

		movfw	PORTB
		INTCON_ADC			; enable portb int on change only
						; and reset timer overflow bit
		bank_1
		bsf	adc_discharge		; make it input so that it does not
		bank_0				; screwed up by read-modify-write

adc_while_loop:	btfss   INTCON, RBIE     	; while INTCON, RBIE
        	 goto    adc_done
        	btfss	INTCON, T0IF    	; skip on no timer overflow
		 goto	adc_while_loop		; no overflow - continue with loop
        	incf    adc_raw_high, F 	; \
		bcf	INTCON, T0IF		;  \ these lines handle the
		movlw	8			;  / timer overflow
		addwf	temp,F			; / 
		goto	adc_while_loop		; overflow handled - continue with loop
		
adc_done:      	movfw	TMR0			; adc_raw.low := TMR0
		movwf	adc_raw_low

        	btfsc	INTCON, T0IF    	; skip on no timer overflow
		incf	adc_raw_high,F		; handle late timer overflow
		
		movfw	temp			; 
		addlw	1			; this is a rough estimation!!!
		movwf	TMR0			; restore hardware timer
		bcf	INTCON, T0IF		; 
		
		btfsc	flag_adc_invalid
		return
	
#ifdef MODE_TEST
		call	ln_send_raw	
#endif

		movfw	loco_spd	; if loco_spd!=ESTOP
		xorlw	SPD_ESTOP
		bnz	adc_normal	;  than goto adc_normal
					;  else look for poti in left margin
		call	adc_raw2byte	; calculate 8 bit value from raw value
		addlw	0x00		; if 8 bit value!=0
		movlw	SPD_ESTOP
		movwf	loco_spd	; restore ESTOP in loco_spd
		skpz			
		 return			;  than do nothing
		
		movlw	SPD_STOP	;  else
		movwf	loco_spd	; loco_spd=SPD_STOP
		bsf	flag_request_speedupdate  ; send speed value=0 to chief
		bsf	flag_update_led	; change to normal green led
		 return
	
adc_normal:	call	adc_raw2byte	; raw value to 8 bit value
		movwf	adc_raw_high	; save new 8 bit value to adc_raw_high

		clrc
		rrf	adc_raw_high,W	; loco_spd=<8 bit value> div 2
		movwf	loco_spd
		decfsz	loco_spd,F	; loco_spd--
		 incf	loco_spd,F	; if (loco_spd!=0) loco_spd++
		
		movfw	adc_raw_high
		subwf	adc_8bit,W	; subtract old from new 8 bit value

		skpc			; w=abs(w)
		 sublw	0
		 
		addlw	-2		; if (w<2)
		skpc
		 return			;  ignore
				
		movfw	adc_raw_high
		movwf	adc_8bit	; 8 bit value := adc_raw_high
		
		bsf	flag_request_speedupdate  ; send update to chief
	        return			; done.
        

adc_handle_overflow:
		return
		

adc_raw2byte:	ee_read	ee_adc_min_low		; W=adc_raw_min_low;
		subwf	adc_raw_low,F		; adc_raw_low-=W;
		skpnc
		 goto	adc_r2s_no_underflow
		
		movfw	adc_raw_high		; if (adc_raw_high==0)
		skpnz				;      return 0;
		 retlw	0
		decf	adc_raw_high,F 		; adc_raw_high--;
		 
adc_r2s_no_underflow:
		ee_read	ee_adc_factor		; EEDATA=adc_factor;

		clrf	EEADR			; EEADR=low byte of 16-bit-result (needed for carry only)
		clrf	temp			; temp=high byte of 16-bit-result
		
adc_r2s_loop
		btfss	EEDATA,0		; if (factor&1)
		 goto	adc_r2s_noadd		; {
		
		movfw	adc_raw_low		;    W=adc_raw_low;
		addwf	EEADR,F			;    EEADR+=W;	// the carry is the interesting part
		skpc				;    if (C)
		 goto	adc_r2s_no_overflow	;    {
		incf	temp,F			;        temp++;
		skpnz				;        if (Z)
		 retlw	0xff			;           return 0xff;
adc_r2s_no_overflow				;    }
		movfw	adc_raw_high		;    W=adc_raw_high
		addwf	temp,F			;    temp+=W;
		skpnc				;    if (C)
		 retlw	0xff			;       return 0xff;
						; } // if(factor&1)
adc_r2s_noadd	
		clrc				; C=0;
		rrf	EEDATA,F		; EEDATA>=1;       // through carry!
		tstf	EEDATA			; Z=(factor==0)
		skpnz				; if (Z)
		 goto	adc_r2s_endloop		;    goto adc_r2s_endloop
		
		clrc				; C=0;
		rlf	adc_raw_low,F		; adc_raw_low<=1;  // through carry!
		rlf	adc_raw_high,F		; adc_raw_high<=1; // through carry!
		skpnc				; if (C)
		 retlw	0xff			;    return 0xff;
		goto	adc_r2s_loop

adc_r2s_endloop		
		movfw	temp			; return temp;
		return				; 
		
        
	endm
	
