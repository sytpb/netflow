// CuteCatDlg.h : header file
//

#if !defined(AFX_CUTECATDLG_H__59A6637C_353F_4A01_BDC6_2F49AFD4827C__INCLUDED_)
#define AFX_CUTECATDLG_H__59A6637C_353F_4A01_BDC6_2F49AFD4827C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <afxcoll.h>
#include <afxtempl.h>
#include "catcore.h"
/////////////////////////////////////////////////////////////////////////////
// CCuteCatDlg dialog

class CCuteCatDlg : public CDialog
{
// Construction
public:
	CCuteCatDlg(CWnd* pParent = NULL);	// standard constructor

	static void SetDlgPtr(CWnd *pDlg);
	static void IOXMLIQ(iks *node);
	static void IOXMLMessage(iks *node);
	static void IOHexMessage(char *Node,size_t Len);
	static int	IOProcessDogMsg(void *data,int iType,iks *node);

// Dialog Data
	//{{AFX_DATA(CCuteCatDlg)
	enum { IDD = IDD_CUTECAT_DIALOG };
	CListCtrl	m_ListMsg;
	CListCtrl	m_ListView;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCuteCatDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;
	char m_Name[32],m_Option[128];
	char m_SelCuteDogName[32];
	static CCuteCatDlg* pCuteCatDlg;
	CMenu m_Menu;
	int				m_Status;
	ikstack			*m_StackForMap;
	CMapWordToPtr	m_MapCapData;
	CRITICAL_SECTION m_CrititialLock;
	CArray<CString,CString> m_CapDataArray;
	CArray<CAPELEMENT*,CAPELEMENT*> m_CapElement;
	int				m_EthModel;
	// Generated message map functions
	//{{AFX_MSG(CCuteCatDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnFilterOption();
	afx_msg void OnBtnLogin();
	afx_msg void OnBtnPioneerStart();
	afx_msg void OnBtnPioneerStop();
	afx_msg void OnBtnHookerStart();
	afx_msg void OnBtnHookerStop();
	afx_msg void OnClickListView(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnMenuHelp();
	afx_msg void OnClickMessageList(NMHDR* pNMHDR, LRESULT* pResult);
	//afx_msg void OnMenuDebug();
	//afx_msg void OnMenuCosedebug();
	afx_msg void OnMenuClear();
	afx_msg void OnMenuDebug(UINT nID/*,NMHDR* pNotifyStruct,LRESULT* lResult*/); 
	afx_msg void ONRadio(UINT nID/*,NMHDR* pNotifyStruct,LRESULT* lResult*/);		//release ??
	afx_msg void ONEthModel(UINT nID/*,NMHDR* pNotifyStruct,LRESULT* lResult*/); 
	//afx_msg void OnRadioHooker();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	void	HCIStatusCenter(int iStatus);	
	static void RespLogin(iks *x);
	static void RespAllFilters(iks *x);
	static void RespFilterNew(iks *x);
	static void RespCatSetHooker(iks *x);
	static void RespFilterNotice(iks *x);

	static void ShowString(char *ptrBuf,unsigned int Len);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CUTECATDLG_H__59A6637C_353F_4A01_BDC6_2F49AFD4827C__INCLUDED_)
