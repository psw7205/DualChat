
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

DWORD WINAPI RecvMsg(LPVOID arg)
{
	CClientDlg *pDlg = (CClientDlg*)arg;

	if (pDlg == NULL)
		return 0;

	int retval;
	char buf[BUFSIZE + 1] = { 0 };

	while (1) {
		// 데이터 받기
		retval = recv(pDlg->sock, buf, BUFSIZE + 1, 0);
		if (retval == SOCKET_ERROR) {
			AfxMessageBox("recv() error");
			break;
		}
		else if (retval == 0)
			break;

		// 받은 데이터 출력
		buf[retval] = '\0';

		CString str = buf;
		str.Delete(0, 1);

		if (buf[0] == FIRSTROOM)
		{
			pDlg->AddFirstRoomMsg(str);
		}
		else if (buf[0] == SECONDROOM)
		{
			pDlg->AddSecondRoomMsg(str);
		}
		else if (buf[0] == SHOWUSERS)
		{

		}
	}

	return 0;
}


CClientDlg::CClientDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CLIENT_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
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
	m_roomNumber = 0;

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

		hThread = CreateThread(NULL, 0, RecvMsg, NULL, 0, NULL);
		if (hThread == NULL)
		{
			MessageBox("fail make thread\n");
		}
		else
		{
			CloseHandle(hThread);
		}
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
	
	CString str;
	str.Format("%d", m_roomNumber);
	str.Append(str);
	
	// 빈메세지가 아닌 경우
	if (str != "")
	{
		m_message.SetWindowText("");
		// 데이터 보내기
		MySendTo(str);
	}
}

void CClientDlg::MySendTo(CString str)
{
	if (m_roomNumber == 0)
		return;


	// 메세지 구성
	str;
	int retval = sendto(sock, str, str.GetLength(), 0,
		(SOCKADDR*)& remoteaddr, sizeof(remoteaddr));
	if (retval == SOCKET_ERROR) {
		AfxMessageBox("sendto() Error");
	}
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
	m_roomNumber = FIRSTROOM;
}

void CClientDlg::OnBnClickedRadio2()
{
	m_roomNumber = SECONDROOM;
}


void CClientDlg::OnBnClickedShowButton()
{
	CString str;
	str.Format("%d", SHOWUSERS);
	MySendTo(str);
}
