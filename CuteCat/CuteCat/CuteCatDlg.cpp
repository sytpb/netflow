// CuteCatDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CuteCat.h"
#include "CuteCatDlg.h"
#include "print.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About
//#define Dbg(a) {}
#define Dbg(a) {printf a ;printf("\n");}

#define ETH_LO	"lo"
#define ETH_NET	"net"
#define CONSOLE_OUT ConsoleOut()
#define CONSOLE_FREE FreeConsole()	
#define	LOGIN_STREAM				"<iq type='set' ><query xmlns='jabber:iq:auth'><username>%s</username></query></iq>"
#define	NEW_CUTEDOG_STREAM			"<iq type='set'><query xmlns='filter:new' action='create' model='%s'><username>%s</username><filterstring>%s</filterstring></query></iq>"
#define	DEL_CUTEDOG_STREAM			"<iq type='set'><query xmlns='filter:new' action='cancel'><username>%s</username></query></iq>"

#define	FILTER_SHOW					"<iq type='get' ><query xmlns='filter:show' action='show'></query></iq>"
#define	HOOK_CUTEDOG				"<iq type='set'><query xmlns='filter:hook' action='hook'><item cat='%s'>%s</item></query></iq>"
#define	CANCEL_HOOK_CUTEDOG			"<iq type='set'><query xmlns='filter:hook' action='cancel'><item cat='%s'>%s</item></query></iq>"
void ConsoleOut()
{
	if( ::AllocConsole() )
	{
		freopen( "CONOUT$", "w", stdout );
		freopen( "CONIN$", "r", stdin );
		freopen( "CON", "w", stderr );
	}
}
BOOL WINAPI ConsoleHandler(DWORD CEvent)
{
	//Stop();
    return TRUE;
}

#define RESET_INTERVAL	4*1024
#define RESET_MAX		10*1024*1024

#define IKS_RESET1(x) \
{unsigned u,a;\
iks_stack_stat(iks_stack((x)),&u,&a);\
if(u>RESET_MAX) iks_parser_reset();\
if(u>RESET_INTERVAL && strncmp("</message>"),buf+l-10) iks_parser_reset();}

#define IKS_RESET(x,n) \
{unsigned u,a;\
	iks_stack_stat(iks_stack((n)),&u,&a);\
	if(u>RESET_MAX) {iks_parser_reset(x);Dbg(("Clean Once !!!!"))};\
	if(u>RESET_INTERVAL) {iks_parser_reset(x);Dbg(("Clean Once !!!!"))}}

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCuteCatDlg dialog
CCuteCatDlg* CCuteCatDlg::pCuteCatDlg = NULL;

typedef enum
{
		HCI_ERROR			=0x0000,
		HCI_UNKNOW			=0x0001,
		
		HCI_UNAUTH			=0x0002,
		HCI_AUTHED			=0x0004,
		
		HCI_HOOKER			=0x0008,
		HCI_HOOKER_RUNNING	=0x0010,
		HCI_HOOKER_STOP		=0x0020,
		
		HCI_PIONEER			=0x0040,
		HCI_PIONEER_RUNNING	=0x0080,
		HCI_PIONEER_STOP	=0x0100
}HCISTATUS;

typedef enum
{
	CAT_HOOKER=0,
	CAT_PIONEER
}CATMODEL;


#define RUN_ERROR	0x0000
#define	RUN_UNKNOW	0x0001		/*not connect*/
		
#define	RUN_UNAUTH	0x0002		/*connect but not login	*/
#define	RUN_AUTHED	0x0003		/*connect && login*/
		
#define	RUN_START	0x0004
#define	RUN_RUNNING	0x0005
#define	RUN_STOP	0x0006
	
#define STACKMAP_RENEW \
if(m_StackForMap!=NULL) {\
	iks_stack_delete(m_StackForMap);\
	m_StackForMap=NULL;\
	m_StackForMap = iks_stack_new (1024*4, 0);}		//4 KB

CCuteCatDlg::CCuteCatDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCuteCatDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCuteCatDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	memset(m_Name,0,sizeof(m_Name));
	memset(m_SelCuteDogName,0,sizeof(m_SelCuteDogName));
	m_Status=RUN_UNKNOW;
	m_StackForMap = iks_stack_new (1024*10, 0);
	m_EthModel=0;				//Eth LO
	DebugFlag=PRINT_CLOSE;
	InitializeCriticalSection(&m_CrititialLock);
}

void CCuteCatDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCuteCatDlg)
	DDX_Control(pDX, IDC_MESSAGE_LIST, m_ListMsg);
	DDX_Control(pDX, IDC_LIST1, m_ListView);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CCuteCatDlg, CDialog)
	//{{AFX_MSG_MAP(CCuteCatDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_COMMAND(ID_MENUITEM_OPTION, OnFilterOption)
	ON_BN_CLICKED(IDC_BTN_LOGIN, OnBtnLogin)
	ON_BN_CLICKED(IDC_BTN_PSTART, OnBtnPioneerStart)
	ON_BN_CLICKED(IDC_BTN_PSTOP, OnBtnPioneerStop)
	ON_BN_CLICKED(IDC_BTN_HSTART, OnBtnHookerStart)
	ON_BN_CLICKED(IDC_BTN_STOP, OnBtnHookerStop)
	ON_NOTIFY(NM_CLICK, IDC_LIST1, OnClickListView)
	ON_COMMAND(ID_MENUITEM_HELP, OnMenuHelp)
	ON_NOTIFY(NM_CLICK, IDC_MESSAGE_LIST, OnClickMessageList)
	//ON_COMMAND(ID_MENUITEM_DEBUG, OnMenuDebug)
	//ON_COMMAND(ID_MENUITEM_COSEDEBUG, OnMenuCosedebug)
	ON_COMMAND_RANGE(ID_MENUITEM_DEBUG,ID_MENUITEM_COSEDEBUG,OnMenuDebug)
	ON_COMMAND(ID_MENUITEM_CLEAR, OnMenuClear)
	//ON_BN_CLICKED(IDC_RADIO1, OnRadioHooker)
	ON_COMMAND_RANGE(IDC_RADIO1,IDC_RADIO2,ONRadio)
	ON_COMMAND_RANGE(ID_MENUITEM_LO,ID_MENUITEM_NET,ONEthModel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCuteCatDlg message handlers
int test ()
{
	static char xml[] =
		"<iq type='result' to='ydobon@jabber.org'><query xmlns='jabber:iq:version'>"
		"<name>TestClient</name><os>SuxOS 2000</os><version><stable solidity='rock'/>"
		"1.2.0 patchlevel 2</version></query></iq>";
	static char xml2[] =
		"<Ni><C/>lala<br/><A/>Hello World<B/></Ni>";
	iks *x, *y, *z;
	char *t;
	
	//setlocale (LC_ALL, "");
	
	x = iks_new ("iq");
	iks_insert_attrib (x, "type", "resultypo");
	iks_insert_attrib (x, "type", "result");                 //overlap
	
	iks_insert_attrib (x, "to", "ydobon@jabber.org");
	y = iks_new_within ("query", iks_stack (x));
	iks_insert_cdata (iks_insert (y, "name"), "TestClient", 10);
	iks_insert_cdata (iks_insert (y, "os"), "SuxOS", 0);
	z = iks_insert (y, "version");
	iks_insert (z, "stable");
	
	iks_insert_cdata (z, "1.2", 3);
	iks_insert_cdata (z, ".0 patchlevel 2", 0);			//join with previous time

	//iks_insert_node (x, y);							//???? end </query></ip>  ???
	z = iks_find (y, "os");
	iks_insert_attrib (z, "error", "yes");
	iks_insert_attrib (z, "error", NULL);				//cancel error attrib

	iks_insert_cdata (z, " 2000<aa>", 9);					//insert....
	z = iks_next (z);
	z = iks_find (z, "stable");
	iks_insert_attrib (z, "solidity", "rock");
	z = iks_parent (iks_parent (z));
	iks_insert_attrib (z, "xmlns", "jabber:iq:version");
	
	t = iks_string (iks_stack (x), x);

	iks_insert_node (x, y);
	char *t1 = iks_string (iks_stack (x), x);
	if(!t || strcmp(t, xml) != 0) {
		printf("Result:   %s\n", t);
		printf("Expected: %s\n", xml);
		return 1;
	}
	iks_delete(x);
	
	
	x = iks_new ("Ni");
	y = iks_insert (x, "br");
	z = iks_prepend_cdata (y, "lala", 4);
	iks_prepend (z, "C");
	z = iks_insert_cdata (x, "Hello", 5);
	y = iks_append (z, "B");
	iks_prepend (z, "A");
	iks_append_cdata (z, " ", 1);
	iks_prepend_cdata (y, "World", 5);
	
	t = iks_string (iks_stack (x), x);
	if(!t || strcmp(t, xml2) != 0) {
		printf("Result:   %s\n", t);
		printf("Expected: %s\n", xml2);
		return 1;
	}
	iks_delete(x);
	
	return 0;
}

BOOL CCuteCatDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	//CMenu menu;

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	m_Menu.LoadMenu(IDR_MENU);
	SetMenu(&m_Menu);
	m_Menu.GetSubMenu(1)->CheckMenuItem(ID_MENUITEM_DEBUG,MF_BYCOMMAND);
	m_Menu.GetSubMenu(1)->CheckMenuItem(ID_MENUITEM_NET,MF_BYCOMMAND);
	//m_Menu.GetSubMenu(1)->ins

	CheckRadioButton(IDC_RADIO1,IDC_RADIO2,IDC_RADIO1);   

	//initialize the listctrl
	m_ListMsg.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_ListMsg.InsertColumn(0,"NO",LVCFMT_LEFT,38);
	m_ListMsg.InsertColumn(1,"SourceIP",LVCFMT_LEFT,110);
	m_ListMsg.InsertColumn(2,"SourcePort",LVCFMT_LEFT,80);
	m_ListMsg.InsertColumn(3,"DestIP",LVCFMT_LEFT,110);
	m_ListMsg.InsertColumn(4,"DestPort",LVCFMT_LEFT,80);
	m_ListMsg.InsertColumn(5,"DataLen",LVCFMT_CENTER,80);
	m_ListMsg.InsertColumn(6,"Content",LVCFMT_LEFT,430);
	m_ListMsg.InsertColumn(7,"ModelName",LVCFMT_CENTER,80);
	m_ListMsg.InsertColumn(8,"Mark",LVCFMT_CENTER,50);

	m_ListView.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_ListView.InsertColumn(0,"Name",LVCFMT_CENTER,80);
	m_ListView.InsertColumn(1,"Option",LVCFMT_LEFT,420);
	HCIStatusCenter(HCI_UNKNOW);

	if (SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler,TRUE)==FALSE)
	{
		return FALSE;
	}

