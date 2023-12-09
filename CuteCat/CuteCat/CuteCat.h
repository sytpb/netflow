// CuteCat.h : main header file for the CUTECAT application
//

#if !defined(AFX_CUTECAT_H__53A069A0_12FA_41F4_B0C5_509509E46C02__INCLUDED_)
#define AFX_CUTECAT_H__53A069A0_12FA_41F4_B0C5_509509E46C02__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CCuteCatApp:
// See CuteCat.cpp for the implementation of this class
//

class CCuteCatApp : public CWinApp
{
public:
	CCuteCatApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCuteCatApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CCuteCatApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CUTECAT_H__53A069A0_12FA_41F4_B0C5_509509E46C02__INCLUDED_)
