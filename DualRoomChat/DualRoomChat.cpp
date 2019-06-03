
// DualRoomChat.cpp: 응용 프로그램에 대한 클래스 동작을 정의합니다.
//

#include "stdafx.h"
#include "DualRoomChat.h"
#include "DualRoomChatDlg.h"

// CDualRoomChatApp
BEGIN_MESSAGE_MAP(CDualRoomChatApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

// 유일한 CDualRoomChatApp 개체입니다.
CDualRoomChatApp theApp;

// CDualRoomChatApp 초기화
BOOL CDualRoomChatApp::InitInstance()
{
	CWinApp::InitInstance();

	CDualRoomChatDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	
	return FALSE;
}

