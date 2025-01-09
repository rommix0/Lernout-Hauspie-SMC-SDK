/*
 *   Project:		L&H ACM wrapper 3.00
 *   Workfile:		lh_reg.h
 *   Authors:		Alfred Wiesen
 *   Created:		30 August 1995
 *   Last update:	29 May 1996
 *   DLL Version:	3.00
 *   Revision:
 *   Comment:           This file contains all information needed to access
 *			the services of the LH_CODEC ACM Wrapper.
 *			See the application notes for more info.
 *
 *	(C) Copyright 1993-96 Lernout & Hauspie Speech Products N.V. (TM)
 *	All rights reserved. Company confidential.
 */

#ifndef __LH_REG__

#define __LH_REG__

#define WAVE_FORMAT_LH_CODEC 0x1100

#define MM_LERNOUT_AND_HAUSPIE 97
#define MM_LERNOUT_AND_HAUSPIE_LHCODEC_ACM 1

//
//  LHCODEC structure definitions
//
//      for WAVE_FORMAT_LH_CODEC   (0x1100)
//
//

typedef struct lh_codecwaveformat_tag {
	WAVEFORMATEX    wfx;
	WORD            wFormatSubTag;
	WORD            wPCMBlockAlign;
	DWORD           dwDLLVersion;
} LHCODECWAVEFORMAT;
typedef LHCODECWAVEFORMAT       *PLHCODECWAVEFORMAT;
typedef LHCODECWAVEFORMAT NEAR *NPLHCODECWAVEFORMAT;
typedef LHCODECWAVEFORMAT FAR  *LPLHCODECWAVEFORMAT;

#endif	//__LH_REG__