/*
	char *tt="<message from='0.0.0.0' fp='1482' to='0.0.0.0' tp='22'><body>aaaa</body></message>";
	char *p=tt;
	char From[32]={0},Fp[32]={0},To[32]={0},Tp[32]={0},Body[1024]={0};
	p=strstr(tt,"from='");
	sscanf(p,"from='%[^']",From);

	p=strstr(p,"fp='");
	sscanf(p,"fp='%[^']",Fp);

	p=strstr(p,"to='");
	sscanf(p,"to='%[^']",To);

	p=strstr(p,"tp='");
	sscanf(p,"tp='%[^']",Tp);
	p=strstr(p,"<body>");
	sscanf(p,"<body>%[^<]",Body);
*/
	return TRUE;  // return TRUE  unless you set the focus to a control

}

void CCuteCatDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CCuteCatDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CCuteCatDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CCuteCatDlg::OnFilterOption() 
{
	AfxMessageBox("unsupport now!");
	int   iRadioButton;      
	iRadioButton=GetCheckedRadioButton(IDC_RADIO1,IDC_RADIO2);   
}

void CCuteCatDlg::HCIStatusCenter(int iStatus)
{
	switch(iStatus)
	{
	case HCI_UNKNOW:
		GetDlgItem(IDC_BTN_PSTART)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_PSTOP)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_HSTART)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_HSTOP)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_OPTION)->EnableWindow(FALSE);
		GetDlgItem(IDC_RADIO1)->EnableWindow(FALSE);
		GetDlgItem(IDC_RADIO2)->EnableWindow(FALSE);
		break;
	case HCI_UNAUTH:
		GetDlgItem(IDC_EDIT_NAME)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_LOGIN)->EnableWindow(TRUE);
		break;
	case HCI_AUTHED:
		GetDlgItem(IDC_BTN_LOGIN)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_NAME)->EnableWindow(FALSE);
		GetDlgItem(IDC_RADIO1)->EnableWindow(TRUE);
		GetDlgItem(IDC_RADIO2)->EnableWindow(TRUE);
		break;
	case HCI_HOOKER:
		GetDlgItem(IDC_BTN_HSTART)->EnableWindow(TRUE);
		//<-->
		GetDlgItem(IDC_EDIT_OPTION)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_PSTART)->EnableWindow(FALSE);
		break;
	case HCI_HOOKER_RUNNING:
		GetDlgItem(IDC_BTN_HSTOP)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_HSTART)->EnableWindow(FALSE);
		GetDlgItem(IDC_RADIO1)->EnableWindow(FALSE);
		GetDlgItem(IDC_RADIO2)->EnableWindow(FALSE);
		break;
	case HCI_HOOKER_STOP:
		GetDlgItem(IDC_RADIO1)->EnableWindow(TRUE);
		GetDlgItem(IDC_RADIO2)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_HSTOP)->EnableWindow(FALSE);
		break;
	case HCI_PIONEER:
		GetDlgItem(IDC_EDIT_OPTION)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_PSTART)->EnableWindow(TRUE);
		//GetDlgItem(IDC_EDIT_OPTION)->EnableWindow(TRUE);
		break;
	case HCI_PIONEER_RUNNING:
		GetDlgItem(IDC_BTN_PSTOP)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_PSTART)->EnableWindow(FALSE);
		GetDlgItem(IDC_RADIO1)->EnableWindow(FALSE);
		GetDlgItem(IDC_RADIO2)->EnableWindow(FALSE);
		break;
	case HCI_PIONEER_STOP:
		GetDlgItem(IDC_RADIO1)->EnableWindow(TRUE);
		GetDlgItem(IDC_RADIO2)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_PSTART)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_PSTOP)->EnableWindow(FALSE);
		break;
	case HCI_ERROR:
		GetDlgItem(IDC_BTN_LOGIN)->EnableWindow(TRUE);
		GetDlgItem(IDC_RADIO1)->EnableWindow(TRUE);
		GetDlgItem(IDC_RADIO2)->EnableWindow(TRUE);

		GetDlgItem(IDC_BTN_PSTART)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_PSTOP)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_HSTART)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_HSTOP)->EnableWindow(FALSE);
		break;
	}
}

