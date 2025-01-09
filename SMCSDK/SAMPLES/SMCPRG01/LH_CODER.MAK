# Microsoft Visual C++ generated build script - Do not modify

PROJ = LH_CODER
DEBUG = 1
PROGTYPE = 0
CALLER = 
ARGS = 
DLLS = 
D_RCDEFINES = -d_DEBUG
R_RCDEFINES = -dNDEBUG
ORIGIN = MSVC
ORIGIN_VER = 1.00
PROJPATH = E:\SMCSDK\SAMPLES\SMCPRG01\
USEMFC = 0
CC = cl
CPP = cl
CXX = cl
CCREATEPCHFLAG = 
CPPCREATEPCHFLAG = 
CUSEPCHFLAG = 
CPPUSEPCHFLAG = 
FIRSTC = LH_CODER.C  
FIRSTCPP =             
RC = rc
CFLAGS_D_WEXE = /nologo /G2 /W3 /Zi /AL /Od /D "_DEBUG" /FR /GA /Fd"LH_CODER.PDB"
CFLAGS_R_WEXE = /nologo /W3 /AM /O1 /D "NDEBUG" /FR /GA 
LFLAGS_D_WEXE = /NOLOGO /ONERROR:NOEXE /NOD /PACKC:61440 /CO /ALIGN:16 /STACK:10240
LFLAGS_R_WEXE = /NOLOGO /ONERROR:NOEXE /NOD /PACKC:61440 /ALIGN:16 /STACK:10240
LIBS_D_WEXE = oldnames libw commdlg shell olecli olesvr llibcew
LIBS_R_WEXE = oldnames libw commdlg shell olecli olesvr mlibcew
RCFLAGS = /nologo
RESFLAGS = /nologo
RUNFLAGS = 
DEFFILE = LH_CODER.DEF
OBJS_EXT = 
LIBS_EXT = ..\..\LIB\ST180VB.LIB 
!if "$(DEBUG)" == "1"
CFLAGS = $(CFLAGS_D_WEXE)
LFLAGS = $(LFLAGS_D_WEXE)
LIBS = $(LIBS_D_WEXE)
MAPFILE = nul
RCDEFINES = $(D_RCDEFINES)
!else
CFLAGS = $(CFLAGS_R_WEXE)
LFLAGS = $(LFLAGS_R_WEXE)
LIBS = $(LIBS_R_WEXE)
MAPFILE = nul
RCDEFINES = $(R_RCDEFINES)
!endif
!if [if exist MSVC.BND del MSVC.BND]
!endif
SBRS = LH_CODER.SBR


ST180VB_DEP = 

LH_CODER_DEP = e:\smcsdk\samples\smcprg01\lh_coder.h \
	e:\smcsdk\samples\smcprg01\lh_res.h


LH_CODVC_RCDEP = e:\smcsdk\samples\smcprg01\lh_res.h


all:	$(PROJ).EXE $(PROJ).BSC

LH_CODER.OBJ:	LH_CODER.C $(LH_CODER_DEP)
	$(CC) $(CFLAGS) $(CCREATEPCHFLAG) /c LH_CODER.C

LH_CODVC.RES:	LH_CODVC.RC $(LH_CODVC_RCDEP)
	$(RC) $(RCFLAGS) $(RCDEFINES) -r LH_CODVC.RC


$(PROJ).EXE::	LH_CODVC.RES

$(PROJ).EXE::	LH_CODER.OBJ $(OBJS_EXT) $(DEFFILE)
	echo >NUL @<<$(PROJ).CRF
LH_CODER.OBJ +
$(OBJS_EXT)
$(PROJ).EXE
$(MAPFILE)
c:\wa\msvc\lib\+
c:\wa\msvc\mfc\lib\+
..\..\LIB\ST180VB.LIB+
$(LIBS)
$(DEFFILE);
<<
	link $(LFLAGS) @$(PROJ).CRF
	$(RC) $(RESFLAGS) LH_CODVC.RES $@
	@copy $(PROJ).CRF MSVC.BND

$(PROJ).EXE::	LH_CODVC.RES
	if not exist MSVC.BND 	$(RC) $(RESFLAGS) LH_CODVC.RES $@

run: $(PROJ).EXE
	$(PROJ) $(RUNFLAGS)


$(PROJ).BSC: $(SBRS)
	bscmake @<<
/o$@ $(SBRS)
<<
