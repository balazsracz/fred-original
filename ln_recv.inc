ISR_LN_RECV	macro
		bsf	lnstat_recv_activity	; 11
		bcf	lnstat_recverror	; 12	no error
		movlw	0xff			; 13		init checkbyte
		movwf	ln_recv_checkbyte	; 14
		DELAY	13			; 15-27
		call	ln_recv_byte		; 28,29-35	receive opcode

		btfsc lnstat_recverror			; 36		if error flag set
		goto	ln_recv_end		; 37		 then recv-error

		xorlw	OPC_LONG_ACK		; 38		check for OPC_LONG_ACK
		bz	ln_recv_long_ack	; 39,40

		xorlw	OPC_LONG_ACK^OPC_SL_RD_DATA ; 41	check for OPC_SL_RD_DATA
		bz  	ln_recv_sl_rd_data	; 42,43

		xorlw	OPC_SL_RD_DATA^OPC_BUSY ; 44		check for OPC_BUSY
		bz	ln_recv_masterbusy	; 45

	;	movlw	debug_code_unknown_opcode
	;	movwf	debug_code
		bsf	lnstat_recverror

ln_recv_end:
		bcf	INTCON, INTF		; (47)
		endm




LN_RECV_HANDLE_ERROR macro
		 btfsc lnstat_recverror
		 goto	ln_recv_end
		endm

LN_RECV_CHECK	macro
	local cont
		 call	ln_recv_wait_startbit  ; <60
		 LN_RECV_HANDLE_ERROR	; 36,37
		 movfw	ln_recv_checkbyte	; 38
		 skpnz			; 39
		 goto	cont
	;	 movlw	debug_code_checkfailed
	;	 movwf	debug_code
		 goto	ln_recv_end	; 40
cont:
		endm

LN_RECV_BYTE_TO_VAR macro variable
		 call	ln_recv_wait_startbit
		 LN_RECV_HANDLE_ERROR
		 movwf	variable
		endm


P_LN_RECV	MACRO

;-----------------------------------------------------------------------------
; The procedure "ln_recv_byte" receives one byte from loconet
; Input parameter: none, assumes carry cleared
; Output parameter: Received data byte in W register
;                   Carry is set on framing error
; Variable usage: "ln_byte_temp" holds data byte during assembly
;                 "ln_recv_checkbyte" gets updated by received data byte
;

ln_recv_bit:	clrc			; 29
		SKPSPACE		; 30	precondition: carry cleared (no error so far) 
		setc			; 31
		rrf	ln_byte_temp,F	; 32	rotoate data from carry into data byte
		DELAY	52		; 33..24
		return			; 25,26

ln_recv_wait_startbit:
		clrf	ln_byte_temp	; use ln_byte_temp as timeout-counter time out after 256*5 cycles
ln_rws_loop:	SKPMARK			; (0/1)/2/(3/4)	gemittelt 2
		goto ln_rws_go		; 3,4 (+-2)
		decfsz	ln_byte_temp, F
		goto	ln_rws_loop
ln_rws_fail:	bsf lnstat_recverror		; start-bit TIMEOUT error!!!
	;	movlw	debug_code_interbyte_timeout
	;	movwf	debug_code
		return

ln_rws_go:	DELAY	25		; 5-29
ln_recv_byte:	SKPMARK			; 30	no start bit?
		goto	ln_recv_byte_startbitok ; 31,32
		bsf	lnstat_recverror	; FRAMING error
		movlw	debug_code_badstartbit
		goto	ln_recv_byte_error
ln_recv_byte_startbitok:
		clrf	ln_byte_temp	; 33	prepare byte buffer
		DELAY	54		; 34..26
		call	ln_recv_bit	; 27,28-26	receive bit 0
		call	ln_recv_bit	; 27,28-26	receive bit 1
		call	ln_recv_bit	; 27,28-26	receive bit 2
		call	ln_recv_bit	; 27,28-26	receive bit 3
		call	ln_recv_bit	; 27,28-26	receive bit 4
		call	ln_recv_bit	; 27,28-26	receive bit 5
		call	ln_recv_bit	; 27,28-26	receive bit 6
		call	ln_recv_bit	; 27,28-26	receive bit 7
		DELAY	4		; 27..30
		movfw	ln_byte_temp	; 31	return received byte in W
		xorwf	ln_recv_checkbyte,F	; 32	update check byte
		SKPSPACE		; 33	no stop bit?
		 return			; 34,35
		bsf lnstat_recverror	; FRAMING error
	;	movlw	debug_code_badstopbit
ln_recv_byte_error:
	;	movwf	debug_code
		return			; 
		
;----------------------------------------------


ln_recv_long_ack:
		btfss	lnstat_wait_lack		; 51
		goto	ln_recv_end			; 52
		call	ln_recv_wait_startbit		; 53	discard opcode, assume the same as sendet
		LN_RECV_HANDLE_ERROR			;	  (time out active!)
		call	ln_recv_wait_startbit		; receive return value
		LN_RECV_HANDLE_ERROR
		movwf	ln_lack				; store return value regardless of correctness
		LN_RECV_CHECK		
		bcf	lnstat_wait_lack	; remember: not again!
		goto ln_recv_end

ln_recv_sl_rd_data:
		btfsc	lnstat_wait_any_data	; 55
		goto	ln_rsrd_go			; 56
		btfss	lnstat_wait_slot_data; 57
		goto	ln_recv_end			; 58
ln_rsrd_go	call	ln_recv_wait_startbit		; 59|58	receive length byte
		LN_RECV_HANDLE_ERROR
		sublw	0x0E				; compare length with 0x0E
		skpz					; if not equal
		goto	ln_recv_end			;  than abort, can only cope with Loconet 1.1
		call	ln_recv_wait_startbit		; receive slot#
		LN_RECV_HANDLE_ERROR
		btfss	lnstat_wait_slot_data
		goto	ln_rsrd_any
		subwf	loco_slot,W			; compare received with stored slot#
		skpz					; if not equal
		goto	ln_recv_end			;  than abort
		goto	ln_rsrd_slot
ln_rsrd_any	movwf	loco_slot
ln_rsrd_slot	LN_RECV_BYTE_TO_VAR loco_stat1
		LN_RECV_BYTE_TO_VAR temp		; receive adr_lo
		subwf	loco_adr_lo,W			; compare to stored version
		skpz					; if not equal
		 bsf	lnstat_adr_mismatch		;   than remember
		movfw	temp				; store
		movwf	loco_adr_lo			;  it
		LN_RECV_BYTE_TO_VAR loco_spd
		LN_RECV_BYTE_TO_VAR loco_dirf
		LN_RECV_BYTE_TO_VAR trk
		LN_RECV_BYTE_TO_VAR loco_stat2
		LN_RECV_BYTE_TO_VAR temp		; receive adr_hi...
		subwf	loco_adr_hi,W
		skpz
		 bsf	lnstat_adr_mismatch
		movfw	temp
		movwf	loco_adr_hi
		LN_RECV_BYTE_TO_VAR loco_snd
		LN_RECV_BYTE_TO_VAR loco_id1
		LN_RECV_BYTE_TO_VAR loco_id2
		LN_RECV_CHECK		
		bcf	lnstat_wait_any_data
		bcf	lnstat_wait_slot_data
		goto ln_recv_end


ln_recv_masterbusy:
		bcf	lnstat_wait_masterbusy	; 46
		goto	ln_recv_end
		
	
;*****************************************************************************
; "ln_recv_wait_reply" is a procedure that waits for a reply from the chief
; to the throttle. If the chief answeres, it answers before anybody else
; is allowed to take the bus under control. However, other command stations
; might answer too early (still during carrier_detect) or too late (after bus
; is formally free).
;
; Used Variable: ln_loop_counter
; Pseudo Code:
;
; boolean ln_recv_wait_reply()
; {
;	for (;;)
;	{
;		ln_status[wait_masterbusy]=1;
;		ln_status[recv_activity]=0;
;		do
;			if (loconet_rx is low) set_software_interrupt;
;		while (carrier_detect);
;		ln_loop_counter=250;
;		for (;;)
;		{
;			if (ln_status[recv_activity])
;				break;
;			check_timer();
;			delay 200 usec;
;			ln_loop_counter--;
;			if (ln_loop_counter==0) 
;				return false;
;		}
;		if (ln_status[wait_masterbusy])
;			return true;
;	}
; }

ln_recv_wait_reply:                                     ; <------------------------------------+
        ;       clrf    debug_code                      ;                                      |
                bsf     lnstat_wait_masterbusy          ; Setup wait_masterbusy early          |
                bcf     lnstat_recv_activity            ; Clear activity flag early            |
ln_recv_carrier_detect_loop:                            ; <-------------------+                |
                btfss   loconet_rx                      ; is there a start bit coming?         |
                 bsf    INTCON, INTF                    ; if yes, simulate edge interrupt      |
                SKPNOCARRIER                            ; wait for no carrier |                |
                goto    ln_recv_carrier_detect_loop     ; >-------------------+                |
                                                        ;                                      A
                movlw   250                             ; wait up to 50 ms (250 * 200 us)      |
                movwf   ln_loop_counter                 ;                                      |
ln_rwr_loop:                                            ; <-+                                  |
                btfsc   lnstat_recv_activity            ; did we receive a packet?             |
                 goto   ln_rwr_checkbusy                ;  yes, check and return-+ or retry    |
                call    check_timer                     ; keep software timers running         |
                DELAY   191                             ; 200 cycles delay minus actual insn   |
                decfsz  ln_loop_counter,F               ;   |                    |             |
                 goto   ln_rwr_loop                     ; >-+                    |             |
        ;       movlw   debug_code_reply_timeout        ;                        |             |
        ;       movwf   debug_code                      ;                        |             |
                bsf     lnstat_recverror                ;                        |             |
                return                                  ;       timed out        |             |
                                                        ;                        |             A
ln_rwr_checkbusy:                                       ;                        |             |
                btfsc   lnstat_wait_masterbusy          ; <----------------------+             |
                 return                                 ;                                      |
                goto    ln_recv_wait_reply              ; master busy received, retry >--------+

		ENDM
