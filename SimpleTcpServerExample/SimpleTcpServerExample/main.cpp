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


int main(int argc, char** argv) {

	int retval;

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		puts("Winsock init error!");
		exit(-1);
	}

	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenSocket == INVALID_SOCKET) {
		puts("Invalid socket error!");
		exit(-1);
	}

	//SOCKADDR : 여러 주소체계에서 사용가능.
	//SOCKADDR_IN : IPv4에서만 사용 가능. IP, PORT NUMBER 사용하기 용이함.
	//bind()
	SOCKADDR_IN serverAddr;
	ZeroMemory(&serverAddr, sizeof(serverAddr));	//serverAddr의 값들을 0으로 초기화. memset을 써도 무방
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(9000);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);	//INADDER_ANY로 초기화 : 서버로의 임의의 연결 요청들을 서버에서 처리하겠다는 의미. 이식성 Up
	retval = bind(listenSocket, reinterpret_cast<SOCKADDR*>(&serverAddr), sizeof(serverAddr));

	//listen()
	//listen(SOCKET s, int backlog) -> backlog는 서버가 당장 처리하지 않더라도 접속 가능한 클라이언트 개수.
	//클라이언트의 접속정보는 큐에 저장되며, backlog는 이 연결큐의 길이를 나타냄.
	//SOMAXCONN : 하부 프로토콜에서 지원가능한 최대값 사용
	//backlog값을 변경하려면 다시 listen함수를 호출하면 된다.
	retval = listen(listenSocket, SOMAXCONN);	
	if (retval == SOCKET_ERROR)
		ErrorQuit("listen()");

	//데이터 통신에 사용할 변수
	SOCKET clientSocket;	//클라이언트와 통신하기 위한 소켓.
	SOCKADDR_IN clientAddr;		//클라이언트의 주소, port정보를 담고있는 주소변수
	int addrLen;		//클라이언트의 IP주소 길이를 저장하는 변수
	char buf[SIZE + 1];		//데이터 송수신에 사용한 송수신 버퍼.

	while (1) {
		//accept()
		//client와 통신하기 위한 새로운 socket을 생성, 리턴함.
		//접속한 client의 주소와 포트번호를 알려줌.(매개변수로 SOCKADDR or SOCKADDR_IN 변수의 주소값과 IP주소 길이를 담을 변수의 주소값을 넘겨줘야함.)
		//client의 주소와 포트번호를 알 필요가 없는 경우 둘다 NULL로 넘겨주면 된다.
		//접속한 client가 없는 경우 Server는 이부분에서 Wait상태로 돌입.
		addrLen = sizeof(clientAddr);
		//연결요청한 클라이언트의 정보를 담기위한 소켓, IP주소, Port를 가져온다.
		clientSocket = accept(listenSocket, reinterpret_cast<SOCKADDR*>(&clientAddr), &addrLen);
		if (clientSocket == INVALID_SOCKET) {
			ErrorDisplay("accpet()");
			continue;
		}

		printf("\n[TCP Server] 클라이언트 접속 : IP주소 = %s, 포트번호 = %d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

		//클라이언트와 데이터 통신
		while (1) {
			//데이터 받기
			//retval : 클라이언트로부터 받은 데이터의 크기를 받는 변수
			retval = recv(clientSocket, buf, SIZE, 0);
			if (retval == SOCKET_ERROR) {
				ErrorDisplay("recv()");
				break;
			}
			else if (retval == 0)
				break;

			//받은 데이터 출력
			buf[retval] = '\0';
			printf("[TCP/%s:%d] %s\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port), buf);
			
			//데이터 전송
			//retval : 전송한 데이터의 크기
			retval = send(clientSocket, buf, retval, 0);
			if (retval = SOCKET_ERROR) {
				ErrorDisplay("send()");
				break;
			}

		}

		//Data transfer socket close
		closesocket(clientSocket);
		printf("[TCP Server] 클라이언트 종료 : IP주소 = %s, 포트번호 = %d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

	}

	//listen socket close
	closesocket(listenSocket);

	//Winsock end.
	WSACleanup();

	return 0;
}