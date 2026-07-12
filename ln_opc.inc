;********************************************************************
; LN_OPC.INC
;
; Authors: - Digitrax
;          - Stefan Bormann (s.bormann@tu-bs.de)
;
; History: - 9712XXsb convertet lnpe1.doc to this file
;          - 971227sb added identifers for bit positions in slot 
;                     bytes
;          - 980215sb added OPC_DEBUG
;
; This file in contains the definition of the Opcodes of all
; Loconet messages.
;
; It is extracted from:
; LocoNet Personal Use version definitions    1.0
; DRAFT DEFINITIONS October 16, 1997 SUBJECT TO REVISION
;********************************************************************

; Private Opcode:

OPC_DEBUG	equ	0xA8	; Debug #				NO
	;<0xA8>,<#1>,<#2>,<CHK>

;
;								 FOLLOW ON	RESPONSE
;								      MSG?	TYPE 
;**************************
;* 2 Byte MESSAGE opcodes *
;* FORMAT = <OPC>,<CKSUM> *
;**************************

OPC_IDLE	equ	0x85	;FORCE IDLE state,			NO
				;B'cast emerg. STOP
OPC_GPON	equ	0x83	;GLOBAL power ON request		NO
OPC_GPOFF	equ	0x82	;GLOBAL power OFF req			NO

OPC_BUSY	equ	0x81	;MASTER busy code, NUL			NO

;****************************************
;* 4 byte MESSAGE OPCODES               *
;* FORMAT = <OPC>,<ARG1>,<ARG2>,<CKSUM> *
;****************************************

OPC_LOCO_ADR	equ	0xBF	;REQ  loco ADR				YES	<E7> SLOT READ
	;<0xBF>,<0>,<ADR>,<CHK> REQ loco ADR
	;DATA return <E7>, is SLOT#,DATA that ADR was found in
	;IF ADR not found, MASTER puts ADR in FREE slot
	;and sends DATA/STATUS return <E7>
	;IF no FREE slot,Fail  LACK,0 is returned
	;[<B4>,<3F>,<0>,<CHK>]

OPC_SW_ACK	equ	0xBD	;					YES 	LACK
	;REQ SWITCH WITH acknowledge function (not DT200)
	;<0xBD>,<SW1>,<SW2>,<CHK> REQ SWITCH function 
	;<SW1>	=<0,A6,A5,A4-A3,A2,A1,A0>, 7 ls adr bits. 
	;A1,A0 select 1 of 4 input pairs in a DS54
	;<SW2>	=<0,0,DIR,ON-A10,A9,A8,A7>, Control bits and 4 MS adr bits.
	;DIR=1 for Closed/GREEN, =0 for Thrown/RED
	;ON=1 for Output ON, =0 FOR output OFF
	;response is <0xB4><3D><00> if DCS100 FIFO is full, command rejected
	;		<0xB4><3D><7F> if DCS100 accepted 

OPC_SW_STATE	equ	0xBC	;REQ state of SWITCH			YES	LACK
	;<0xBC>,<SW1>,<SW2>,<CHK> REQ state of SWITCH

OPC_RQ_SL_DATA	equ	0xBB	;Request SLOT DATA/status block		YES	<E7> SLOT READ
	;<0xBB>,<SLOT>,<0>,<CHK>  Request SLOT DATA/status block

OPC_MOVE_SLOTS	equ	0xBA	;MOVE slot SRC to DEST			YES	<E7> SLOT READ
	;<0xBA>,<SRC>,<DEST>,<CHK> Move SRC to DEST if SRC			or LACK  etc
	;is NOT IN_USE,   clr SRC
	;SPECIAL CASES:
	;If SRC=0 (DISPATCH GET), DEST=dont care,
	;  Return SLOT READ DATA of DISPATCH Slot
	;IF SRC=DEST (NULL move) then SRC=DEST is set to IN_USE, 
	;  if legal move
	;If DEST=0, is DISPATCH Put, mark SLOT as DISPATCH
	;RETURN slot status <0xE7> of DESTINATION slot DEST if move legal
	;RETURN Fail LACK code if illegal move <B4>,<3A>,<0>,<chk>
	;illegal to move to/from slots 120/127

