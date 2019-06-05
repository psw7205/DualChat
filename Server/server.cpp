
#pragma warning(disable:4996)
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#define SERVERPORT	9000
#define BUFSIZE		1024
#define NAMESIZE	64

#define FIRSTROOM	11
#define SECONDROOM	12
#define SHOWUSERS	13

// 소켓 정보 저장을 위한 구조체와 변수
struct SOCKETINFO
{
	SOCKET	sock;
	char	buf[BUFSIZE];
	int		recvbytes;
	int		sendbytes;
	int		room;
	char	name[NAMESIZE];
	int		ID;
};

bool b_login;
bool b_show;
bool b_firstRoom;
bool b_secondRoom;
int  socketCount;
int  userID;
char  chatMsg[BUFSIZE];
SOCKETINFO	*SockArray[FD_SETSIZE];

// 소켓 관리 함수
BOOL AddInfo(SOCKET sock);
void RemoveInfo(int nIndex);
bool CheckUserName(SOCKETINFO *ptr, int retval, int index);
void Initialize();

// 소켓 함수 오류 출력 후 종료
void err_quit(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

// 소켓 함수 오류 출력
void err_display(char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[%s] %s", msg, (char *)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

int main()
{
	Initialize();

	int retval;

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return 1;

	// socket()
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) err_quit(const_cast<char*>("socket()"));

	// bind()
	SOCKADDR_IN serverAddr;
	ZeroMemory(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(SERVERPORT);
	retval = bind(sock, (SOCKADDR *)&serverAddr, sizeof(serverAddr));
	if (retval == SOCKET_ERROR) err_quit(const_cast<char*>("bind()"));

	// listen()
	retval = listen(sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit(const_cast<char*>("listen()"));

	// 넌블로킹 소켓으로 전환
	u_long on = 1;
	retval = ioctlsocket(sock, FIONBIO, &on);
	if (retval == SOCKET_ERROR) err_display(const_cast<char*>("ioctlsocket()"));

	// 데이터 통신에 사용할 변수(공통)
	FD_SET rset, wset;
	SOCKET clientSock;
	int len, i, j;
	SOCKADDR_IN clientAddr;

	while (1) {
		// 소켓 셋 초기화
		FD_ZERO(&rset);
		FD_ZERO(&wset);
		FD_SET(sock, &rset);
		for (i = 0; i < socketCount; i++) {

			if (SockArray[i]->recvbytes > SockArray[i]->sendbytes)
				FD_SET(SockArray[i]->sock, &wset);
			else
				FD_SET(SockArray[i]->sock, &rset);
		}

		// select()
		retval = select(0, &rset, &wset, NULL, NULL);
		if (retval == SOCKET_ERROR) {
			err_display(const_cast<char*>("select()"));
			break;
		}

		// 소켓 셋 검사(1): 클라이언트 접속 수용
		if (FD_ISSET(sock, &rset)) {
			len = sizeof(clientAddr);
			clientSock = accept(sock, (SOCKADDR *)&clientAddr, &len);
			if (clientSock == INVALID_SOCKET) {
				err_display(const_cast<char*>("accept()"));
				break;
			}
			else {
				// 접속한 클라이언트 정보 출력
				printf("##클라이언트 접속: %s:%d\n",
					inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
				// 소켓 정보 추가
				AddInfo(clientSock);
			}
		}

		// 소켓 셋 검사(2): 데이터 통신
		for (i = 0; i < socketCount; i++) 
		{
			SOCKETINFO *ptr = SockArray[i];

			if (FD_ISSET(ptr->sock, &rset)) 
			{
				// 데이터 받기
				retval = recv(ptr->sock, ptr->buf, BUFSIZE, 0);
				if (retval == 0 || retval == SOCKET_ERROR)
				{
					RemoveInfo(i);
					continue;
				}

				ptr->recvbytes = retval;

				len = sizeof(clientAddr);
				getpeername(ptr->sock, (SOCKADDR *)&clientAddr, &len);
				ptr->buf[retval] = '\0';

				/*

				데이터 처리

				*/

				if (FD_ISSET(ptr->sock, &wset))
				{
					// 데이터 보내기
					for (j = 0; j < socketCount; j++) 
					{  // 여러 접속자에게 발송
						SOCKETINFO *sptr = SockArray[j];


						retval = send(sptr->sock, ptr->buf + ptr->sendbytes,
							ptr->recvbytes - ptr->sendbytes, 0);

						if (retval == SOCKET_ERROR) 
						{
							err_display(const_cast<char*>("send()"));
							RemoveInfo(i);
							continue;
						}
					}

					ptr->sendbytes += retval;
					if (ptr->recvbytes == ptr->sendbytes) 
					{
						ptr->recvbytes = ptr->sendbytes = 0;
					}
				}
			}
		}
	}

	return 0;
}

// 소켓 정보 추가
BOOL AddInfo(SOCKET sock)
{
	if (socketCount >= FD_SETSIZE) {
		printf("[오류] 소켓 정보를 추가할 수 없습니다!\n");
		return FALSE;
	}

	SOCKETINFO *ptr = new SOCKETINFO;
	if (ptr == NULL) {
		printf("[오류] 메모리가 부족합니다!\n");
		return FALSE;
	}
	//시간에 따른 랜덤 UserID값 생성
	srand((unsigned int)time(NULL));
	userID = rand();

	ptr->sock = sock;
	ptr->recvbytes = 0;
	ptr->ID = userID;
	SockArray[socketCount++] = ptr;
	b_login = true;

	return TRUE;
}

// 소켓 정보 삭제
void RemoveInfo(int nIndex)
{
	SOCKETINFO *ptr = SockArray[nIndex];

	// 종료한 클라이언트 정보 출력
	SOCKADDR_IN clientAddr;
	int addrlen = sizeof(clientAddr);
	getpeername(ptr->sock, (SOCKADDR *)&clientAddr, &addrlen);
	printf("클라이언트 종료: [%s]:%d\n",
		inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

	closesocket(ptr->sock);
	delete ptr;

	if (nIndex != (socketCount - 1))
		SockArray[nIndex] = SockArray[socketCount - 1];

	--socketCount;
}

bool CheckUserName(SOCKETINFO *ptr, int retval, int index) {
	for (int i = 0; i < socketCount; i++) {
		SOCKETINFO *pInfo = SockArray[i];
		if (!strcmp(pInfo->name, ptr->name) && pInfo->room == ptr->room && pInfo->ID != ptr->ID) {
			sprintf(chatMsg, "같은 이름의 접속자가 있습니다. 닉네임을 바꿔주세요");
			retval = send(ptr->sock, (char *)&chatMsg, BUFSIZE, 0);
			RemoveInfo(index);
			return true;
		}
	}
	return false;
}

void Initialize() {
	b_login = false;
	b_show = false;
	b_firstRoom = false;
	b_secondRoom = false;
}