void CCuteCatDlg::RespAllFilters(iks *x)
{
	char *Name=NULL,*Option=NULL;
	CCuteCatDlg *pDlg=CCuteCatDlg::pCuteCatDlg;
	pDlg->m_ListView.DeleteAllItems();
	iks* Child=x;
	while(Child)
	{
		Name=iks_find_cdata(Child,"name");
		Option=iks_find_cdata(Child,"filterstring");
		if(Name!=NULL || Option!=NULL)
		{	
			int nItem = pDlg->m_ListView.InsertItem(pDlg->m_ListView.GetItemCount(),"");
			pDlg->m_ListView.SetItemText(nItem,0,Name);
			pDlg->m_ListView.SetItemText(nItem,1,Option);
		}
		Child=iks_next(Child);
	}
	if(pDlg->m_ListView.GetItemCount()>0 /*&& Status == HOOKER_START*/)
	{
	}
}
void CCuteCatDlg::RespLogin(iks *x)
{
	iks		*err = iks_find(x,"error");
	char	*ID=iks_find_attrib(err,"id");
	CCuteCatDlg *pDlg=CCuteCatDlg::pCuteCatDlg;
	
	if(iks_strcmp(ID,"OK")==0)
	{
		PushSendStream(FILTER_SHOW,0);
		pDlg->m_Status=RUN_AUTHED;
		pDlg->HCIStatusCenter(HCI_AUTHED);

		int iRadioButton=pDlg->GetCheckedRadioButton(IDC_RADIO1,IDC_RADIO2);
		if(iRadioButton==IDC_RADIO2)
		{
			pDlg->HCIStatusCenter(HCI_PIONEER);
		}
		else if(iRadioButton==IDC_RADIO1)
		{
		}
	}
	else
	{
		pDlg->m_Status=RUN_UNAUTH;
		AfxMessageBox("Name have existed please change another one!");
	}
}
void CCuteCatDlg::RespFilterNew(iks *x)
{
	char	*action=iks_find_attrib(x,"action");
	iks		*err = iks_find(x,"error");
	char	*ID=iks_find_attrib(err,"id");
	CCuteCatDlg *pDlg=CCuteCatDlg::pCuteCatDlg;
	
	if(iks_strcmp(action,"create")==0) 
	{
		if(iks_strcmp(ID,"OK")==0)
		{
			int iRadioButton=pDlg->GetCheckedRadioButton(IDC_RADIO1,IDC_RADIO2);
			pDlg->m_Status=RUN_RUNNING;
			if(iRadioButton==IDC_RADIO2)
			{
				pDlg->HCIStatusCenter(HCI_PIONEER_RUNNING);		//???? server create thread fail ??
			}
		}
		else
		{
			pDlg->m_Status=RUN_STOP;
			//pDlg->HCIStatusCenter(HCI_PIONEER_STOP);
			AfxMessageBox("Option Set Fail,Please check Option condition!");
		}
	}
	else if(iks_strcmp(action,"cancel")==0 )
	{
		if(iks_strcmp(ID,"OK")==0)
		{
			pDlg->m_Status=RUN_STOP;
			pDlg->HCIStatusCenter(HCI_PIONEER_STOP);
		}
		else
		{		
			AfxMessageBox("Stop Pionner Fail!!");
		}
	}

}