OPC_LINK_SLOTS	equ	0xB9	;					YES	<E7> SLOT READ
	;LINK slot ARG1 to slot ARG2
	;<0xB9>,<SL1>,<SL2>,<CHK>  SLAVE slot SL1 to slot SL2
	;Master LINKER sets the SL_CONUP/DN flags appropriately
	;Reply is return of SLOT Status <0xE7>. Inspect to see result of Link
	;invalid Link will return Long Ack Fail <B4>,<39>,<0>,<CHK>

OPC_UNLINK_SLOTS	equ	0xB8	;				YES	<E7> SLOT READ
	; UNLINK slot ARG1 from slot ARG2
	;<0xB8>,<SL1>,<SL2>,<CHK>  UNLINK slot SL1 from SL2
	;UNLINKER executes unlink STRATEGY and returns new SLOT#
	; DATA/STATUS of unlinked LOCO . Inspect data to evaluate UNLINK

;CODES 0xB8 to 0xBF have responses

OPC_CONSIST_FUNC	equ	0xB6	;				NO
	;SET FUNC bits in a CONSIST uplink element
	;<0xB6>,<SLOT>,<DIRF>,<CHK> UP consist FUNC bits
	;NOTE this SLOT adr is considered in UPLINKED slot space

OPC_SLOT_STAT1	equ	0xB5	;WRITE slot stat1			NO
	;<0xB5>,<SLOT>,<STAT1>,<CHK> WRITE stat1

OPC_LONG_ACK	equ	0xB4	;Long acknowledge			NO
	;<0xB4>,<LOPC>,<ACK1>,<CHK> Long acknowledge
	;<LOPC> is COPY of OPCODE responding to (msb=0).
	;LOPC=0 (unused OPC) is also VALID fail code
	;<ACK1> is appropriate response code for the OPCode

OPC_INPUT_REP	equ	0xB2	; General SENSOR Input codes		NO
	; <0xB2>, <IN1>, <IN2>, <CHK>
	;<IN1>=<0,A6,A5,A4- A3,A2,A1,A0>, 7 ls adr bits.
	;  A1,A0 select 1 of 4 inputs pairs in a DS54
	;<IN2>=<0,X,I,L- A10,A9,A8,A7>	Report/status bits
	;  and 4 MS adr bits.
	;"I"=0 for DS54 "aux" inputs and 1 for "switch"
	;  inputs mapped to 4K SENSOR space.
	;  (This is effectively a least significant adr bit when
	;  using DS54 input configuration)
	;"L"=0 for input SENSOR now 0V (LO) , 1 for Input sensor >=+6V (HI)
	;"X"=1, control bit , 0 is RESERVED for future!

OPC_SW_REP	equ	0xB1	; Turnout SENSOR state REPORT		NO
	;<0xB1>,<SN1>,<SN2>,<CHK> SENSOR state REPORT
	;<SN1>=<0,A6,A5,A4- A3,A2,A1,A0>, 7 ls adr bits.
	;  A1,A0 select 1 of 4 input pairs in a DS54
	;
	;<SN2>=<0,1,I,L- A10,A9,A8,A7>	Report/status bits and 4 MS adr bits.
	;This <B1> opcode encodes INPUT LEVELS for turnout feedback
	;"I" =0 for "aux" inputs (normally not feedback),
	;    =1 for "switch" input used for turnout feedback for DS54 
	;       ouput/turnout # encoded by A0-A10
	;"L" = 0 for this input 0V (LO), 1= this input > +6V (HI)
	;
	;<SN2>=<0,0,C,T- A10,A9,A8,A7>	Report/status bits and 4 MS adr bits.
	;This <B1> opcode encodes current OUTPUT LEVELS
	;"C" =0 if "Closed" ouput line is OFF,
	;    =1 if "closed" output line is ON (sink current)
	;"T" =0 if "Thrown" output line is OFF,
	;    =1 if "thrown" output line is ON (sink I)

