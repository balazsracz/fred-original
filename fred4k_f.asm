;*******************************************************************
; fred/fred4k_f.asm
;
; Copyright: Stefan Bormann (s.bormann@tu-bs.de)
;            Martin Pischky (martin.pischky@fernuni-hagen.de)
;
; SEE COPYRGHT.TXT FOR COPYRIGHT INFO!!!
;
; History:  030411pi  new asm file for version 1.7 (FOURKEYS)
;
; This is the root assembler file for the SLNTHR alias FRED project
;
; ++** This version uses a 4MHz 16F84 **++
;                                /^\
;*******************************************************************


;*******************************************************************
#define MODE_NORMAL	; Normal Operation Version
#define FOURKEYS        ; version with 4 function keys
;*******************************************************************


        LIST    P=16F84, w=0, r=DEC

        INCLUDE <P16F84.INC>

        __CONFIG       _CP_OFF & _PWRTE_ON & _WDT_OFF & _XT_OSC
                                          	;configure programmer directive

OSC_CLK		equ	D'4000000'		; oscylator frequency


	include "fred_all.inc"

	end
