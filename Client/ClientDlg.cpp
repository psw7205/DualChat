
// ClientDlg.cpp: 구현 파일
//

#include "stdafx.h"
#include "Client.h"
#include "ClientDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CClientDlg 대화 상자

DWORD WINAPI ThreadRecvMsg(LPVOID arg)
{
	CClientDlg *pDlg = (CClientDlg*)arg;

	if (pDlg == NULL)
		return 0;

	int retval;
	MSGDATA recvMsg;

	while (1) {
		// 데이터 받기
		retval = recv(pDlg->sock, (char*)&recvMsg, sizeof(recvMsg), 0);
		if (retval == SOCKET_ERROR) {
			AfxMessageBox("recv() error");
			break;
		}
		else if (retval == 0)
			break;

		CString str1 = "";
		CString str2 = "";
		int size = recvMsg.buf[0];

		switch (recvMsg.type)
		{
		case FIRSTROOM:
			pDlg->AddFirstRoomMsg(recvMsg.buf);
			break;
		case SECONDROOM:
			pDlg->AddSecondRoomMsg(recvMsg.buf);
			break;
		case SHOWUSERS:
			pDlg->AddFirstRoomMsg("ROOM1 명단################");
			pDlg->AddSecondRoomMsg("ROOM2 명단################");
			for (int i = 0; i < size; ++i)
			{
				recv(pDlg->sock, (char*)&recvMsg, sizeof(recvMsg), 0);
				str1.Append(recvMsg.buf);
				str1.Append("//");
				recv(pDlg->sock, (char*)&recvMsg, sizeof(recvMsg), 0);
				str2.Append(recvMsg.buf);
				str2.Append("//");
			}

			pDlg->AddFirstRoomMsg(str1);
			pDlg->AddSecondRoomMsg(str2);

			pDlg->AddFirstRoomMsg("########################");
			pDlg->AddSecondRoomMsg("########################");
			break;

		case NAMECHECK:
			if (recvMsg.buf[0] == '0')
			{
				AfxMessageBox("닉네임 중복");
				(pDlg->GetDlgItem(IDC_SEND_BUTTON))->EnableWindow(false);
				(pDlg->GetDlgItem(IDC_SHOW_BUTTON))->EnableWindow(false);
			}
			else
			{
				(pDlg->GetDlgItem(IDC_SEND_BUTTON))->EnableWindow(true);
				(pDlg->GetDlgItem(IDC_SHOW_BUTTON))->EnableWindow(true);
			}
			break;
		default:
			break;
		}
		
	}

	return 0;
}


CClientDlg::CClientDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CLIENT_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CClientDlg::~CClientDlg()
{
	closesocket(sock);
	WSACleanup();
}

void CClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_ipAddr);
	DDX_Control(pDX, IDC_EDIT2, m_port);
	DDX_Control(pDX, IDC_EDIT_MSG, m_message);
	DDX_Control(pDX, IDC_LIST_ROOM1, m_fistRoom);
	DDX_Control(pDX, IDC_LIST_ROOM2, m_secondRoom);
	DDX_Control(pDX, IDC_EDIT_NAME, m_name);
}