OPC_SW_REQ	equ	0xB0	;REQ SWITCH function			NO
	;<0xB0>,<SW1>,<SW2>,<CHK> REQ SWITCH function 
	;<SW1>	=<0,A6,A5,A4- A3,A2,A1,A0>, 7 ls adr bits.
	;  A1,A0 select 1 of 4 input pairs in a DS54
	;<SW2>	=<0,0,DIR,ON- A10,A9,A8,A7>, Control bits and 4 MS adr bits.
	;DIR=1 for Closed,/GREEN, =0 for Thrown/RED
	;ON=1 for Output ON, =0 FOR output OFF
	;
	;Note-,Immediate response of <0xB4><30><00> if command failed,
	;otherwise no response

;"A" CLASS codes

;CODES 0xA8 to 0xAF have responses

OPC_LOCO_SND	equ	0xA2	; SET SLOT sound functions		NO
OPC_LOCO_DIRF	equ	0xA1	; SET SLOT dir,F0-4 state		NO
OPC_LOCO_SPD	equ	0xA0	; SET SLOT speed			NO
	;e.g. <A0><SLOT#><SPD><CHK>


;******************************************************
;* 6 Byte MESSAGE OPCODES                             *
;* FORMAT = <OPC>,<ARG1>,<ARG2>,<ARG3>,<ARG4>,<CKSUM> *
;******************************************************
;<reserved>


;*******************************************************************
;* VARIABLE Byte MESSAGE OPCODES                                   *
;* FORMAT = <OPC>,<COUNT>,<ARG2>,<ARG3>,...,<ARG(COUNT-3)>,<CKSUM> *
;*******************************************************************

OPC_WR_SL_DATA	equ	0xEF	;					YES	LACK
	; WRITE SLOT DATA, 10 bytes
	;<0xEF>,<0E>,<SLOT#>,<STAT>,<ADR>,<SPD>,<DIRF>,<TRK>
	;<SS2>,<ADR2>,<SND>,<ID1>,<ID2>,<CHK> 
	; SLOT DATA WRITE, 10 bytes data /14 byte MSG

OPC_SL_RD_DATA	equ	0xE7	;					NO
	; SLOT DATA return, 10 bytes
	;<0xE7>,<0E>,<SLOT#>,<STAT>,<ADR>,<SPD>,<DIRF>,<TRK>    
	;<SS2>,<ADR2>,<SND>,<ID1>,<ID2>,<CHK> 
	; SLOT DATA READ, 10 bytes data /14 byte MSG
	;
	;NOTE; If STAT2.2=0 EX1/EX2 encodes an ID#,
	;[if STAT2.2=1 the STAT.3=0 means EX1/EX2 are ALIAS]
	;ID1/ID2 are two 7 bit values encoding a 14 bit unique DEVICE usage ID
	;00/00          -means NO ID being used
	;01/00 to 7F/01 -ID shows PC usage.Lo nibble is TYP PC#
	;                (PC can use hi values)
	;00/02 to 7F/03 -SYSTEM reserved
	;00/04 to 7F/7E -NORMAL throttle RANGE

OPC_PEER_XFER	equ	0xE5	;					NO
	; move 8 bytes PEER to PEER, SRC->DST
	;<0xE5>,<10>,<SRC>,<DSTL><DSTH>,<PXCT1>,<D1>,<D2>,<D3>,<D4>,
	;<PXCT2>,<D5>,<D6>,<D7>,<D8>,<CHK>
	;SRC/DST are 7 bit args. DSTL/H=0 is BROADCAST msg
	;SRC=0 is MASTER
	;SRC=0x70-0x7E are reserved
	;SRC=7F is THROTTLE msg xfer, <DSTL><DSTH> encode ID#,
	; <0><0> is THROT B'CAST
	;<PXCT1>=<0,XC2,XC1,XC0 - D4.7,D3.7,D2.7,D1.7>
	;XC0-XC2=ADR type CODE-0=7 bit Peer TO Peer adrs
	;			  1=><D1>is SRC HI,<D2>is DST HI
	;<PXCT2>=<0,XC5,XC4,XC3 - D8.7,D7.7,D6.7,D5.7>
	;XC3-XC5=data type CODE- 0=ANSI TEXT string,balance RESERVED

