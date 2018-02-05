#pragma comment(lib, "ws2_32")
#include <WinSock2.h>
#include <iostream>

#define SIZE 512

void ErrorQuit(char* msg) {
	LPVOID lpMsgBuf;

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);

	LocalFree(lpMsgBuf);
	exit(-1);
}

void ErrorDisplay(char* msg) {
	LPVOID lpMsgBuf;

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);

	printf("[%s] %s", msg, (LPCTSTR)lpMsgBuf);
	LocalFree(lpMsgBuf);

}

int recvn(SOCKET s, char* buf, int len, int flags) {
	int received;
	char* ptr = buf;
	int left = len;

	while (left > 0) {
		received = recv(s, ptr, left, flags);
		if (received == SOCKET_ERROR)
			return SOCKET_ERROR;
		else if (received == 0)
			break;

		left -= received;
		ptr += received;

		return (len - left);
	}

}

int main(int argc, char** argv) {

	int retval;

	//Winsock Init
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		puts("Winsock init error!");
		exit(-1);
	}

	//socket()
	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET)
		ErrorQuit("socket()");

	//connect()
	SOCKADDR_IN serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = ntohs(9000);
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	//bind()함수를 따로 호출하지 않고 사용 -> OS에서 자동으로 지역IP와 Port 할당해줌.
	retval = connect(sock, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
	if (retval == SOCKET_ERROR)
		ErrorQuit("connect()");

	//데이터 통신에 사용할 변수
	char buf[SIZE + 1];
	int len;

	while (1) {
		//데이터 입력
		ZeroMemory(buf, sizeof(buf));
		printf("\n[보낼 데이터]");
		if (fgets(buf, SIZE, stdin) == NULL)
			break;

		//'\n'문자 제거
		len = strlen(buf);
		if (buf[len - 1] == '\n')
			buf[len - 1] = '\0';
		if (strlen(buf) == 0)
			break;

		//데이터 전송
		retval = send(sock, buf, len, 0);
		if (retval == SOCKET_ERROR) {
			ErrorDisplay("send()");
			break;
		}
		printf("[TCP Client] %d바이트를 전송했습니다.\n", retval);

		//데이터 받기
		retval = recvn(sock, buf, retval, 0);
		if (retval == SOCKET_ERROR) {
			ErrorDisplay("recv()");
			break;
		}
		else if (retval == 0)
			break;
		
		buf[retval] = '\0';
		printf("[TCP Client] %d바이트를 수신했습니다.\n", retval);
		printf("[Received Data] %s\n", buf);
		
	}

	//closeSocket
	closesocket(sock);

	//Winsock end
	WSACleanup();

	return 0;
	
}