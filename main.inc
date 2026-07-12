;*******************************************************************
; FRED\MAIN.INC
;
; Copyright: Stefan Bormann (stefan.bormann@gmx.de)
;            Martin Pischky (martin.pischky@fernuni-hagen.de)
;
; SEE COPYRGHT.TXT FOR COPYRIGHT INFO!!!
;
; History: 980125sb  Created by splitting main file
;          9802??sb  Worked on 1.0
;          980303sb  Made reconnect more reliable
;          980313sb  Revised reconnect feature again. Reconnect
;                    tries until loco is taken by another loco
;                    or user disconnects by key command
;          980804sb  Kicked out left over error detection
;                    in order to save program memory
;          980809sb  Revised function key handling
;                    - made more code space efficient [pi:Grrrr] :-)
;                    - introduced function mapping
;          980815sb  buttons are now polled syncronous to adc conv.
;          000319sb  fixed reconnect as uFRED
;          021129sh  Enhanced function key handling, F3 added, Lenz V36 mode added
;          030411pi  Version 1.7: use FOURKEYS - code for FD is conditional include
;
;
; This is the main program for the SLNTHR project including
; upper layer network protocoll
;*******************************************************************
;	
;		                               init
;		                                Y
;		                              start
;		                                Y
;		                     _____________________
;		                    /                     \
;	  +--------------->+<[yes]-< loco addr. in eeprom? >-[no]-+
;	  |	           |        \_____________________/       |
;	  |	           |                                      |
;	  |	           |                     +----->----------+
;	  |	           V                     |                V
;	  |	+---------------------------+    |    +-----------------------+
;	  |	|     main_reconnect        |    |    |       main_idle       |
;	  |	| - - - - - - - - - - - - - |    |    |- - - - - - - - - - -  |
;	  |	| > request loco by address |    |    | wait for dispatch key |
;	  A	|    if IN_USE & same ID    |    |    | to be pressed         |
;	  |	|     than  reconnect       |    |    +-----------------------+
;	  |	|    if COMMON/FREE grab it |    A                |
;	  |	| if loco taken by other    |	 |		  |
;	  |	| throttle=>give up.        |    |                |
;	  |	| if no success=>retry loop |    |                |
;	  |	+---------------------------+    |      [dispatch key pressed]
;	  |	      |      |      |            |                V
;	  +<-[failure]+  [success]  +-[l.taken]->+      +-------------------+
;	  A	             |      |            |      |   dispatch get    |
;	  |	             V      +[usr.abort]>+      +-------------------+
;	  |                  |                   |          |          |
;	  |                  |                   A      [failure]  [success]
;	  |                  |                   +----<-----+          |
;	  |                  |                   |                     |
;	  |                  +---<-------------- | ---<----------------+
;	  |                  V                   |
;	  |      +---------------------+         |
;	  |      |   connect to slot   |         |
;	  |	 | - - - - - - - - - - |         |
;	  |	 | > write new data    |         |
;	  |	 |   to slot in chief  |         A
;	  A	 | > write address and |         |
;	  |	 |   speed mode to     |         |
;	  |  	 |   eeprom            |         |
;	  |	 +---------------------+         |
;	  |           |      |                   |
;	  +-[failure]-+  [success]               |
;	                     V                   |
;		+-----------------------+        |
;		|     normal operation  |        |
;		| - - - - - - - - - - - |        |
;		| > poll buttons and    |        |
;		|   and generate        |        A
;		|   apropriate <spd>,   |        |       
;		|   <dirf> and <snd>-   |        |
;		|   messageses          |        |
;		| > whatch out for      |        |
;		|   dispatch key        |        |
;		+-----------------------+        A
;		          |                      |
;		          +[dispatch key pressed]+



MAIN		MACRO
		
;-----------------------------------------------------------------------------
;				loco addr. in eeprom?
;-----------------------------------------------------------------------------

		call	main_read_ee_adr
		btfsc	flag_temp
		 goto	main_reconnect	; if (address!=0) try to reconnect


;-----------------------------------------------------------------------------
;				main_idle
;-----------------------------------------------------------------------------

main_reconnect_wrong_id:

main_idle:	INTCON_NONE
		STOP_REFRESH
		clrf	ln_status		; not receiving any particular message!
		clrf	loco_slot		; no loco assigned!
		bsf	flag_update_led		; report this on leds (steady red)
		
		ee_clear ee_adr_lo		; wipe out old address
		ee_clear ee_adr_hi

