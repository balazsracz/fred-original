;*****************************************************************************
; LOCONET.INC
;
; Copyright: Stefan Bormann (stefan.bormann@gmx.de)
;
; History: 9712XXsb initial version
;          980421sb split error flag (5) into two flags for send (6)
;                   and receive (7)
;
; This file contains the entire Loconet subsystem.
; Variables are defined here and the code is included from various files
;*****************************************************************************

; Variable definition

	cblock
        	; vars for loconet subsystem ----------------------
        	ln_byte_temp		: 1 ; buffer to recv/send one byte
        	ln_loop_counter		: 1 ; counter for several loops
		ln_send_checkbyte 	: 1 ; buffer for xoring bytes of packet
		ln_recv_checkbyte	: 1 ;  same for receiver
		ln_status		: 1 ; bit field with state of ln-subsystem
		ln_lack			: 1 ; byte to hold LACK-return value
		ln_priority		: 1 ; priority delay
	endc


; bit positions of variable "ln_status"
#define lnstat_wait_lack	ln_status, 0	; 1 if OPC_LONG_ACK may be received
#define lnstat_wait_slot_data	ln_status, 1	; 1 if OPC_RD_SL_DATA for known slot may be received
#define lnstat_wait_any_data	ln_status, 2	; 1 if OPC_RD_SL_DATA for any slot may be received
#define lnstat_wait_masterbusy	ln_status, 3	; 1 if OPC_BUSY may be received
#define lnstat_recv_activity	ln_status, 4	; set by receive interrupt routine to notify of loconet traffic
#define lnstat_adr_mismatch	ln_status, 5	; this gets set to 1 if a address is received, unequal to the stored one
#define lnstat_senderror	ln_status, 6	; error flag for send
#define lnstat_recverror	ln_status, 7	; error flag for receive

; error messages for determinating cause of receiption errors:

debug_code_reply_timeout	equ	1
debug_code_unknown_opcode	equ	2
debug_code_checkfailed		equ	3
debug_code_interbyte_timeout	equ	4
debug_code_badstartbit		equ	5
debug_code_badstopbit		equ	6



; LN_OPC.INC defines identifers for the opcodes and bit position in
; data structores that are transfered.

	include	"ln_opc.inc"


; Macros to access the Port-Pins that are involved with the
; physical Loconet Interface

SENDSPACE       macro
		 bsf    loconet_tx
		endm

SENDMARK        macro
		 bcf    loconet_tx
		endm

SKPSPACE        macro
		 btfsc  loconet_rx
		endm

SKPMARK         macro
		 btfss  loconet_rx
		endm
		
SKPCARRIER	macro
		 btfsc	loconet_cs
		endm
		
SKPNOCARRIER	macro
		 btfss	loconet_cs
		endm


; LN_RECV.INC and LN_SEND.INC contain the procedures that receive and send
; Loconet messages.

	include	"ln_recv.inc"
	include "ln_send.inc"


; INIT_LOCONET initializes the Loconet subsystem.

INIT_LOCONET	macro			; pins initialized in portdefs.inc
		 clrf	ln_status	; interrupt flag initialized
		endm			;  in main program


