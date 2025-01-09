//////////////////////////////////////////////////////////////////////////////////
//                                                                              //
//  LH_Coder.exe : Low level coding DLLs calling application.                   //
//                                                                         	//
//  Copyright (c) 1993-1996 Lernout & Hauspie Speech Products N.V. (TM)         //
//  All rights reserved. Company confidential.                             	//
//                                                                         	//
//  Author  	: Alfred Wiesen                                                	//
//  Orig.date  	: June 25, 1995                                                 //
//  Last update	: May 28, 1996                                                  //
//  Version	: Streamtalk Product Line test.		                        //
//                                                                         	//
//////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// The files needed to build this program are . . .
//
// lh_coder.h				Header file for application
// lh_res.h				Header file for application and resources
// lh_coder.c				Source file for application
// lh_res.rc				Resource file for application
//
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Contains the necessary headers and definitions for this program
// -----------------------------------------------------------------------------
#include "lh_coder.h"

// -----------------------------------------------------------------------------
// Coder specific files and defines
// Change these settings to access to another algorithms :
// - include correct header file with coding functions declarations
// - SAMPFREQ			The sampling frequency
// - CODERINITIALIZE		The coding initialization function
// - ENCODE			The encoding function
// - CODERTERMINATE		The coding termination function
// - DECODERINITIALIZE		The decoding initialization function
// - DECODE			The decoding function
// - DECODERTERMINATE		The decoding termination function
// - GETCODECINFOEX		The GetCodecInfoEx function for the selected coder
// - MAXPCMSIZE			The maximum PCM data buffer size (used for memory allocation)
// - MAXCODESIZE		The maximumu coded data buffer size (used for memory allocation)
// - PCMFORMAT			The format of PCM. This can be LINEAR_PCM_16_BIT or LINEAR_PCM_8_BIT
//				Caution! No verification of the original wav file is performed by
//				this sample application. If the values given for SAMPFREQ and PCMFORMAT
//				are wrong, this could result in crashing the coder.
// - INISECTION			The section in the LH_Coder.ini file were the MRU files are stored
// -----------------------------------------------------------------------------
#include "st180VB.h"
#define SAMPFREQ 					8000
#define CODERINITIALIZE(dwFlags) 			ST180VB_Open_Coder(dwFlags)
#define ENCODE(hAccess,lpSrc,lpSrcSize,lpDst,lpDstSize) ST180VB_Encode(hAccess,lpSrc,lpSrcSize,lpDst,lpDstSize)
#define CODERTERMINATE(hAccess) 			ST180VB_Close_Coder(hAccess)
#define DECODERINITIALIZE(dwFlags)			ST180VB_Open_Decoder(dwFlags)
#define DECODE(hAccess,lpSrc,lpSrcSize,lpDst,lpDstSize) ST180VB_Decode(hAccess,lpSrc,lpSrcSize,lpDst,lpDstSize)
#define DECODERTERMINATE(hAccess) 			ST180VB_Close_Decoder(hAccess)
#define GETCODECINFOEX(lpCIEx,dwSize)			ST180VB_GetCodecInfoEx(lpCIEx,dwSize)
#define MAXPCMSIZE					1000
#define MAXCODESIZE					100
#define PCMFORMAT					LINEAR_PCM_16_BIT
#define INISECTION					"StreamTalk180VB"

#define __HEADER__		// Defining __header__ will skip the WAV header from the original file
				// and write it down in the decoded file.

// -----------------------------------------------------------------------------
// Global Variables
// -----------------------------------------------------------------------------
char szOriginalFileName[256];	// The name of the original file
char szCodedFileName[256];	// The name of the coded file
char szDecodedFileName[256];	// The name of the decoded file
char szStatus[256];		// Status string
DWORD dwOriginalFileSize;	// Size of the original file
char szCodingLoad[256];		// String displaying the CPU load for coding
char szDecodingLoad[256];	// String displaying the CPU load for decoding
HINSTANCE hInst;		// Current instance
DWORD dwCodeSize,dwPCMSize;	// Size of PCM and Coded buffers as returned by GetCodecInfoEx
char szExplanationText[80];	// Coder description as returned by GetCodecInfoEx
CODECINFOEX CodecInfoEx;	// Codec information structure

