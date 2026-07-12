;*****************************************************************************
; LN_SEND.INC
;
; Copyright: Stefan Bormann (s.bormann@tu-bs.de)
;
; History:  9712XXsb  initial version
;           980815sb  made macro structure more modular 
;
; This file contains the code that sends packets over the loconet
; Contains:
;  - physical layer (manipulating the ports that are connected to the
;    loconet)
;  - bus arbitration:
;     o carrier sense (sensing the bus before attempting access)
;     o collision detect (while sending MARK, bus is probed to detect 
;       collision)
;     o in case of busy bus, transmit attempt is delayed (active waiting)
;     o in case of collision, priority delay is modified
;  - upper layer (whole packets are send according to LNPE1.DOC including
;    checkbyte)
;
; The numbers between the code and the comments are counting the cycles 
; within one bit time. A 4MHz crystal is assumed.
; 0 is is the cycle where the bit time startes with the update of the
; port "loconet_tx"
; 59 is the last cycle, it is followed by 60, which is equal to 0
;*****************************************************************************

;-----------------------------------------------------------------------------
; The procedure "ln_send_byte" sends one byte containing one start bit,
; eight data bits (from w register) and one stop bits.
; One bit time is 60 ms (60 cycles is one loconet bit time with a PIC 
; running 4Mhz)
; Input parameter: register w contains data to be send
; Output parameter: carry flag is set if a collision is detected
;                   and break is sent
; Variable usage: uses variable "ln_byte_temp" to store data
;                 updates xor-checksum in "ln_checksum"
;


SEND_CONSTANT	macro data
		 movlw	data		; 53
		 call	ln_send_byte	; 54,55
		endm
		
SEND_VARIABLE	macro var
		 movfw	var		; 53
		 call	ln_send_byte	; 54,55
		endm

send_count=0

SEND_INIT	macro
 		 call	ln_send_init1
send_count++
send_retry_#v(send_count)
		 call	ln_send_init2
		endm
		
SEND_DONE	macro
		 SEND_VARIABLE	ln_send_checkbyte		; send check byte
		 bcf	INTCON, INTF
		 bsf	INTCON, GIE
		 btfss lnstat_senderror				; if no error
		 return					;  than finish sending
		 decfsz	ln_priority,F			; on error, decrement priority delay
		 goto	send_retry_#v(send_count)	;  and retry
		 return					;  or abort if delay counted down to 0
		endm


P_LN_SEND	MACRO

;ln_send_done:	bcf	INTCON,INTF		; clear cs-int-flag
;		retfie				; return & gobal interrupt on


ln_send_init1:	movlw	25			; 25 transmit attempts
		movwf	ln_priority		; init priority delay
		return

 
ln_send_init2:	call	check_timer
		SKPNOCARRIER			; wait for CS=1 (no activity on loconet)
		goto	ln_send_init2

		movlw	5			; w:=priority-5
 		subwf	ln_priority,W
		skpc				; if result negativ than w:=0
		clrw 
		addlw	6+3			; backoff_time = priorty_delay+6
		movwf	ln_byte_temp

T	EQU TRUE

        IF T
ln_send_backoff_loop:				; loop for backoff_time bit times
		DELAY	54			; make one loop 60 cycles (1 bit time)
		SKPCARRIER			; if traffic on loconet
		goto 	nofuckingerror
		bsf lnstat_senderror
		return				;  return and do great loop because of carry flag
nofuckingerror:	
	ELSE
		bsf lnstat_senderror

ln_send_backoff_loop:				; loop for backoff_time bit times

		DELAY 55
		SKPNOCARRIER
		return
	ENDIF

		decfsz	ln_byte_temp,F		
		goto	ln_send_backoff_loop

		bcf lnstat_senderror		; initialize error flag: cleared=no_error
		
ln_send_init_gie:
		bcf	INTCON,GIE		; disable interrupts
		btfsc	INTCON,GIE
		goto	ln_send_init_gie
		movlw	0xff			; initialize xor-checkbyte
		movwf	ln_send_checkbyte
		return				; than go ahead


;-----------------------------------------------------------------------------
; ln_send_byte is the physical layer routine that sends one startbit,
; 8 data bits (from w register) and one stopbit.
; If it detects a collsion (by checking for a 0 on the bus while trying to
; send a 1), it calles "ln_send_collison", which puts a break signal of 
; 15 bit times on the bus.

ln_send_mark	SENDMARK			; 0	Sending 1
		DELAY	22			; 1-22	Wait for raise time
		movlw	4			; 23	init loop counter
ln_send_mark_loop:
		SKPMARK				; 24/../42
		goto	ln_send_collision	; 25/../43
		addlw	-1			; 26/../44
		skpz				; 27/../45
		goto	ln_send_mark_loop	; 28/../46
		return				; 47,48


ln_send_collision:
		bsf lnstat_senderror			; remember transmit failier
		SENDSPACE			; send break to signal collision
;		bsf	nc_1
		DELAY	900			; how long? (15 bit times of 60us)
;		bcf	nc_1
		return				; and return to "ln_send_byte"


ln_send_abort:	bsf lnstat_senderror			; remember transmit failier
		SENDMARK			; network driver back to idle
		return				; back to caller

