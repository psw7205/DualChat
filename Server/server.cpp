
#pragma warning(disable:4996)
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string>

#define SERVERPORT	9000
#define BUFSIZE		1024
#define NAMESIZE	32

// 메세지 TYPE
#define FIRSTROOM	11
#define SECONDROOM	12
#define SHOWUSERS	13
#define NAMECHANGE	14
#define INIT		15

using namespace std;

// 소켓 정보 저장을 위한 구조체와 변수
struct MSGDATA
{
	int		type;
	int		ID;
	char	name[2][NAMESIZE];
	char	buf[BUFSIZE];
}chatMsg;

typedef struct SOCKETINFO
{
	SOCKET	sock;
	int		recvbytes;
	int		ID;
	char	name[2][NAMESIZE];
}SOCKETINFO;

int  socketCount;
SOCKETINFO	*SockArray[FD_SETSIZE];

// 소켓 관리 함수
BOOL AddInfo(SOCKET sock);
void RemoveInfo(int nIndex);

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
	int retval;
	memset(&chatMsg, 0, sizeof(chatMsg));

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
	FD_SET rset;
	SOCKET clientSock;
	int len, i, j;
	SOCKADDR_IN clientAddr;

	while (1) {
		// 소켓 셋 초기화
		FD_ZERO(&rset);
		FD_SET(sock, &rset);
		for (i = 0; i < socketCount; ++i) {
			FD_SET(SockArray[i]->sock, &rset);
		}

		// select()
		retval = select(0, &rset, NULL, NULL, NULL);
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

				recv(clientSock, (char*)&chatMsg, sizeof(chatMsg), 0);
				chatMsg.type


				// 소켓 정보 추가
				AddInfo(clientSock);
			}
		}

		// 소켓 셋 검사(2): 데이터 통신
		for (i = 0; i < socketCount; ++i)
		{
			SOCKETINFO *ptr = SockArray[i];

			if (FD_ISSET(ptr->sock, &rset))
			{
				// 데이터 받기
				retval = recv(ptr->sock, (char*)&chatMsg, sizeof(chatMsg), 0);
				printf("%s\n", chatMsg.buf);

				if (retval == 0 || retval == SOCKET_ERROR)
				{
					RemoveInfo(i);
					continue;
				}

				ptr->recvbytes += retval;

				if (ptr->recvbytes == sizeof(chatMsg))
				{
					int idx = 0;
					int size = 0;
					int n;

					ptr->recvbytes = 0;

					switch (chatMsg.type)
					{
					case NAMECHANGE:
						for (j = 0; j < socketCount; ++j)
						{
							SOCKETINFO *ptr2 = SockArray[j];
							if (ptr2->ID == chatMsg.ID)
							{
								idx = j;
								if (!strcmp(chatMsg.name[0], ptr2->name[0]))
									n = 1;
								else
									n = 0;

								break;
							}
						}

						for (j = 0; j < socketCount; ++j)
						{
							if (!strcmp(chatMsg.name[n], SockArray[j]->name[n]))
								break;
						}

						if (j == socketCount)
						{
							strcpy(SockArray[idx]->name[n], chatMsg.name[n]);
							strcpy(chatMsg.buf, "1");
							send(SockArray[idx]->sock, (char*)&chatMsg, sizeof(chatMsg), 0);
						}
						else
						{
							strcpy(chatMsg.buf, "0");
							send(SockArray[idx]->sock, (char*)&chatMsg, sizeof(chatMsg), 0);
						}
						
						break;
					case SHOWUSERS:
						for (j = 0; j < socketCount; ++j)
						{
							SOCKETINFO *ptr2 = SockArray[j];

							if (ptr2->ID == chatMsg.ID)
							{
								idx = j;
								break;
							}
						}

						chatMsg.buf[0] = socketCount;
						send(SockArray[idx]->sock, (char*)&chatMsg, sizeof(chatMsg), 0);
						
						for (j = 0; j < socketCount; ++j)
						{
							SOCKETINFO *ptr2 = SockArray[j];

							strcpy(chatMsg.buf, ptr2->name[0]);
							send(SockArray[idx]->sock, (char*)&chatMsg, sizeof(chatMsg), 0);

							strcpy(chatMsg.buf, ptr2->name[1]);
							send(SockArray[idx]->sock, (char*)&chatMsg, sizeof(chatMsg), 0);
						}

						break;

					default:
						// 현재 접속한 모든 클라이언트에게 데이터를 보냄!
						for (j = 0; j < socketCount; ++j) {
							SOCKETINFO *ptr2 = SockArray[j];
							retval = send(ptr2->sock, (char*)&chatMsg, sizeof(chatMsg), 0);
							if (retval == SOCKET_ERROR) {
								err_display(const_cast<char*>("send()"));
								RemoveInfo(j);
								--j; // 루프 인덱스 보정
								continue;
							}
						}

						memset(&chatMsg, 0, sizeof(chatMsg));
						break;
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

	int retval = recv(sock, (char*)&chatMsg, sizeof(chatMsg), 0);

	ptr->sock = sock;
	ptr->recvbytes = 0;
	ptr->ID = chatMsg.ID;
	strcpy(ptr->name[0], chatMsg.name[0]);
	strcpy(ptr->name[1], chatMsg.name[1]);
	SockArray[socketCount++] = ptr;

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