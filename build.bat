@REM $Log: build.bat,v $
@REM Revision 1.5  2008/02/26 09:08:26  pischky
@REM version -none-  (added support for MPAS 5.06 and .cvsignore)
@REM
@REM Revision 1.4  2004/09/14 20:28:50  pischky
@REM version 1.7 (conditional assembly for variants)
@REM
@REM Revision 1.3  2004/09/14 20:07:43  pischky
@REM version 1.5 (improved network code)
@REM
@REM Revision 1.2  2004/09/14 19:38:32  pischky
@REM version 1.4 (first stable release)
@REM
@REM Revision 1.1  2004/09/13 19:43:17  pischky
@REM build file for version 1.0
@REM
@REM ***** set installation directory of assembler *****
@REM 
@REM tested with MPASM v02.14 on W2KSP4:
SET MPASM_HOME=D:\Programme\microchip\asm21500
@REM tested with MPASM v5.06 on W2KSP4:
@REM SET MPASM_HOME=D:\Programme\microchip\MPASM Suite
@REM 
@REM ***** select assembler (depends on your os) *******
@REM 
@REM SET MPASM=%MPASM_HOME%\mpasm.exe
@REM SET MPASM=%MPASM_HOME%\mpasm_dp.exe
SET MPASM=%MPASM_HOME%\mpasmwin.exe
@REM
@REM ***** command line switches *****
@REM
SET MPASM_SW=/q
@REM
@REM ***************************************************
if "%1" == "clean" GOTO :clean
if "%1" == "distclean" GOTO :distclean
"%MPASM%" %MPASM_SW% fred4k_f.asm
"%MPASM%" %MPASM_SW% fred_f.asm
"%MPASM%" %MPASM_SW% fred_c.asm
"%MPASM%" %MPASM_SW% ftest_f.asm
"%MPASM%" %MPASM_SW% ftest_c.asm
GOTO :fin
:distclean
ERASE *.HEX
:clean
ERASE *.COD
ERASE *.ERR
ERASE *.LST
:fin