BEGIN_MESSAGE_MAP(CClientDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CClientDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_SEND_BUTTON, &CClientDlg::OnBnClickedSendButton)
	ON_BN_CLICKED(IDC_RADIO1, &CClientDlg::OnBnClickedRadio1)
	ON_BN_CLICKED(IDC_RADIO2, &CClientDlg::OnBnClickedRadio2)
	ON_BN_CLICKED(IDC_SHOW_BUTTON, &CClientDlg::OnBnClickedShowButton)
	ON_BN_CLICKED(IDC_ID_BUTTON, &CClientDlg::OnBnClickedIdButton)
	ON_BN_CLICKED(IDCANCEL, &CClientDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CClientDlg 메시지 처리기

BOOL CClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	m_ipAddr.SetWindowText("127.0.0.1");
	m_port.SetWindowText("9000");
	((CButton*)GetDlgItem(IDC_RADIO1))->SetCheck(true);
	
	memset(&chatMsg.buf, 0, sizeof(chatMsg.buf));
	chatMsg.type = FIRSTROOM;

	srand((unsigned int)time(NULL));
	chatMsg.ID = rand();

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CClientDlg::OnBnClickedButton1()
{
	if (!CheckAddr())
	{
		m_ipAddr.SetWindowText("127.0.0.1");
		MessageBox("잘못된 IP입력");
		return;
	}

	if (!CheckPort())
	{
		m_port.SetWindowText("9000");
		MessageBox("잘못된 Port입력");
		return;
	}

	if (!CheckName())
	{
		MessageBox("잘못된 ID 입력");
		return;
	}

	if (initSock())
	{
		m_ipAddr.EnableWindow(false);
		m_port.EnableWindow(false);
		GetDlgItem(IDC_BUTTON1)->EnableWindow(false);

		hThread = CreateThread(NULL, 0, ThreadRecvMsg, this, 0, NULL);
		if (hThread == NULL)
		{
			MessageBox("fail make thread\n");
		}
		else
		{
			CloseHandle(hThread);
		}

		chatMsg.type = NAMECHECK;
		send(sock, (char*)&chatMsg, sizeof(chatMsg), 0);
		chatMsg.type = FIRSTROOM;
	}	
}

bool CClientDlg::CheckName()
{
	CString str;
	m_name.GetWindowText(str);

	if (str == "")
		return false;

	if (str.GetLength() >= NAMESIZE)
		return false;
	
	strcpy(chatMsg.name[0], str);
	strcpy(chatMsg.name[1], str);
	return true;
}

bool CClientDlg::CheckPort()
{
	CString str;
	m_port.GetWindowText(str);
	
	if (str == "")
		return false;

	int port = atoi(str);

	// 포트번호 확인
	if (0 <= port && port <= 65535 )
		return true;
	
	return false;
}

bool CClientDlg::CheckAddr()
{
	CString str;
	m_ipAddr.GetWindowText(str);
	str.Remove(' ');

	int len = str.GetLength();

	// 입력이 안된 경우 X
	if (len == 0)
		return false;

	int cnt = 0;
	for (int i = 0; i < str.GetLength(); ++i)
	{
		char curChar = str.GetAt(i);
		// '.'이나 숫자가 아닌 경우 X
		if (curChar != '.' && !(curChar >= '0' && curChar <= '9'))
			return false;

		if (curChar == '.')
			cnt++;
	}

	// ip가 모두 입력되었는지 확인('.'이 3개 있어야 함)
	if (cnt != 3)
		return false;

	// 문자열을 '.' 기준으로 분리
	int pos = 0;
	for (int i = 0; i < 4; ++i)
	{
		int ip = atoi(str.Tokenize(".", pos));

		// 255 이상이면 잘못된 ip주소
		if (ip > 255)
			return false;
	}
	
	return true;
}

void CClientDlg::OnBnClickedSendButton()
{
	CString tmp;
	m_message.GetWindowText(tmp);

	if (tmp != "")
	{
		CString str;
		if (chatMsg.type == FIRSTROOM)
			str = chatMsg.name[0];
		else if(chatMsg.type == SECONDROOM)
			str = chatMsg.name[1];

		str += " : " + tmp;

		// 빈메세지가 아닌 경우
	
		m_message.SetWindowText("");
		// 데이터 보내기
		MySend(str);
	}
}

void CClientDlg::MySend(CString str)
{
	strcpy(chatMsg.buf, str);
	int retval = send(sock, (char*)&chatMsg, sizeof(chatMsg), 0);
	if (retval == SOCKET_ERROR) {
		AfxMessageBox("sendto() Error");
	}

	memset(&chatMsg.buf, 0, sizeof(chatMsg.buf));
}

bool CClientDlg::initSock()
{
	// 윈속 초기화
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return false;

	// socket()
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET)
	{ 
		MessageBox("socket() Error");
		return false;
	};

	CString tmp;
	m_port.GetWindowText(tmp);
	int port = atoi(tmp);

	m_ipAddr.GetWindowText(tmp);

	// connect()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(tmp);
	serveraddr.sin_port = htons(port);
	retval = connect(sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR)
	{
		MessageBox("connect() error");
		return false;
	}

	send(sock, (char*)&chatMsg, sizeof(chatMsg), 0);
	return true;
}