main_idle_loop:	call	check_timer
		call	get_button_event
		xorlw	event_dispatch		; compare w with dispatch_event
		skpz				; if not equal
		 goto	main_idle_loop		;  than resume idle_loop


;-----------------------------------------------------------------------------
;				dispatch get
;-----------------------------------------------------------------------------
		
		movlw	led_connect			; display fast yellow flashing to
		movwf	leds				; indicate that reconnect is process
		
		call	ln_send_dispatch_get		; send DISPATCH_GET
		btfsc	lnstat_senderror		; if not successfully sent
		 goto	main_idle			;  than resume idle_loop

; waiting for reply of dispatch_get
		call	main_wait_any_data		; give chief time to answer
		btfsc	lnstat_wait_any_data		; did we receive any slot data? #
		 goto	main_idle			;  if not than resume idle_loop

; remember address for later reconnect
		ee_write_var  ee_adr_hi,  loco_adr_hi
		ee_write_var  ee_adr_lo,  loco_adr_lo
		ee_write_var  ee_status1, loco_stat1
		ee_write_var  ee_functionconfig, functionconfig

; setting up assigned slot data
		movlw	SPD_ESTOP
		movwf	loco_spd			; estop to loco speed

		call	button_update_dir		; loco_dirf[SL_DIR]:=dir_switch
		goto	main_connect_to_slot


;-----------------------------------------------------------------------------
;				main_reconnect
;-----------------------------------------------------------------------------


main_reconnect:	
		movlw	led_connect		; display fast yellow flashing to
		movwf	leds			; indicate that connect is process

main_reconnect_wait_and_retry:
		bcf	flag_reconnect			; wait 500 ms
		TIMER_START	reconnect_timer, reconnect_time
connect_wait_loop:
		call	get_button_event
		xorlw	event_dispatch		; compare w with dispatch_event
		skpnz				; if equal
		 goto main_idle			;  user wants to abort reconnection
		call	check_timer
		btfss	flag_reconnect		; loop exit criteria: flag_reconnect
		goto	connect_wait_loop	; is set by timer proc.

		call	main_read_ee_adr
		call	ln_send_loco_adr
		btfsc	lnstat_senderror
		 goto	main_reconnect_wait_and_retry

		call	main_wait_any_data		; give chief time to answer
		btfsc	lnstat_wait_any_data		; did we receive any slot data?
		 goto	main_reconnect_wait_and_retry	;  if not than resume idle_loop
		btfss	lnstat_adr_mismatch
		 goto	main_reconnect_check_IN_USE
	;	movlw	debug_adr_mismatch
	;	movwf	debug_high
	;	movwf	debug_low
	;	call	ln_send_debug
		goto	main_reconnect_wait_and_retry
	
main_reconnect_check_IN_USE:	
; check IN_USE flag
		btfss	loco_stat1, SL_ACTIVE		; if not ACTIVE (assigned to throttle?)
		 goto	main_grab_loco			;  than grab slot
		btfss	loco_stat1, SL_BUSY		; if BUSY (refreshed)
		 goto	main_grab_loco			;  than grab slot
							;  else loco is IN_USE
; check if ID of slot is the same of throttle
		ee_read	ee_id1
		xorwf	loco_id1,W
		skpz
		 goto	main_reconnect_wrong_id		; if slot_id1<>ID1 than ->idle
		ee_read	ee_id2
		xorwf	loco_id2,W
		skpz
		 goto	main_reconnect_wrong_id		; if slot_id2<>ID2 than ->idle
		goto	main_finish_reconnect		; id ok and slot IN_USE ->reuse "our" slot
	
main_grab_loco:						; 
		call	ln_send_null_move		; slot not IN_USE -> reestablish  
		btfsc	lnstat_senderror		;				  
		 goto	main_reconnect_wait_and_retry	;				  
							;				  
		call	main_wait_any_data		; wait for loco data		  
		btfsc	lnstat_wait_any_data		;				  
		 goto	main_reconnect_wait_and_retry	;				  
		btfss	lnstat_adr_mismatch		;				  
		 goto	main_finish_reconnect		;				  
	;	movlw	debug_adr_mismatch		;				  
	;	movwf	debug_high			;				  
	;	movwf	debug_low			;				  
	;	call	ln_send_debug			;
		bcf	loco_stat1,SL_ACTIVE
		call	ln_send_wr_sl_data				  
		goto	main_reconnect_wait_and_retry	;				  