void CCuteCatDlg::RespFilterNotice(iks *x)
{
	char	*action=iks_find_attrib(x,"action");
	char	*name=iks_find_cdata(x,"username");
	CCuteCatDlg *pDlg=CCuteCatDlg::pCuteCatDlg;
	char Msg[128]={0};

	if(iks_strcmp(action,"lose")==0) 
	{
		Dbg(("****************** LOSE [%s]!!!! OK",name));
		sprintf(Msg,"Pioneer [%s] have exit,so stop now!",name);
		AfxMessageBox(Msg);
		pDlg->HCIStatusCenter(HCI_HOOKER_STOP);
		pDlg->m_Status=RUN_STOP;
	}
}
void CCuteCatDlg::IOXMLIQ(iks *node)
{
	char *type=iks_find_attrib(node,"type");
	char *xmlns=iks_find_attrib(iks_find(node,"query"),"xmlns");
	iks	 *x=iks_find(node,"query");
	
	if(strcmp(type,"result")==0 && strcmp(xmlns,"jabber:iq:auth")==0) 	{RespLogin(x);return;}
	if(strcmp(type,"result")==0 && strcmp(xmlns,"filter:show")==0)   	{RespAllFilters(x);return;}
	if(strcmp(type,"result")==0 && strcmp(xmlns,"filter:hook")==0)    	{RespCatSetHooker(x);return;}
	if(strcmp(type,"result")==0 && strcmp(xmlns,"filter:new")==0)    	{RespFilterNew(x);return;}
	if(strcmp(type,"set")==0 && strcmp(xmlns,"filter:notice")==0)    	{RespFilterNotice(x);return;}
	Dbg(("\n\t¡ø-----------------------IOXML IQ ERROR !!! -----------------------¡ø"));
}

//?? iks_hide recover ?
void CCuteCatDlg::IOHexMessage(char *Node,size_t Len)
{
	char *pNode=Node;
	CCuteCatDlg *pDlg=CCuteCatDlg::pCuteCatDlg;
	char From[32]={0},Fp[32]={0},To[32]={0},Tp[32]={0},Body[1024]={0},BLen[8]={0};
	int Key;
	if(iks_strncasecmp("<message",Node,8) && strstr(Node,"<body>") && strstr(Node,"</body>"))	//</»á²»È«??????
	{
		pNode=strstr(pNode,"from='");
		sscanf(pNode,"from='%[^']",From);
		
		pNode=strstr(pNode,"fp='");
		sscanf(pNode,"fp='%[^']",Fp);
		
		pNode=strstr(pNode,"to='");
		sscanf(pNode,"to='%[^']",To);
		
		pNode=strstr(pNode,"tp='");
		sscanf(pNode,"tp='%[^']",Tp);

		pNode=strstr(pNode,"size='");
		sscanf(pNode,"size='%[^']",BLen);

		pNode=strstr(pNode,"<body>");
		sscanf(pNode,"<body>%[^<]",Body);

		Key=pDlg->m_ListMsg.GetItemCount();
		int nItem = pDlg->m_ListMsg.InsertItem(Key,"");
		pDlg->m_ListMsg.SetItemData(Key,Key);
		
		pDlg->m_ListMsg.SetItemText(nItem,1,From);
		pDlg->m_ListMsg.SetItemText(nItem,2,Fp);
		pDlg->m_ListMsg.SetItemText(nItem,3,To);
		pDlg->m_ListMsg.SetItemText(nItem,4,Tp);
		pDlg->m_ListMsg.SetItemText(nItem,5,BLen);			//add len lable ???
		pDlg->m_ListMsg.SetItemText(nItem,6,Body);
		
		pDlg->m_CapDataArray.Add(Body);						//???
	}
}
void CCuteCatDlg::IOXMLMessage(iks *node)
{
	CCuteCatDlg *pDlg=CCuteCatDlg::pCuteCatDlg;
	int	Key;
	size_t iLen=0;
	CAPELEMENT *pCapEle=NULL;
	char StrLen[32]={0};
	char *From=iks_find_attrib(node,"from");
	char *Fp=iks_find_attrib(node,"fp");
	char *To=iks_find_attrib(node,"to");
	char *Tp=iks_find_attrib(node,"tp");
	char *Len=iks_find_attrib(iks_find(node,"body"),"size");
	char *RawLen=iks_find_attrib(iks_find(node,"body"),"rawsize");
	char *Sn=iks_find_attrib(iks_find(node,"body"),"sn");
	char *data=iks_find_cdata(node,"body");	

	iLen=atoi(Len);
	if(iLen>0)
	{
		char *pDataDecode;
		if(iLen>4096)
			{Dbg(("so long data ???"));return;}
		//lock
		EnterCriticalSection(&pDlg->m_CrititialLock);
		pCapEle = (CAPELEMENT*)iks_stack_alloc(pDlg->m_StackForMap,sizeof(size_t)+iLen+1);
		//unlock
		LeaveCriticalSection(&pDlg->m_CrititialLock);
		if(pCapEle==NULL)
			{Dbg(("error !!!! %p",pDlg->m_StackForMap));return;}
		//pCapEle->pBuf=(char*)(pCapEle+sizeof(size_t));	//??? byte align
		memset(pCapEle->pBuf,0,iLen+1);						//???
		pDataDecode=iks_base64_decode(data);
		/*if(pDataDecode != NULL)*/memcpy(pCapEle->pBuf,pDataDecode,iLen);
		pCapEle->iLen=iLen;
	}
	Key=pDlg->m_ListMsg.GetItemCount();
	int nItem = pDlg->m_ListMsg.InsertItem(Key,"");
	pDlg->m_ListMsg.SetItemData(Key,Key);

	pDlg->m_ListMsg.SetItemText(nItem,0,Sn);
	pDlg->m_ListMsg.SetItemText(nItem,1,From);
	pDlg->m_ListMsg.SetItemText(nItem,2,Fp);
	pDlg->m_ListMsg.SetItemText(nItem,3,To);
	pDlg->m_ListMsg.SetItemText(nItem,4,Tp);
	pDlg->m_ListMsg.SetItemText(nItem,5,Len);
	pDlg->m_ListMsg.SetItemText(nItem,6,pCapEle->pBuf);
	pDlg->m_CapElement.Add(pCapEle);					//mutex ??

	PRT_PARSESN(("\n\tSN [%s] ",Sn));	
	PRT_OK("\n\t",(unsigned char*)pCapEle->pBuf,pCapEle->iLen,Sn);
	//Dbg(("\n\t-sn [%s] ",Sn));

}