#ifdef __HEADER__
char HEADER[46];		// Temp buffer for storing the WAV header
#endif

// -----------------------------------------------------------------------------
// GetFileSize : returns the size of a given file
// -----------------------------------------------------------------------------
DWORD GetFileSize(char *FileName)
{
	FILE *ftemp;
        DWORD dwTemp;

	if ((ftemp=fopen(FileName,"rb"))==NULL)
        	return 0L;
	dwTemp=(DWORD)filelength(fileno(ftemp));
	fclose(ftemp);
        return (dwTemp);
}

// -----------------------------------------------------------------------------
// GetLastUsed : read a LH_Coder.ini file for last files used
// -----------------------------------------------------------------------------
void GetLastUsed(void)
{
	GetPrivateProfileString(INISECTION, "OriginalFile","", szOriginalFileName, sizeof(szOriginalFileName), "lh_coder.ini");
	GetPrivateProfileString(INISECTION, "CodedFile","", szCodedFileName, sizeof(szCodedFileName), "lh_coder.ini");
	GetPrivateProfileString(INISECTION, "DecodedFile","", szDecodedFileName, sizeof(szDecodedFileName), "lh_coder.ini");
	GetPrivateProfileString(INISECTION, "CodingLoad","[Unknown]", szCodingLoad, sizeof(szCodingLoad), "lh_coder.ini");
	GetPrivateProfileString(INISECTION, "DecodingLoad","[Unknown]", szDecodingLoad, sizeof(szCodingLoad), "lh_coder.ini");
}

// -----------------------------------------------------------------------------
// WriteLastUsed : read a LH_Coder.ini file for last files used
// -----------------------------------------------------------------------------
void WriteLastUsed(void)
{
	WritePrivateProfileString(INISECTION, "OriginalFile", szOriginalFileName, "lh_coder.ini");
	WritePrivateProfileString(INISECTION, "CodedFile", szCodedFileName, "lh_coder.ini");
	WritePrivateProfileString(INISECTION, "DecodedFile", szDecodedFileName, "lh_coder.ini");
	WritePrivateProfileString(INISECTION, "CodingLoad", szCodingLoad, "lh_coder.ini");
	WritePrivateProfileString(INISECTION, "DecodingLoad", szDecodingLoad, "lh_coder.ini");
}

