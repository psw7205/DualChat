
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
}

BEGIN_MESSAGE_MAP(CClientDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CClientDlg::OnBnClickedButton1)
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
	if(!CheckAddr())
	{
		m_ipAddr.SetWindowText("127.0.0.1");
		MessageBox("잘못된 IP입력");

	}

	if (!CheckPort())
	{
		m_port.SetWindowText("9000");
		MessageBox("잘못된 Port입력");
	}


	MessageBox("접속 완료");
	m_ipAddr.EnableWindow(false);
	m_port.EnableWindow(false);
}

bool CClientDlg::CheckPort()
{
	CString str;
	m_port.GetWindowText(str);
	
	int port = atoi(str);

	if (0 <= port && port <= 65535 )
	{
		return true;
	}
	
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