OPC_IMM_PACKET	equ	0xED	;SEND n-byte packet immediate		LACK
	;<0xED>,<0B>,<7F>,<REPS>,<DHI>,<IM1>,<IM2>,<IM3>,<IM4>,<IM5>,<CHK>
	;<DHI>=<0,0,1,IM5.7-IM4.7,IM3.7,IM2.7,IM1.7>
	;in <REPS> D4,5,6=#IM bytes,D3=0(reserved); D2,1,0=repeat CNT
	;Not limited MASTER then LACK=<B4>,<7D>,<7F>,<chk> if CMD ok
	;IF limited MASTER then  Lim  Masters respond with 
	;  <B4>,<7E>,<lim adr>,<chk>
	;If internal buffer BUSY/full respond with <B4>,<7D>,<0>,<chk>


;-----------------------------------------------------------------------------

;***********************************
; Bit positions in slot data bytes *
;***********************************


;--------------------------------- Slot status 1 -----------------------------

SL_SPURGE	equ	7	; 1=SLOT purge en, ALSO adrSEL 
				; (INTERNAL use only, not seen on NET!)

SL_CONUP	equ	6	; CONDN/CONUP: bit encoding-Control double 
SL_CONDN	equ	3	; linked Consist List
				; 11=LOGICAL MID CONSIST , Linked up AND down
				; 10=LOGICAL CONSIST TOP, Only linked downwards
				; 01=LOGICAL CONSIST SUB-MEMBER, Only linked upwards
				; 00=FREE locomotive, no CONSIST indirection/linking
				; ALLOWS "CONSISTS of CONSISTS". Uplinked 
				; means that Slot SPD number is now SLOT adr 
				; of SPD/DIR and STATUS of consist. i.e. is 
				; an Indirect pointer. This Slot has same 
				; BUSY/ACTIVE bits as TOP of Consist. 
				; TOP is loco with SPD/DIR for whole consist. 
				; (top of list). 

SL_BUSY		equ	5	; BUSY/ACTIVE: bit encoding for SLOT activity
SL_ACTIVE	equ	4	; 11=IN_USE loco adr in SLOT  	-REFRESHED
				; 10=IDLE   loco adr in SLOT  	-NOT refreshed
				; 01=COMMON loco adr IN SLOT    -refreshed
				; 00=FREE SLOT, no valid DATA	-not refreshed

SL_SPDEX	equ	2	; 3 BITS for Decoder TYPE encoding for this SLOT
SL_SPD14	equ	1	; 011=send 128 speed mode packets
SL_SPD28	equ	0	; 010=14 step MODE
				; 001=28 step. Generate Trinary packets for this Mobile ADR
				; 000=28 step/ 3 BYTE PKT regular mode
				; 111=128 Step decoder, Allow Advanced DCC consisting
				; 100=28 Step decoder, Allow Advanced DCC consisting

;--------------------------------------- SPD ---------------------------------

SPD_STOP	equ	0
SPD_ESTOP	equ	1

;-------------------------------------- DIRF ---------------------------------

SL_XCNT		equ	6	; reserved , set 0
SL_DIR		equ	5	; 1=loco direction FORWARD
SL_F0		equ	4	; 1=Directional lighting ON
SL_F4		equ	3	; 1=F4 ON
SL_F3		equ	2	; 1=F3 ON
SL_F2		equ	1	; 1=F2 ON
SL_F1		equ	0	; 1=F1 ON

;--------------------------------------- TRK ---------------------------------

SL_SND4		equ	3	; F8
SL_SND3		equ	2	; F7
SL_SND2		equ	1	; F6
SL_SND1		equ	0	; F5 1= SLOT Sound 1 function 1active (accessory 2)

;--------------------------------------- TRK ---------------------------------

				; D7-D4		Reserved
GTRK_PROG_BUSY	equ	3	; 1=Programming TRACK in this Master is BUSY.
GTRK_MLOK1	equ	2	; 1=This Master IMPLEMENTS LocoNet 1.1 capability0=Master is DT200
GTRK_IDLE	equ	1	; 0=TRACK is PAUSED, B'cast EMERG STOP.
GTRK_POWER	equ	0	; 1=DCC packets are ON in MASTER, Global POWER up

;--------------------------------------