// -----------------------------------------------------------------------------
// GetFileName gets the name of a file using a common dialog.
// The mode parameter selects which file is being searched :
//	0 = original file
//	1 = coded file
//	2 = decoded file 
// -----------------------------------------------------------------------------
void GetFileName( HWND hWnd, int mode )
{
	OPENFILENAME ofnTemp;
	DWORD Errval;	// Error value
	char buf[5];	// Error buffer
	char Errstr[50]="GetOpenFileName returned Error #";
	char szTemp1[]="Wave files (*.wav)\0*.wav\0All Files (*.*)\0*.*\0";
	char szTemp2[]="Coded files (*.cod)\0*.cod\0All Files (*.*)\0*.*\0";
	#ifdef __HEADER__
	char szTemp3[]="Decoded files (*.wav)\0*.wav\0All Files (*.*)\0*.*\0";
	#else
	char szTemp3[]="Decoded files (*.dec)\0*.dec\0All Files (*.*)\0*.*\0";
        #endif

	char *szTemp;
	char szName[256];

	switch (mode)
        {
	case 0 : // Open original file
                 strcpy( szName, "*.wav");
		 szTemp=szTemp1;
		 ofnTemp.lpstrTitle = "Select original file";	// Title for dialog
	 	 ofnTemp.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;
		 break;
	case 1 : // Open coded file
		 strcpy( szName, "*.cod");
		 szTemp=szTemp2;
		 ofnTemp.lpstrTitle = "Select coded file";	// Title for dialog
		 ofnTemp.Flags = OFN_HIDEREADONLY;
		 break;
	case 2 : // Open decoded file
		 #ifdef __HEADER__
		 strcpy( szName, "*.wav");
                 #else
		 strcpy( szName, "*.dec");
                 #endif
		 szTemp=szTemp3;
		 ofnTemp.lpstrTitle = "Select decoded file";	// Title for dialog
		 ofnTemp.Flags = OFN_HIDEREADONLY;
		 break;
	}

        // Fill the openfile structure
	ofnTemp.lStructSize = sizeof( OPENFILENAME );
	ofnTemp.hwndOwner = hWnd;	// An invalid hWnd causes non-modality
	ofnTemp.hInstance = 0;
	ofnTemp.lpstrFilter = (LPSTR)szTemp;
	ofnTemp.lpstrCustomFilter = NULL;
	ofnTemp.nMaxCustFilter = 0;
	ofnTemp.nFilterIndex = 1;
	ofnTemp.lpstrFile = (LPSTR)szName;	// Stores the result in this variable
	ofnTemp.nMaxFile = sizeof( szName );
	ofnTemp.lpstrFileTitle = NULL;
	ofnTemp.nMaxFileTitle = 0;
	ofnTemp.lpstrInitialDir = NULL;
	ofnTemp.nFileOffset = 0;
	ofnTemp.nFileExtension = 0;
	ofnTemp.lpstrDefExt = "*";
	ofnTemp.lCustData = NULL;
	ofnTemp.lpfnHook = NULL;
	ofnTemp.lpTemplateName = NULL;

	if(GetOpenFileName( &ofnTemp ) != TRUE)
	{
		Errval=CommDlgExtendedError();
		if(Errval!=0)	// 0 value means user selected Cancel
		{
			sprintf(buf,"%ld",Errval);
			strcat(Errstr,buf);
			MessageBox(hWnd,Errstr,"WARNING",MB_OK|MB_ICONSTOP);
		}
                return;
	}
	switch (mode)
        {
	case 0 : // Open original file
		 strcpy( szOriginalFileName, szName);
		 dwOriginalFileSize=GetFileSize(szOriginalFileName);
		 break;
	case 1 : // Open coded file
		 strcpy( szCodedFileName, szName);
		 break;
	case 2 : // Open decoded file
		 strcpy( szDecodedFileName, szName);
		 break;
	}

	InvalidateRect( hWnd, NULL, TRUE );	// Repaint to display the new name
}

// -----------------------------------------------------------------------------
// Application initialization by registration of window class
// -----------------------------------------------------------------------------
BOOL InitApplication(HINSTANCE hInstance)
{
	WNDCLASS  wc;

	// Fill in window class structure with parameters that describe the
	// main window.
	wc.style = CS_HREDRAW | CS_VREDRAW;	// Class style(s).
	wc.lpfnWndProc = (long (FAR PASCAL*)())MainWndProc;	// Function to retrieve messages for
														// windows of this class.
	wc.cbClsExtra = 0;	// No per-class extra data.
	wc.cbWndExtra = 0;	// No per-window extra data.
	wc.hInstance = hInstance;	// Application that owns the class.
	wc.hIcon = LoadIcon(hInstance, "LH_ICON");
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = "LH_CODERMENU";	// Name of menu resource in .RC file.
	wc.lpszClassName = "LH_CODER";	// Name used in call to CreateWindow.

	// Register the window class and return success/failure code.
	return (RegisterClass(&wc));
}