#define E_OK 0
void CCuteCatDlg::SetDlgPtr(CWnd *pDlg)
{
	//return_if_fail(pDlg);
	CCuteCatDlg::pCuteCatDlg=(CCuteCatDlg*)pDlg;
}
int CCuteCatDlg::IOProcessDogMsg(void *data,int iType,iks *node)
{
	ikspak *pak = NULL;
	PCLIENTINFO pClientInfo=(PCLIENTINFO)(data);
	CCuteCatDlg *pDlg=CCuteCatDlg::pCuteCatDlg;
	char SndBuf[1024]={0};

	switch(iType)
	{
	case IKS_NODE_START: 
		printf("\n\t¡ø-----------------------START !!!--------------------¡ø");
		SetClientStatus(CLIENT_SHAKEED);
		sprintf(SndBuf,LOGIN_STREAM,pDlg->m_Name);
		if(PushSendStream(SndBuf,0))SetClientStatus(CLIENT_LOGINED);
		break;
	case IKS_NODE_NORMAL:
		pak = iks_packet(node);
		switch (pak->type) 
		{
		case IKS_PAK_NONE:
			printf("\n\t¡ø--------------------JABBER: NONE------------------¡ø");
			break;
		case IKS_PAK_MESSAGE:
			//printf("\n¡ø--------------------JABBER:Message ----------------¡ø\n");
			IOXMLMessage(node);
			break;
		case IKS_PAK_PRESENCE:
			break;
		case IKS_PAK_S10N:
			break;
		case IKS_PAK_IQ:
			printf("\n\t¡ø--------------------JABBER: IQ  ----------------¡ø");
			IOXMLIQ(node);
			break;
		default:
			printf("JABBER: I Dont know %d\n", pak->type);
			break;
		}
		break;
	case IKS_NODE_ERROR:
		printf("\n\t¡ø--------------------ERROR !!!----------------------¡ø");
		break;
	case IKS_NODE_STOP:
		printf("\n\t¡ø--------------------STOP !!!-----------------------¡ø");
		if(pClientInfo->Status==CLIENT_STOPPED)
		{
			pDlg->HCIStatusCenter(HCI_UNKNOW);
			pDlg->HCIStatusCenter(HCI_UNAUTH);
			pDlg->m_Status=RUN_UNAUTH;
			AfxMessageBox("Server Stopped,Please check it!!");
		}
		break;
	}
	if(node) IKS_RESET(pClientInfo->Prs,node);
	//Dbg(("¡ø ProcessMsg :%s¡ø\n",pTmp));
	return E_OK;
}
void CCuteCatDlg::OnBtnLogin() 
{
	CString Name;
	char *pName=NULL;
	GetDlgItemText(IDC_EDIT_NAME,Name);
	if(Name.GetLength()<3)
	{
		AfxMessageBox("name len should more then 2 !!");
		return;
	}
	
	pName=Name.GetBuffer(Name.GetLength());
	if(pName)
	{
		strncpy(m_Name,pName,sizeof(m_Name)-1);
		Start(0,pName,CCuteCatDlg::IOProcessDogMsg);
		//if(m_Status==RUN_UNKNOW) m_Status=RUN_UNAUTH;
	}
	
}
void CCuteCatDlg::OnBtnPioneerStart() 
{
	char *ptrOpton=NULL;
	char SndBuf[1024]={0};
	CString Option;
	GetDlgItemText(IDC_EDIT_OPTION,Option);
	if(Option.GetLength()<1)
	{
		AfxMessageBox("Please Input Option String!!");
		return;
	}
	ptrOpton=Option.GetBuffer(Option.GetLength());
	if(ptrOpton)
	{
		strncpy(m_Option,ptrOpton,sizeof(m_Option)-1);
	}

	if(m_Status==RUN_AUTHED || m_Status==RUN_STOP)
	{
		if(m_EthModel==0)
		{
			sprintf(SndBuf,NEW_CUTEDOG_STREAM,ETH_LO,m_Name,m_Option);
		}
		else if(m_EthModel==1)
		{
			sprintf(SndBuf,NEW_CUTEDOG_STREAM,ETH_NET,m_Name,m_Option);
		}
		
		PushSendStream(SndBuf,0);
		m_Status=RUN_START;
	}
	else
	{
		AfxMessageBox("Status Error !! ");
	}
}