ln_send_byte:	btfsc	lnstat_senderror	; 56	error before?
		goto	ln_send_abort		; 57	 than abort immediately
		SKPMARK				; 58	somebody else was faster?
		goto	ln_send_abort		; 59	 than abort immediately

		SENDSPACE			; 0	STARTBIT
		
		movwf	ln_byte_temp		; 1	save data
		xorwf	ln_send_checkbyte,F	; 2	check:=data xor check;
		
		DELAY	51			; 3-53
		
		movlw	8			; 54
		movwf	ln_loop_counter		; 55
ln_send_byte_loop:	
		btfss	ln_byte_temp,0		; 56	bit==0?
		goto	ln_send_bit0		; 57,58
		
ln_send_bit1:	call	ln_send_mark		; 58,59-48
		btfss lnstat_senderror			; 49
		goto	ln_send_endif		; 50,51
		goto	ln_send_abort
		
ln_send_bit0:	nop				; 59
		SENDSPACE			; 0	Sending 0
		DELAY	51			; 1-51
ln_send_endif:
		rrf	ln_byte_temp,F		; 52
		decfsz	ln_loop_counter,F	; 53
		goto	ln_send_byte_loop	; 54,55
		
		DELAY	2			; 56-57
		call	ln_send_mark		; 58,59-48 STOPBIT
		btfsc lnstat_senderror			; 49
		goto	ln_send_abort		; 50
		
		return				; 51,52	 then return

		ENDM

;-----------------------------------------------------------------------------

P_LN_SEND_RAW	macro
ln_send_raw	SEND_INIT
		rlf	adc_raw_low,W
		rlf	adc_raw_high,W
		andlw	0x7f
		movwf	trk
		SEND_CONSTANT	0xaf
		movlw	0x7f
		andwf	adc_raw_low,W
		call	ln_send_byte
		SEND_VARIABLE	trk
		SEND_DONE
		endm

P_LN_SEND_DEBUG macro
ln_send_debug:
		SEND_INIT
		SEND_CONSTANT	OPC_DEBUG
		SEND_VARIABLE	loco_id1
		SEND_VARIABLE	loco_id2
		SEND_DONE
		endm

P_LN_SEND_LOCO_ADR macro
ln_send_loco_adr:
		SEND_INIT
		SEND_CONSTANT	OPC_LOCO_ADR
		SEND_VARIABLE	loco_adr_hi
		SEND_VARIABLE	loco_adr_lo
		SEND_DONE
		endm

P_LN_SEND_RQ_SL_DATA macro
ln_send_rq_sl_data:
		SEND_INIT
		SEND_CONSTANT	OPC_RQ_SL_DATA
		SEND_VARIABLE	loco_slot
		SEND_CONSTANT	0
		SEND_DONE
		endm

P_LN_SEND_NULL_MOVE macro
ln_send_dispatch_get:
		clrf	loco_slot
ln_send_null_move:
		SEND_INIT
		SEND_CONSTANT	OPC_MOVE_SLOTS
		SEND_VARIABLE	loco_slot
		SEND_VARIABLE	loco_slot
		SEND_DONE
		endm

P_LN_SEND_LOCO_STAT1 macro
ln_send_loco_stat1:
		SEND_INIT
		SEND_CONSTANT	OPC_SLOT_STAT1
		SEND_VARIABLE	loco_slot
		SEND_VARIABLE	loco_stat1
		SEND_DONE
		endm
		
P_LN_SEND_LOCO_SPD macro
ln_send_loco_spd:
		SEND_INIT
		SEND_CONSTANT	OPC_LOCO_SPD
		SEND_VARIABLE	loco_slot
		SEND_VARIABLE	loco_spd
		SEND_DONE
		endm
		
P_LN_SEND_LOCO_DIRF macro
ln_send_loco_dirf:
		SEND_INIT
		SEND_CONSTANT	OPC_LOCO_DIRF
		SEND_VARIABLE	loco_slot 
		SEND_VARIABLE	loco_dirf
		SEND_DONE
		endm

P_LN_SEND_LOCO_SND macro
ln_send_loco_snd:
		SEND_INIT
		SEND_CONSTANT	OPC_LOCO_SND
		SEND_VARIABLE	loco_slot 
		SEND_VARIABLE	loco_snd
		SEND_DONE
		endm

P_LN_SEND_WR_SL_DATA macro
ln_send_wr_sl_data:
		SEND_INIT
		SEND_CONSTANT	OPC_WR_SL_DATA
		SEND_CONSTANT	0x0E		; length of packet
		SEND_VARIABLE	loco_slot
		SEND_VARIABLE	loco_stat1
		SEND_VARIABLE	loco_adr_lo
		SEND_VARIABLE	loco_spd
		SEND_VARIABLE	loco_dirf
		SEND_VARIABLE	trk
		SEND_VARIABLE	loco_stat2
		SEND_VARIABLE	loco_adr_hi
		SEND_VARIABLE	loco_snd
		SEND_VARIABLE	loco_id1
		SEND_VARIABLE	loco_id2
		SEND_DONE
		endm

		