// -----------------------------------------------------------------------------
// Create and show window
// -----------------------------------------------------------------------------
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND	hWnd;	// Main window handle.
	char	S[256];

	// Save the instance handle in static variable
	hInst = hInstance;
	// Create a main window for this application instance.
	hWnd = CreateWindow(
		"LH_CODER",
		WINDOWCAPTION,			// Text for window title bar.
		WS_OVERLAPPEDWINDOW,		// Window style.
		CW_USEDEFAULT, 			// Default horizontal position.
		CW_USEDEFAULT,	   	   	// Default vertical position.
		600,	      			// Default width.
		300, 		   		// Default height.
		NULL,		   		// Overlapped windows have no parent.
		NULL,	   			// Use the window class menu.
		hInstance,  			// This instance owns this window.
		NULL				// Pointer not needed.
	);

	// If window could not be created, return "failure"
	if (!hWnd) return (FALSE);

        // Get here the information about the given codec
	GETCODECINFOEX((LPCODECINFOEX)&CodecInfoEx,sizeof(CODECINFOEX));
        // Display that info in a message box
	sprintf(S,"CodecInfo :\n   PCMSize=%d\n   CodedSize=%d\n   BitsPerSample=%d\n   SampleRate=%ld\n   SubTag=%d\n   String=%s\n   DLLVersion=%d.%02d",
		CodecInfoEx.wInputBufferSize,CodecInfoEx.wCodedBufferSize,
		CodecInfoEx.wInputDataFormat,CodecInfoEx.dwInputSampleRate,
		CodecInfoEx.wFormatSubTag,CodecInfoEx.szFormatSubTagName,
		HIWORD(CodecInfoEx.dwDLLVersion),LOWORD(CodecInfoEx.dwDLLVersion));
	MessageBox(hWnd,S,"INFO",MB_OK);
        // Save the values we need to perform the coding
	dwPCMSize=CodecInfoEx.wInputBufferSize;
	dwCodeSize=CodecInfoEx.wCodedBufferSize;
	strcpy(szExplanationText,CodecInfoEx.szFormatSubTagName);

	// Make the window visible; update its client area; and return "success"
	ShowWindow(hWnd, nCmdShow);		// Show the window
	UpdateWindow(hWnd);			// Sends WM_PAINT message
	return (TRUE);				// Returns the value from PostQuitMessage
}

// -----------------------------------------------------------------------------
// Displays main information on the window 
// -----------------------------------------------------------------------------
void RepaintWindow(HWND hWnd)
{
	HDC hdc;
	PAINTSTRUCT ps;	// Paint Struct for BeginPaint call
	char S[256];

	hdc=BeginPaint(hWnd,&ps);

	sprintf(S,"Original file name :");
	TextOut(hdc, 10, 10, S, strlen(S));
	if (szOriginalFileName[0]==0) // empty string
		sprintf(S,"[No file selected]");
        else
  		sprintf(S,szOriginalFileName);
	TextOut(hdc, 200, 10, S, strlen(S));
	sprintf(S,"Coded file name :");
	TextOut(hdc, 10, 25, S, strlen(S));
	if (szCodedFileName[0]==0) // empty string
		sprintf(S,"[No file selected]");
        else
  		sprintf(S,szCodedFileName);
	TextOut(hdc, 200, 25, S, strlen(S));
	sprintf(S,"Decoded file name :");
	TextOut(hdc, 10, 40, S, strlen(S));
	if (szDecodedFileName[0]==0) // empty string
		sprintf(S,"[No file selected]");
        else
		sprintf(S,szDecodedFileName);
	TextOut(hdc, 200, 40, S, strlen(S));
	sprintf(S,"Original file size :");
	TextOut(hdc, 10, 80, S, strlen(S));
	if (szOriginalFileName[0]==0) // empty string
		sprintf(S,"[No file selected]");
        else
		sprintf(S,"%ld bytes - %5.2f seconds",dwOriginalFileSize,(float)dwOriginalFileSize/(2.0*SAMPFREQ));
	TextOut(hdc, 200, 80, S, strlen(S));

	sprintf(S,"Coding computation load :");
	TextOut(hdc, 10, 120, S, strlen(S));
	sprintf(S,szCodingLoad);
	TextOut(hdc, 200, 120, S, strlen(S));
	sprintf(S,"Decoding computation load :");
	TextOut(hdc, 10, 135, S, strlen(S));
	sprintf(S,szDecodingLoad);
        TextOut(hdc, 200, 135, S, strlen(S));

	sprintf(S,szExplanationText);
	TextOut(hdc, 10, 210, S, strlen(S));

	EndPaint(hWnd,&ps);

}

