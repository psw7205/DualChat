
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

// ���� ���� ������ ���� ����ü�� ����
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

// ���� ���� �Լ�
BOOL AddInfo(SOCKET sock);
void RemoveInfo(int nIndex);
bool CheckUserName(SOCKETINFO *ptr, int retval, int index);
void Initialize();

// ���� �Լ� ���� ��� �� ����
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

// ���� �Լ� ���� ���
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

	// ���� �ʱ�ȭ
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

	// �ͺ��ŷ �������� ��ȯ
	u_long on = 1;
	retval = ioctlsocket(sock, FIONBIO, &on);
	if (retval == SOCKET_ERROR) err_display(const_cast<char*>("ioctlsocket()"));

	// ������ ��ſ� ����� ����(����)
	FD_SET rset, wset;
	SOCKET clientSock;
	int len, i, j;
	SOCKADDR_IN clientAddr;

	while (1) {
		// ���� �� �ʱ�ȭ
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

		// ���� �� �˻�(1): Ŭ���̾�Ʈ ���� ����
		if (FD_ISSET(sock, &rset)) {
			len = sizeof(clientAddr);
			clientSock = accept(sock, (SOCKADDR *)&clientAddr, &len);
			if (clientSock == INVALID_SOCKET) {
				err_display(const_cast<char*>("accept()"));
				break;
			}
			else {
				// ������ Ŭ���̾�Ʈ ���� ���
				printf("##Ŭ���̾�Ʈ ����: %s:%d\n",
					inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
				// ���� ���� �߰�
				AddInfo(clientSock);
			}
		}

		// ���� �� �˻�(2): ������ ���
		for (i = 0; i < socketCount; i++) 
		{
			SOCKETINFO *ptr = SockArray[i];

			if (FD_ISSET(ptr->sock, &rset)) 
			{
				// ������ �ޱ�
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

				������ ó��

				*/

				if (FD_ISSET(ptr->sock, &wset))
				{
					// ������ ������
					for (j = 0; j < socketCount; j++) 
					{  // ���� �����ڿ��� �߼�
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

// ���� ���� �߰�
BOOL AddInfo(SOCKET sock)
{
	if (socketCount >= FD_SETSIZE) {
		printf("[����] ���� ������ �߰��� �� �����ϴ�!\n");
		return FALSE;
	}

	SOCKETINFO *ptr = new SOCKETINFO;
	if (ptr == NULL) {
		printf("[����] �޸𸮰� �����մϴ�!\n");
		return FALSE;
	}
	//�ð��� ���� ���� UserID�� ����
	srand((unsigned int)time(NULL));
	userID = rand();

	ptr->sock = sock;
	ptr->recvbytes = 0;
	ptr->ID = userID;
	SockArray[socketCount++] = ptr;
	b_login = true;

	return TRUE;
}

// ���� ���� ����
void RemoveInfo(int nIndex)
{
	SOCKETINFO *ptr = SockArray[nIndex];

	// ������ Ŭ���̾�Ʈ ���� ���
	SOCKADDR_IN clientAddr;
	int addrlen = sizeof(clientAddr);
	getpeername(ptr->sock, (SOCKADDR *)&clientAddr, &addrlen);
	printf("Ŭ���̾�Ʈ ����: [%s]:%d\n",
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
			sprintf(chatMsg, "���� �̸��� �����ڰ� �ֽ��ϴ�. �г����� �ٲ��ּ���");
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