void CCuteCatDlg::OnBtnPioneerStop() 
{	
	char SndBuf[1024]={0};

	if(m_Status==RUN_RUNNING)
	{
		sprintf(SndBuf,DEL_CUTEDOG_STREAM,m_Name);
		PushSendStream(SndBuf,0);	
	}
	m_Status=RUN_STOP;
}
void CCuteCatDlg::RespCatSetHooker(iks *x)
{
	char	*action=iks_find_attrib(x,"action");	
	iks		*err = iks_find(x,"error");
	char	*ID=iks_find_attrib(err,"id");
	CCuteCatDlg *pDlg=CCuteCatDlg::pCuteCatDlg;
	
	if(iks_strcmp(action,"hook")==0) 
	{
		if(iks_strcmp(ID,"OK")==0)
		{
			pDlg->m_Status=RUN_RUNNING;
			pDlg->HCIStatusCenter(HCI_HOOKER_RUNNING);
		}
		else
		{
			AfxMessageBox("Set Hook Error !!!");
			pDlg->m_Status=RUN_STOP;
		}
	}
	else if(iks_strcmp(action,"cancel")==0)
	{
		if(iks_strcmp(ID,"OK")==0)
		{
			pDlg->m_Status=RUN_STOP;
			pDlg->HCIStatusCenter(HCI_HOOKER_STOP);
		}
		else
		{			
			AfxMessageBox("Stop Hooker Fail!!");
		}
	}
}
void CCuteCatDlg::OnBtnHookerStart() 
{
	char SndBuf[1024]={0};

	if(*m_SelCuteDogName!='\0' && (m_Status==RUN_AUTHED || m_Status==RUN_STOP))
	{
		sprintf(SndBuf,HOOK_CUTEDOG,m_Name,m_SelCuteDogName);
		PushSendStream(SndBuf,0);
		m_Status=RUN_START;
	}	
}

void CCuteCatDlg::OnBtnHookerStop() 
{
	char SndBuf[1024]={0};
	
	if(m_Status==RUN_RUNNING)
	{
		sprintf(SndBuf,CANCEL_HOOK_CUTEDOG,m_Name,m_SelCuteDogName);
		PushSendStream(SndBuf,0);	
	}
	//m_Status=RUN_STOP;
}

void CCuteCatDlg::OnClickListView(NMHDR* pNMHDR, LRESULT* pResult) 
{
	CCuteCatDlg *pDlg=CCuteCatDlg::pCuteCatDlg;
	POSITION pos = m_ListView.GetFirstSelectedItemPosition();
	if(pos)
	{
		int nItem = m_ListView.GetNextSelectedItem(pos);	
		CString tmpSelDog = m_ListView.GetItemText(nItem,0);
		if(!tmpSelDog.IsEmpty() && m_Status!=RUN_RUNNING) 
		{
			strcpy(m_SelCuteDogName,tmpSelDog.GetBuffer(tmpSelDog.GetLength()));
			int iRadioButton=pDlg->GetCheckedRadioButton(IDC_RADIO1,IDC_RADIO2);
			if(iRadioButton==IDC_RADIO1)
			{
				pDlg->HCIStatusCenter(HCI_HOOKER);
			}
		}
	}

	*pResult = 0;
}

void CCuteCatDlg::OnMenuHelp() 
{
	CAboutDlg help;
	help.DoModal();
}

void CCuteCatDlg::ShowString(char *ptrBuf,unsigned int Len)
{
	char OutStrBuf[4096]={0};
	int i=0;
	CCuteCatDlg *pDlg=CCuteCatDlg::pCuteCatDlg;
	if(Len<0 || Len>4096)return;

	i=Len;
	memcpy(OutStrBuf,ptrBuf,Len);
	while(--i>=0)
	{
		if(OutStrBuf[i] == 0)OutStrBuf[i]='.';
	}
	pDlg->SetDlgItemText(IDC_EDIT_STR,OutStrBuf);
}