// -----------------------------------------------------------------------------
// Main coding processing function
// - open the original and coded files - initialize coder
// - perform the main coding loop with progress control
// - terminate coding and close files
// - compute computation load
// -----------------------------------------------------------------------------
void ProcessCoding(HWND hWnd)
{
	BYTE InputUncoded[MAXPCMSIZE],OutputCoded[MAXCODESIZE];		// Buffers for speech data
	WORD InputUncodedSize,OutputCodedSize;		// Size of buffers
	FILE *InputUncodedFile,*OutputCodedFile;	// Input and output files
	DWORD t;					// Computation time
	HANDLE hAccess;					// Handle for accessing coding engine
	LH_ERRCODE ErrorCode;				// Coding engine error codes
	DWORD count=0;					// Counter for progress display
	char S[256];					// String for progress display
	HDC hdc;
	MSG msg;					// Message to implement message loop in function

        // First some error checkings
	if (szOriginalFileName[0]==0)	// empty string
	{
	   MessageBox(hWnd,"No original file selected.\nSelect one by using the File menu.", "L&H Coder Error", MB_OK | MB_ICONSTOP);
           return;
	}
	if (szCodedFileName[0]==0)	// empty string
	{
	   MessageBox(hWnd,"No coded file selected.\nSelect one by using the File menu.", "L&H Coder Error", MB_OK | MB_ICONSTOP);
           return;
	}
	if ((InputUncodedFile=fopen(szOriginalFileName,"rb"))==NULL)
	{
	   MessageBox(hWnd,"Could not open original file.", "L&H Coder Error", MB_OK | MB_ICONSTOP);
           return;
	}
	if ((OutputCodedFile=fopen(szCodedFileName,"wb"))==NULL)
	{
	   MessageBox(hWnd,"Could not open coded file.", "L&H Coder Error", MB_OK | MB_ICONSTOP);
	   return;
	}

	hAccess=CODERINITIALIZE(PCMFORMAT);	// Call initialization function with selected PCM format
	if (!hAccess)
	{
		MessageBox(hWnd,"Could not initialize coding engine.", "L&H Coder Error", MB_OK | MB_ICONSTOP);
		fcloseall();
		return;
	}

	hdc=GetDC(hWnd);
	sprintf(S,"Coding in progress : ");
	TextOut(hdc, 10, 175, S, strlen(S));

	t=0;					// Initialize start time

	#ifdef __HEADER__
	fread(HEADER,1,46,InputUncodedFile);	// Read WAV header of PCM file	
        #endif

	// Initialize correct PCM buffer size depending if data is 8-bit or 16-bit
	#if PCMFORMAT==LINEAR_PCM_16_BIT
	InputUncodedSize=dwPCMSize;		
	#else
	InputUncodedSize=dwPCMSize/2;
        #endif

	// Main loop of coding process
	while (fread((void*)&InputUncoded,sizeof(char),InputUncodedSize,InputUncodedFile))
	{
                // Update display of progress indicator
		count+=InputUncodedSize;

		// Set output coded buffer size to the value returned by GetCodecInfoEx
                // This must be done if we work with variable bit rates
		OutputCodedSize=dwCodeSize;

                // Mesure time before encoding
		t-=GetTickCount();
                // Call encoding function
		ErrorCode=ENCODE(hAccess,(LPBYTE)&InputUncoded,(LPWORD)&InputUncodedSize,
		    (LPBYTE)&OutputCoded,(LPWORD)&OutputCodedSize);
		// Mesure time after encoding
		t+=GetTickCount();
                // Check errors
		if (ErrorCode==LH_EBADARG)
		{
			MessageBox(hWnd,"Error running coder : bad arguments.", "L&H Coder Error", MB_OK | MB_ICONSTOP);
			fcloseall();
			return;
		}
		if (ErrorCode==LH_BADHANDLE)
		{
			MessageBox(hWnd,"Error running coder : bad handle.", "L&H Coder Error", MB_OK | MB_ICONSTOP);
			fcloseall();
			return;
		}
                // Write exact amount of coded data to file
		fwrite((void*)&OutputCoded,sizeof(char),OutputCodedSize,OutputCodedFile);

                // Display progress indicator
		sprintf(S,"%3d %%    ",count*100/dwOriginalFileSize);
		TextOut(hdc, 200, 175, S, strlen(S));

                // Return control to windows
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

        // Close coding engine
	if (CODERTERMINATE(hAccess))
	{
		MessageBox(hWnd,"Could not close coding engine.", "L&H Coder Error", MB_OK | MB_ICONSTOP);
	}
        // Close all files
        fcloseall();

        // Display ratio of CPU load
	#if PCMFORMAT==LINEAR_PCM_16_BIT
	sprintf(szCodingLoad,"%5.2f seconds - %5.2f real time",(float)t/1000.0,(float)t*2.0*SAMPFREQ/(1000.0*(float)dwOriginalFileSize));
        #else
	sprintf(szCodingLoad,"%5.2f seconds - %5.2f real time",(float)t/1000.0,(float)t*SAMPFREQ/(1000.0*(float)dwOriginalFileSize));
        #endif

	sprintf(S,"                                                       ");
	TextOut(hdc, 10, 175, S, strlen(S));
	ReleaseDC(hWnd,hdc);

	InvalidateRect( hWnd, NULL, TRUE );	// Repaint to display the new values
}

// -----------------------------------------------------------------------------
// Main decoding processing function
// - open the coded and decoded files - initialize decoder
// - perform the main decoding loop with progress control
// - terminate decoding and close files
// - compute computation load
// -----------------------------------------------------------------------------
void ProcessDecoding(HWND hWnd)
{
	BYTE InputCoded[MAXCODESIZE],OutputDecoded[MAXPCMSIZE];	// Buffers for speech data
	BYTE *InputPointer;				// This pointer indicates where the new coded data
        						// have to be read.
	WORD InputCodedSize,OutputDecodedSize;		// Size of buffers
        WORD CodesToRead;				// this value represents the number of new coded data bytes ot be read
	FILE *InputCodedFile,*OutputDecodedFile;	// Input and output files
	DWORD t;					// Computation time
	HANDLE hAccess;					// Handle for accessing coding engine
	LH_ERRCODE ErrorCode;				// Coding engine error codes
	DWORD count=0;					// Counter for progress display
	char S[256];					// String for progress display
	HDC hdc;
	MSG msg;					// Message to implement message loop in function
	short i;

        // First some error checkings
	if (szCodedFileName[0]==0)	// empty string
	{
	   MessageBox(hWnd,"No coded file selected.\nSelect one by using the File menu.", "L&H Coder Error", MB_OK | MB_ICONSTOP);
           return;
	}
	if (szDecodedFileName[0]==0)	// empty string
	{
	   MessageBox(hWnd,"No decoded file selected.\nSelect one by using the File menu.", "L&H Coder Error", MB_OK | MB_ICONSTOP);
           return;
	}
	if ((InputCodedFile=fopen(szCodedFileName,"rb"))==NULL)
	{
	   MessageBox(hWnd,"Could not open coded file.", "L&H Coder Error", MB_OK | MB_ICONSTOP);
           return;
	}
	if ((OutputDecodedFile=fopen(szDecodedFileName,"wb"))==NULL)
	{
	   MessageBox(hWnd,"Could not open coded file.", "L&H Coder Error", MB_OK | MB_ICONSTOP);
	   return;
	}

	hAccess=DECODERINITIALIZE(PCMFORMAT);	// Call initialization function with selected output PCM format
	if (!hAccess)
	{
		MessageBox(hWnd,"Could not initialize decoding engine.", "L&H Coder Error", MB_OK | MB_ICONSTOP);
		fcloseall();
		return;
	}

	hdc=GetDC(hWnd);
	sprintf(S,"Decoding in progress : ");
	TextOut(hdc, 10, 175, S, strlen(S));

	t=0;			// Initialize start time

	// Initialisation input data pointer and number of codes to read.
	InputPointer=InputCoded;
	CodesToRead=dwCodeSize;
        // Initialize size of output PCM buffers depending if 8 or 16-bit data
	#if PCMFORMAT==LINEAR_PCM_16_BIT
	OutputDecodedSize=dwPCMSize;
	#else
	OutputDecodedSize=dwPCMSize/2;
        #endif

	// Write header to output file
	// this is only approximate since it writes to the decoded file the same wave header
	// as in the original file. This would be no problem if original and decoded files
	// have exactely the same sizes, but due to buffering of coding, the decoded file
	// could be a little bit smaller than the original (the coder will process a buffer
	// only if it has been filled +- 20 ms of data; if not, the end of the original
	// file is not processed).
	// Most of WAV play programs do not really care about a small difference of 20 ms
	// of data, but some others will request to read the exact amount of data as stated
	// in the WAV header, and then will return a 'corrupt file' error if there is even a
	// little difference like the one created here.
	// the proper way to handle this is to use the mmio functions of mmsystem.dll to
	// read and write in WAV files, but this is not the aim of this sample code. 
	#ifdef __HEADER__
	fwrite(HEADER,1,46,OutputDecodedFile);
	#endif

	// Main loop of coding process
	while (fread((void*)InputPointer,sizeof(char),CodesToRead,InputCodedFile)==CodesToRead)
	{
		// Update display of progress indicator
		count+=OutputDecodedSize;

		// Set input coded buffer size to the value returned by GetCodecInfoEx
                // This must be done if we work with variable bit rates
		InputCodedSize=dwCodeSize;

		// Mesure time before decoding
		t-=GetTickCount();
		// Call encoding function
		ErrorCode=DECODE(hAccess,(LPBYTE)&InputCoded,(LPWORD)&InputCodedSize,
		    (LPBYTE)&OutputDecoded,(LPWORD)&OutputDecodedSize);
		// Mesure time after decoding
		t+=GetTickCount();
		// Check errors
		if (ErrorCode==LH_EBADARG)
		{
			MessageBox(hWnd,"Error running decoder : bad arguments.", "L&H Coder Error", MB_OK | MB_ICONSTOP);
			fcloseall();
			return;
		}
		if (ErrorCode==LH_BADHANDLE)
		{
			MessageBox(hWnd,"Error running decoder : bad handle.", "L&H Coder Error", MB_OK | MB_ICONSTOP);
			fcloseall();
			return;
		}
                // write exact amount of decoded data to disk
		fwrite((void*)&OutputDecoded,sizeof(char),OutputDecodedSize,OutputDecodedFile);

		// Process update of coded data buffer
		// 1. If not all coded data have been used
                // This management is only needed if we work with variable bit rate coders
		if (InputCodedSize<dwCodeSize)
		{
                   // Save unused data by copying them to the beginning of the buffer
		   for (i=0;i<(dwCodeSize-InputCodedSize);i++)
		      InputCoded[i]=InputCoded[InputCodedSize+i];
                   // Set read pointer after these data
		   InputPointer=&InputCoded[dwCodeSize-InputCodedSize];
                   // Set number of codes to be read to fill the buffer
		   CodesToRead=InputCodedSize;
		}
		else
		// 2. If all coded data have been used
		{
		   InputPointer=InputCoded;
		   CodesToRead=dwCodeSize;
                }

                // Update progress control display
		sprintf(S,"%3d %%    ",count*100/dwOriginalFileSize);
		TextOut(hdc, 200, 175, S, strlen(S));

		// Return control to windows
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

        // Close decoding engine
	if (DECODERTERMINATE(hAccess))
	{
		MessageBox(hWnd,"Could not terminate decoding engine.", "L&H Coder Error", MB_OK | MB_ICONSTOP);
	}
        // Close all files
        fcloseall();

        // Display ratio of CPU load
	#if PCMFORMAT==LINEAR_PCM_16_BIT
	sprintf(szDecodingLoad,"%5.2f seconds - %5.2f real time",(float)t/1000.0,(float)t*2.0*SAMPFREQ/(1000.0*(float)dwOriginalFileSize));
	#else
	sprintf(szDecodingLoad,"%5.2f seconds - %5.2f real time",(float)t/1000.0,(float)t*SAMPFREQ/(1000.0*(float)dwOriginalFileSize));
        #endif

	sprintf(S,"                                                       ");
	TextOut(hdc, 10, 175, S, strlen(S));
	ReleaseDC(hWnd,hdc);

	InvalidateRect( hWnd, NULL, TRUE );	// Repaint to display the new values
}

// -----------------------------------------------------------------------------
// Main message processing function.
// -----------------------------------------------------------------------------
long FAR PASCAL _export MainWndProc(HWND hWnd, unsigned message,
									WORD wParam, LONG lParam)
{
	RECT rTemp;     // Client are needed by DrawText()
	HDC hdc;        // HDC for Window
	PAINTSTRUCT ps;	// Paint Struct for BeginPaint call

	switch (message) {
	case WM_CREATE: // Initialize Global vars
			GetLastUsed();				// Initialize display variables and file names
			if (szOriginalFileName[0]!=0)
				dwOriginalFileSize=GetFileSize(szOriginalFileName);
		return NULL;

	case WM_PAINT:
		RepaintWindow(hWnd);
		break;

	case WM_COMMAND:	// message: command from application menu
		switch(wParam)
		{
			case CM_EXIT:
					DestroyWindow(hWnd);
				break;

			case CM_ORIFILE:
					GetFileName(hWnd,0);
				break;

			case CM_CODEDFILE:
					GetFileName(hWnd,1);
				break;

			case CM_DECODEDFILE:
					GetFileName(hWnd,2);
				break;

			case CM_ENCODE:
					ProcessCoding(hWnd);
				break;

			case CM_DECODE:
					ProcessDecoding(hWnd);
				break;

			case CM_ABOUT:
					MessageBox(hWnd,LH_CODERAbout,"About L&H Coder",MB_OK);
				break;

			default:
				break;
		}
		break;

	case WM_QUIT:
	case WM_DESTROY:	// message: window being destroyed
			WriteLastUsed();
			PostQuitMessage(0);
		break;

	default:			// Passes it on if unproccessed
		return (DefWindowProc(hWnd, message, wParam, lParam));
	}
	return (NULL);
}

// -----------------------------------------------------------------------------
// Main : register window class, create window and implement message loop.
// -----------------------------------------------------------------------------
#pragma argsused
int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
				   LPSTR lpCmdLine, int nCmdShow)
{
	MSG msg;

	if (!hPrevInstance)				// Other instances of app running?
		if (!InitApplication(hInstance))	// Register window class
			return (FALSE);			// Exits if unable to initialize

	// Create and show window
	if (!InitInstance(hInstance, nCmdShow))
		return (FALSE);

	// Main message loop
	while (GetMessage(&msg,	NULL,NULL,NULL))
	{
		TranslateMessage(&msg);	// Translates virtual key codes
		DispatchMessage(&msg);	// Dispatches message to window
	}
	return (msg.wParam);	// Returns the value from PostQuitMessage
}

// -----------------------------------------------------------------------------