void CClientDlg::AddFirstRoomMsg(CString str)
{
	int idx = m_fistRoom.InsertString(-1, str);
	m_fistRoom.SetCurSel(idx);

	//화면보다 큰 메세지를 추가할 때 화면에 스크롤 바 생성
	CString    strTmp;
	CSize      sz;
	int        dx = 0;
	TEXTMETRIC tm;
	CDC*       pDC = m_fistRoom.GetDC();
	CFont*     pFont = m_fistRoom.GetFont();

	CFont* pOldFont = pDC->SelectObject(pFont);
	pDC->GetTextMetrics(&tm);

	for (int i = 0; i < m_fistRoom.GetCount(); i++)
	{
		m_fistRoom.GetText(i, strTmp);
		sz = pDC->GetTextExtent(strTmp);

		sz.cx += tm.tmAveCharWidth;

		if (sz.cx > dx)
			dx = sz.cx;
	}

	pDC->SelectObject(pOldFont);
	m_fistRoom.ReleaseDC(pDC);

	m_fistRoom.SetHorizontalExtent(dx);
}

void CClientDlg::AddSecondRoomMsg(CString str)
{
	int idx = m_secondRoom.InsertString(-1, str);
	m_secondRoom.SetCurSel(idx);

	//화면보다 큰 메세지를 추가할 때 화면에 스크롤 바 생성
	CString    strTmp;
	CSize      sz;
	int        dx = 0;
	TEXTMETRIC tm;
	CDC*       pDC = m_secondRoom.GetDC();
	CFont*     pFont = m_secondRoom.GetFont();

	CFont* pOldFont = pDC->SelectObject(pFont);
	pDC->GetTextMetrics(&tm);

	for (int i = 0; i < m_secondRoom.GetCount(); i++)
	{
		m_secondRoom.GetText(i, strTmp);
		sz = pDC->GetTextExtent(strTmp);

		sz.cx += tm.tmAveCharWidth;

		if (sz.cx > dx)
			dx = sz.cx;
	}

	pDC->SelectObject(pOldFont);
	m_secondRoom.ReleaseDC(pDC);

	m_secondRoom.SetHorizontalExtent(dx);
}

void CClientDlg::OnBnClickedRadio1()
{
	chatMsg.type = NAMECHECK;
	MySend("ROOM1 check");
	chatMsg.type = FIRSTROOM;
	m_name.SetWindowText(chatMsg.name[0]);

}

void CClientDlg::OnBnClickedRadio2()
{
	chatMsg.type = NAMECHECK;
	MySend("ROOM2 check");
	chatMsg.type = SECONDROOM;
	m_name.SetWindowText(chatMsg.name[1]);
}

void CClientDlg::OnBnClickedShowButton()
{
	int tmp = chatMsg.type;
	chatMsg.type = SHOWUSERS;

	MySend("show user list");

	chatMsg.type = tmp;
}

void CClientDlg::OnBnClickedIdButton()
{
	CString str;
	m_name.GetWindowText(str);
	if (chatMsg.type == FIRSTROOM)
	{
		strcpy(chatMsg.name[0], str);
	}
	else if(chatMsg.type == SECONDROOM)
	{
		strcpy(chatMsg.name[1], str);
	}

	int tmp = chatMsg.type;
	chatMsg.type = NAMECHECK;

	MySend("name change");

	chatMsg.type = tmp;
}

BOOL CClientDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_ESCAPE)
			return TRUE;
		else if (pMsg->wParam == VK_RETURN)
			return TRUE;
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}

void CClientDlg::OnBnClickedCancel()
{
	CString str = chatMsg.name[0];
	chatMsg.type = FIRSTROOM;
	str.Append(" 님이 퇴장하셨습니다.");
	MySend(str);

	chatMsg.type = SECONDROOM;
	str = chatMsg.name[1];
	str.Append(" 님이 퇴장하셨습니다.");
	MySend(str);

	CDialogEx::OnCancel();
}
