#define STRICT

#include <windows.h>
#include <io.h>
#include <stdio.h>
#include <string.h>
#include <commdlg.h>
#include "lh_res.h"

long FAR PASCAL _export MainWndProc(HWND, unsigned, WORD, LONG);
BOOL InitApplication(HINSTANCE);
