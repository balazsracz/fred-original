debug_subopc_cd		equ	42
debug_subopc_button	equ	43

TEST_MAIN	MACRO

;--- Check LEDS --------------------------------------------------------------
		movlw	phase0(red)+phase1(green)+phase2(green)
		movwf	leds
		call	delay1000
		call	delay1000
		call	delay1000

;--- Check CD ----------------------------------------------------------------
		movlw	steady(yellow)
		movwf	leds
		call	delay1000
waitforCD:	SKPNOCARRIER
		goto	waitforCD
		
		SKPMARK
		goto	test_abort_red

;--- CD idle ok --------------------------------------------------------------

		movlw	steady(green)
		movwf	leds
		call	delay1000

;--- Measure CD Time----------------------------------------------------------

		movlw	steady(yellow)
		movwf	leds
		call	delay250
		SENDSPACE
		nop
		nop
		nop
		clrf	loco_id2
		SKPSPACE
		 goto	test_abort_red
		SENDMARK
measure_cd_loop:
		DELAY	4			; 1-4	loop period 10us
		incf	loco_id2,F		; 5
		SKPNZ				; 6
		 goto	measure_cd_abort	; 7
		SKPNOCARRIER			; 8
		 goto	measure_cd_loop		; 9,10
measure_cd_abort:

;--- CD measurement successfull ----------------------------------------------

		movlw	steady(green)
		movwf	leds
		call	delay1000

;--- Loconet Transmit --------------------------------------------------------

		movlw	debug_subopc_cd
		movwf	loco_id1
		call	ln_send_debug
		
;--- Button/ADC Loop ---------------------------------------------------------

		movlw	slowflash(red,green)
		movwf	leds
test_final_loop:
		call	check_timer
		call	get_button_event
		xorlw	0x00
		skpnz
		goto	test_no_button
		movwf	loco_id2
		movlw	debug_subopc_button
		movwf	loco_id1
		call	ln_send_debug
test_no_button:
		call	adc
		call	delay250
		call	check_timer
		call	delay250
		goto	test_final_loop
		
		
		
		P_LN_SEND_DEBUG
		P_LN_SEND_RAW


test_abort_red:	movlw	steady(red)
		movwf	leds
test_abort_loop:
		call	check_timer
		goto	test_abort_loop

delay1000:	call	delay250
delay750:	call	delay250
delay500:	call	delay250
		call	delay250
		return


delay250:	movlw	250
		movwf	temp
delay250_loop:	call	check_timer
		DELAY	1000		; 1 ms exact delay
		decfsz	temp,F
		goto	delay250_loop
		return
	
		ENDM



test_led	macro
		movlw	led_fastblink_red_green
		movwf	leds
test_led_loop:	call	check_timer
		goto	test_led_loop
		endm



test_raw	macro
		movlw	led_fastblink_red_green
		movwf	leds
		movlw	0x12
		movwf	loco_slot
		
raw_test_loop	call	adc
		
		movlw	255
		movwf	temp
raw_test_delay_loop:
		call 	check_timer
		DELAY	1000
		decfsz	temp
		goto	raw_test_delay_loop
		
		goto	raw_test_loop

		endm
		

test_adc	macro
		movlw	0x14
		movwf	loco_slot
test_adc_loop	
		call	check_timer
		call	adc
		goto	test_adc_loop

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
		

test_init_loco	macro
		movlw	SPD_ESTOP
		movwf	loco_spd
		clrf	loco_dirf
		bsf	loco_dirf, SL_DIR
		clrf	loco_snd
 		movlw	0x13
		movwf	loco_slot
		movlw	7
		movwf	loco_stat1
		clrf	loco_stat2
		bsf	loco_stat2, 3
		movlw	0x47
		movwf	loco_adr_hi		
		movlw	0x11
		movwf	loco_adr_lo
		ee_read	ee_id1
		movwf	loco_id1
		ee_read	ee_id2
		movwf	loco_id2
		endm
		
		
test_spd	macro
		movlw	led_fastblink_red
		movwf	leds
		test_init_loco
test_spd_loop	call	ln_send_loco_spd
		DELAY	1000
		goto	test_spd_loop
		endm
		
test_send_byte:	macro
test_send_byte_loop
		movlw	0xaa
		clrf	ln_status
		call	ln_send_byte
		movlw	42
		movwf	ln_byte_temp
test_send_byte_delay
		DELAY 57
		decfsz	ln_byte_temp, F
		goto	test_send_byte_delay
		goto	test_send_byte_loop
		endm	
		
		
test_ln_send	macro
		test_init_loco
		call 	delay10
		call	ln_send_loco_spd
		call 	delay10
		call	ln_send_loco_dirf
		call 	delay10
		call	ln_send_loco_snd
		call 	delay10
		call	ln_send_loco_stat1
		call 	delay10
		call	ln_send_wr_sl_data
		call 	delay10
		call	ln_send_rq_sl_data
		call 	delay10
		call	ln_send_loco_adr
		call 	delay10
		call	ln_send_null_move
		call 	delay10
		call	ln_send_dispatch_get
		sleep
		endm
		
test_ln_recv	macro
		movlw	led_fastblink_red_green
		movwf	leds
		test_init_loco
fuckforchief:	call	delay10
		movlw	0x42
		movwf	ln_send_checkbyte
		call	ln_send_loco_adr
		clrf	ln_status
		bsf	ln_status, lnstat_wait_any_data
		call	ln_recv_wait_reply
		addlw	0x00
		movlw	0x42
		movwf	ln_send_checkbyte
		skpz
		goto	test_ln_recv_ok
		call	ln_send_loco_stat1
		goto fuckforchief
test_ln_recv_ok	btfsc	ln_status, lnstat_wait_any_data
		goto	test_ln_recv_x
		call	ln_send_wr_sl_data
		goto fuckforchief
test_ln_recv_x	call	ln_send_loco_spd
		bsf	nc_1
		nop
		bcf	nc_1
		call send_gpoff
		goto fuckforchief

send_gpoff:	SEND_INIT
		SEND_CONSTANT 0x82
		SEND_DONE		
		endm



test_button	macro
		test_init_loco
		movlw	led_fastblink_red
		movwf	leds
test_button_loop:
		call	check_timer
		call	get_button_event
		addlw	0
		skpnz
		goto	test_button_loop
		movwf	loco_spd
		call	ln_send_loco_spd
		goto	test_button_loop
		endm
		
		
test_eeprom	macro
		test_init_loco
		bsf	nc_1
		ee_write_var  ee_status1, loco_stat1
		bcf	nc_1
		ee_write_var  ee_adr_hi,  loco_adr_hi
		bsf	nc_1
		ee_write_var  ee_adr_lo,  loco_adr_lo
		bcf	nc_1
		goto	$
		endm
		