main_finish_reconnect:	
		ee_read	ee_status1
		movwf	loco_stat1
; check if dir switch is touched in the meantime
		movfw	loco_dirf			; remember dir+F
		call	button_update_dir		; update dir bit in dirf (MUST NOT USE W)
		xorwf	loco_dirf,W			; same as before?
		skpnz					; if yes
		 goto	main_connect_to_slot		;  than proceed
		movlw	SPD_ESTOP
		movwf	loco_spd
		goto	main_connect_to_slot
		
		

;-----------------------------------------------------------------------------
;				connect to slot
;-----------------------------------------------------------------------------


main_connect_to_slot
; filling in throttle ID into slot data
		ee_read	ee_id1
		movwf	loco_id1			; loco_id1:=EEPROM[ee_id1]
		ee_read	ee_id2
		movwf	loco_id2			; loco_id2:=EEPROM[ee_id2]

; setting loco to IN_USE
		bsf	loco_stat1, SL_BUSY
		bsf	loco_stat1, SL_ACTIVE		

; writing slot with modified data back
		call	ln_send_wr_sl_data		; write slot data
		btfsc	lnstat_senderror
		 goto	main_reconnect_wait_and_retry	; retry
		
; waiting for reply of modifying slot data
		call	main_wait_any_data
		btfsc	lnstat_wait_lack		; did we receive long ack?
		 goto	main_reconnect_wait_and_retry	;  if not [failure]
							;  if yes [success]

;-----------------------------------------------------------------------------
;				normal operation
;-----------------------------------------------------------------------------

main_normal:	clrf	ln_status
		bsf	flag_update_led
		START_REFRESH
		bsf	flag_do_adc
		bsf	flag_request_speedupdate		
main_normal_loop:
		call	check_timer
		
		btfss	flag_do_adc
		 goto	main_normal_noadc
		bcf	flag_do_adc	
		call	adc
		call	main_handle_buttons
main_normal_noadc:
		
		btfsc	flag_request_speedupdate
		 call	ln_speedupdate
		bcf	flag_request_speedupdate
		goto	main_normal_loop
		

main_handle_buttons:
; Jumptable - inspired by AN556, but optimized :-)
		movlw	HIGH event_table	; PCLATH = high(t_buttons_tbl)
		movwf	PCLATH

		call	get_button_event	; MUST NOT MODIFY PCLATH!!!
		andlw	B'00001111'		; W = buttonstate & 15
		
		addlw	LOW event_table		; W += low(t_buttons_tbl)
		skpnc				; handle overflow
		incf	PCLATH,F
		movwf	PCL			; Jump!  (PC = W | (PCLATH<<8))
		
event_table:	return				; 0	no event
		goto	main_event_p_FA		; 1	A pressed
		return				; 2	A released
		goto	main_event_p_FB		; 3	B pressed
		goto	main_event_p_FBr	; 4	B released
		goto	main_event_p_FC		; 5	C pressed
		goto	main_event_p_FCr	; 6	C released
		goto	main_event_p_ESTOP	; 7	ESTOP pressed
		return				; 8	ESTOP released
		goto	main_event_f_DIR	; 9	DIR forward
		goto	main_event_r_DIR	; 10	DIR reverse
		goto	main_event_dispatch	; 11	dispatch combination
                #ifdef FOURKEYS
		 goto	main_event_p_FD		; 12	D pressed
		 return				; 13	D released
                #endif
                #ifndef FOURKEYS
		 goto	0			; 12
		 goto	0			; 13
                #endif
		goto	0			; 14
		goto	0			; 15
		




main_event_finish_dirf:
		xorwf	loco_dirf,F
main_event_exit_dirf:
		call	ln_send_loco_dirf	; tell chief the new dirf
		return


main_event_p_FA:
		movlw	(1 << SL_F0)		; bit mask into W
		goto	main_event_finish_dirf	


main_event_p_FB:
		btfsc	functionconfig,0
		 goto	main_event_p_FB_other
		movlw	(1 << SL_F1)
		goto	main_event_finish_dirf

