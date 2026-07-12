;*******************************************************************
; fred/fred_c.asm
;
; Copyright: Stefan Bormann (s.bormann@tu-bs.de)
;            Martin Pischky (martin.pischky@fernuni-hagen.de)
;
; SEE COPYRGHT.TXT FOR COPYRIGHT INFO!!!
;
; History:  9804xxsb  Split slnthr.asm into fred_c.asm, fred_f.asm
;                     and fred_all.inc
;
; This is the root assembler file for the SLNTHR alias FRED project
;
; ++** This version uses a 4MHz 16 C 84 **++
;                                 /^\
;*******************************************************************


;*******************************************************************
#define MODE_NORMAL	; Normal Operation Version
;*******************************************************************


        LIST    P=16C84, w=0, r=DEC

        INCLUDE <P16C84.INC>

        __CONFIG       _CP_OFF & _PWRTE_ON & _WDT_OFF & _XT_OSC
                                          	;configure programmer directive

OSC_CLK		equ	D'4000000'		; oscylator frequency


	include "fred_all.inc"

	end