void CCuteCatDlg::OnClickMessageList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	char /*Element0[2]={0},*/Element[3]={0};
	unsigned char c;
	int iCount=0;
	CString HEX;
	char *ptrBuf=NULL;

	CCuteCatDlg *pDlg=CCuteCatDlg::pCuteCatDlg;
	POSITION pos = m_ListMsg.GetFirstSelectedItemPosition();
	int	iIndex;
	CString strData;
	CAPELEMENT *pCapEle=NULL;
	if(pos)
	{
		int nItem = m_ListMsg.GetNextSelectedItem(pos);	
		iIndex=m_ListMsg.GetItemData(nItem);
		pCapEle=m_CapElement.GetAt(iIndex);
		if(iIndex<m_CapElement.GetSize()) 
		{
			/*pDlg->SetDlgItemText(IDC_EDIT_STR,pCapEle->pBuf);*/
			CCuteCatDlg::ShowString(pCapEle->pBuf,pCapEle->iLen);
			iCount=pCapEle->iLen;
			ptrBuf=pCapEle->pBuf;
 			while(--iCount>=0)
 			{
				c=*ptrBuf;
 				sprintf(Element,"%-3X",c);
 				HEX+=Element;
 				++ptrBuf;
 			}
 			pDlg->SetDlgItemText(IDC_EDIT_HEX,HEX);
		}
	}
	*pResult = 0;
}

void CCuteCatDlg::OnMenuDebug(UINT nID) 
{	
	CMenu *submenu=m_Menu.GetSubMenu(1);
	UINT state = submenu->GetMenuState(nID, MF_BYCOMMAND);
	ASSERT(state != 0xFFFFFFFF);

	if (state & MF_CHECKED)
	{
		submenu->CheckMenuItem(nID, MF_UNCHECKED| MF_BYCOMMAND);
		if(nID==ID_MENUITEM_DEBUG)
		{
			DebugFlag&=~PRINT_RAW;
		}
		else if(nID==ID_MENUITEM_COSEDEBUG)
		{
			DebugFlag&=~PRINT_TCPDUMP;
		}
		if(DebugFlag==PRINT_CLOSE)
		{CONSOLE_FREE;}
	}
	else
	{
		CONSOLE_OUT;
		submenu->CheckMenuItem(nID, MF_CHECKED| MF_BYCOMMAND);
		if(nID==ID_MENUITEM_DEBUG)
		{
			DebugFlag|=PRINT_RAW;
		}
		else if(nID==ID_MENUITEM_COSEDEBUG)
		{
			DebugFlag|=PRINT_TCPDUMP;
		}
	}
}
void CCuteCatDlg::OnMenuClear()
{
	CCuteCatDlg *pDlg=CCuteCatDlg::pCuteCatDlg;	
	pDlg->m_ListMsg.DeleteAllItems();

	/*lock*/
	EnterCriticalSection(&m_CrititialLock);
	m_CapElement.RemoveAll();
	STACKMAP_RENEW		//???
	LeaveCriticalSection(&m_CrititialLock);
	/*unlock*/
	//reuse ???
	pDlg->SetDlgItemText(IDC_EDIT_STR,"");
	pDlg->SetDlgItemText(IDC_EDIT2,"");
}

void CCuteCatDlg::ONRadio( UINT nID/*,NMHDR* pNotifyStruct,LRESULT* lResult*/)
{
	if(m_Status==RUN_UNAUTH)
	{
		AfxMessageBox("You have not login succeed!!");
		return;
	}

	if(nID==IDC_RADIO1)
	{
		HCIStatusCenter(HCI_HOOKER);
	}
	else if(nID==IDC_RADIO2)
	{
		HCIStatusCenter(HCI_PIONEER);
	}
}
void CCuteCatDlg::ONEthModel( UINT nID/*,NMHDR* pNotifyStruct,LRESULT* lResult*/)
{
	CMenu *submenu=m_Menu.GetSubMenu(1);
	UINT state = submenu->GetMenuState(nID, MF_BYCOMMAND);
	ASSERT(state != 0xFFFFFFFF);
	
	if (state & MF_CHECKED)
	{
	}
	else
	{
		submenu->CheckMenuItem(nID, MF_CHECKED| MF_BYCOMMAND);
		if(nID==ID_MENUITEM_NET)
		{
			submenu->CheckMenuItem(ID_MENUITEM_LO, MF_UNCHECKED| MF_BYCOMMAND);
			m_EthModel=1;
		}
		else if(nID==ID_MENUITEM_LO)
		{
			submenu->CheckMenuItem(ID_MENUITEM_NET, MF_UNCHECKED| MF_BYCOMMAND);
			m_EthModel=0;
		}
	}
}