main_event_p_FB_other:
		btfss	functionconfig,1
		 goto	main_event_p_FB_34
		movlw	(1 << SL_F1)
		iorwf	loco_dirf,F
		goto	main_event_exit_dirf

main_event_p_FB_34:
		movlw	(1 << SL_F3)
		goto	main_event_finish_dirf
		
main_event_p_FBr:
		btfss	functionconfig,1
		 return
		movlw	~(1 << SL_F1)
		andwf	loco_dirf,F
		 goto	main_event_exit_dirf


main_event_p_FC:
		btfsc	functionconfig,0
		 goto	main_event_p_FC_other
		movlw	(1 << SL_F2)
		 goto	main_event_finish_dirf

main_event_p_FC_other:
		btfss	functionconfig,1
		 goto	main_event_p_FC_34
		movlw	(1 << SL_F2)
		iorwf	loco_dirf,F
		 goto	main_event_exit_dirf

main_event_p_FC_34:
		movlw	(1 << SL_F4)
		 goto	main_event_finish_dirf

main_event_p_FCr:
		btfss	functionconfig,1
		 return
		movlw	~(1 << SL_F2)
		andwf	loco_dirf,F
		 goto	main_event_exit_dirf

#ifdef FOURKEYS

main_event_p_FD:
		btfsc	functionconfig,0
		 goto	main_event_p_FD_other
main_event_p_FD3:
		movlw	(1 << SL_F3)
		 goto	main_event_finish_dirf

main_event_p_FD_other:
		btfsc	functionconfig,1
		 goto	main_event_p_FD3
		movlw	(1 << SL_F1)
		goto	main_event_finish_dirf

#endif

main_event_p_ESTOP:
		call	set_speed_estop
		return


main_event_f_DIR:
		bsf	loco_dirf, SL_DIR
		goto	main_normal_dirf


main_event_r_DIR:
		bcf	loco_dirf, SL_DIR
main_normal_dirf:
		tstf	loco_spd
		skpz
		call	set_speed_estop
		call	ln_send_loco_dirf
		return


main_event_dispatch:
		bcf	loco_stat1, SL_ACTIVE	; status1:=IDLE
		movlw	SPD_ESTOP
		movwf	loco_spd
		call	ln_send_wr_sl_data	; release loco
		goto	main_idle


;*****************************************************************************
;* main_read_ee_adr reads the loco address from eeprom into variables
;* return value: flag_temp=0 means, the address is 0:0
;* Pseudo code:
;*
;* void main_read_ee_adr()
;* {
;*      functionconfig=eeprom(ee_functionconfig);
;*	flag_temp=0;
;*	loco_adr_lo=eeprom(ee_adr_lo);
;*	if (loco_adr_lo)
;*		flag_temp=1;
;*	loco_adr_hi=eeprom(ee_adr_hi);
;*	if (loco_adr_hi)
;*		flag_temp=1;
;* }

main_read_ee_adr:
		ee_read	ee_functionconfig
		movwf	functionconfig
		
		bcf	flag_temp		; clear return value
		
		ee_read	ee_adr_lo
		movwf	loco_adr_lo
		skpz
		 bsf	flag_temp
		
		ee_read	ee_adr_hi
		movwf	loco_adr_hi
		skpz
		 bsf	flag_temp
		 
		return
		
		
;*****************************************************************************
;* main_wait_any_data gets called after sending a messages that receives
;* a data message as reply.
;* Pseudo Code:
;*
;* void main_wait_any_data()
;* {
;*	enable_loconet_interupt();
;*	check_timer();
;*	ln_status=0;		// ln_status is bit array
;*	ln_status[wait_any_data]=1;
;*	ln_status[wait_lack]=1;
;*	ln_recv_wait_reply();
;*	disable_loconet_interupt();
;*	check_timer();
;* }

main_wait_any_data:
		INTCON_LOCONET
		clrf	ln_status			; clear all bits, including activity bit
		bsf	lnstat_wait_any_data		; wait for any slot data
		bsf	lnstat_wait_lack		; wait for long ack, too
		call	ln_recv_wait_reply		
		INTCON_NONE
		call	check_timer			; keep system alive...
		return


set_speed_estop:
		movlw	SPD_ESTOP
		movwf	loco_spd
		bsf	flag_update_led
		call	ln_speedupdate
		return

		ENDM
		
