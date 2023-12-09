; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CCuteCatDlg
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "CuteCat.h"

ClassCount=3
Class1=CCuteCatApp
Class2=CCuteCatDlg
Class3=CAboutDlg

ResourceCount=4
Resource1=IDD_CUTECAT_DIALOG
Resource2=IDR_MAINFRAME
Resource3=IDD_ABOUTBOX
Resource4=IDR_MENU

[CLS:CCuteCatApp]
Type=0
HeaderFile=CuteCat.h
ImplementationFile=CuteCat.cpp
Filter=N

[CLS:CCuteCatDlg]
Type=0
HeaderFile=CuteCatDlg.h
ImplementationFile=CuteCatDlg.cpp
Filter=D
LastObject=IDC_RADIO1
BaseClass=CDialog
VirtualFilter=dWC

[CLS:CAboutDlg]
Type=0
HeaderFile=CuteCatDlg.h
ImplementationFile=CuteCatDlg.cpp
Filter=D

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=5
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889
Control5=IDC_STATIC,static,1342308352

[DLG:IDD_CUTECAT_DIALOG]
Type=1
Class=CCuteCatDlg
ControlCount=17
Control1=IDC_MESSAGE_LIST,SysListView32,1350631425
Control2=IDC_EDIT_STR,edit,1352734724
Control3=IDC_EDIT_HEX,edit,1352734724
Control4=IDC_EDIT_NAME,edit,1350631552
Control5=IDC_STATIC,static,1342308352
Control6=IDC_LIST1,SysListView32,1350631425
Control7=IDC_EDIT_OPTION,edit,1350631552
Control8=IDC_STATIC,static,1342308352
Control9=IDC_BTN_PSTART,button,1342242816
Control10=IDC_RADIO1,button,1342177289
Control11=IDC_RADIO2,button,1342177289
Control12=IDC_STATIC,static,1342308352
Control13=IDC_BTN_PSTOP,button,1342242816
Control14=IDC_BTN_HSTART,button,1342242816
Control15=IDC_BTN_HSTOP,button,1342242816
Control16=IDC_STATIC,button,1342177287
Control17=IDC_BTN_LOGIN,button,1342242816

[MNU:IDR_MENU]
Type=1
Class=?
Command1=ID_MENUITEM_OPEN
Command2=ID_MENUITEM_SAVE
Command3=ID_MENUITEM_EXIT
Command4=ID_MENUITEM_LO
Command5=ID_MENUITEM_NET
Command6=ID_MENUITEM_DEBUG
Command7=ID_MENUITEM_COSEDEBUG
Command8=ID_MENUITEM_CLEAR
Command9=ID_MENUITEM_HELP
CommandCount=9

