#pragma once

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#if defined(_DEBUG) || defined (DEBUG)
#define _CRTDBG_MAP_ALLOC
#define _CRTDBG_MAP_ALLOC_NEW
#endif

#ifdef VLD_FORCE_ENABLE
# include <vld.h>

#include <stdlib.h>
#include <crtdbg.h>
#endif

#define CUSTOM_MIN(a,b) (((a)<(b))?(a):(b))
#define CUSTOM_MAX(a,b) (((a)>(b))?(a):(b))

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit
//#define _AFX_NO_MFC_CONTROLS_IN_DIALOGS         // remove support for MFC controls in dialogs

// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxmt.h>
#include <wininet.h>
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT