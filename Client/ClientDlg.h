
// ClientDlg.h: 헤더 파일
//

#pragma once

#pragma warning(disable:4996)
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#define BUFSIZE		1024
#define NAMESIZE	64

#define FIRSTROOM	11
#define SECONDROOM	12
#define SHOWUSERS	13

// CClientDlg 대화 상자
class CClientDlg : public CDialogEx
{
// 생성입니다.
public:
	CClientDlg(CWnd* pParent = nullptr);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CLIENT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CEdit m_ipAddr;
	CEdit m_port;
	void OnBnClickedButton1();
	bool CheckAddr();
	bool CheckPort();
	bool CheckName();
	afx_msg void OnBnClickedSendButton();
	afx_msg void AddFirstRoomMsg(CString str);
	afx_msg void AddSecondRoomMsg(CString str);
	CEdit m_message;

	WSADATA wsa;
	SOCKET sock;
	int retval;
	char sendbuf[BUFSIZE + 1];
	int len;
	HANDLE hThread;
	SOCKADDR_IN remoteaddr;

	void MySendTo(CString str);
	bool initSock();
	CListBox m_fistRoom;
	CListBox m_secondRoom;
	CEdit m_name;

	char m_roomNumber;

	afx_msg void OnBnClickedRadio1();
	afx_msg void OnBnClickedRadio2();
	afx_msg void OnBnClickedShowButton();
};
