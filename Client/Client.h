// 2019년 1학기 네트워크프로그래밍 숙제 3번
// 성명: 박상우 학번: 16013093
// 플랫폼: VS2017

// Client.h: PROJECT_NAME 응용 프로그램에 대한 주 헤더 파일입니다.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH에 대해 이 파일을 포함하기 전에 'stdafx.h'를 포함합니다."
#endif

#include "resource.h"		// 주 기호입니다.


// CClientApp:
// 이 클래스의 구현에 대해서는 Client.cpp을(를) 참조하세요.
//

class CClientApp : public CWinApp
{
public:
	CClientApp();

// 재정의입니다.
public:
	virtual BOOL InitInstance();

// 구현입니다.

	DECLARE_MESSAGE_MAP()
};

extern CClientApp theApp;
