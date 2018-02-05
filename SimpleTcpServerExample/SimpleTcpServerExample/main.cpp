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

	//SOCKADDR : ���� �ּ�ü�迡�� ��밡��.
	//SOCKADDR_IN : IPv4������ ��� ����. IP, PORT NUMBER ����ϱ� ������.
	//bind()
	SOCKADDR_IN serverAddr;
	ZeroMemory(&serverAddr, sizeof(serverAddr));	//serverAddr�� ������ 0���� �ʱ�ȭ. memset�� �ᵵ ����
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(9000);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);	//INADDER_ANY�� �ʱ�ȭ : �������� ������ ���� ��û���� �������� ó���ϰڴٴ� �ǹ�. �̽ļ� Up
	retval = bind(listenSocket, reinterpret_cast<SOCKADDR*>(&serverAddr), sizeof(serverAddr));

	//listen()
	//listen(SOCKET s, int backlog) -> backlog�� ������ ���� ó������ �ʴ��� ���� ������ Ŭ���̾�Ʈ ����.
	//Ŭ���̾�Ʈ�� ���������� ť�� ����Ǹ�, backlog�� �� ����ť�� ���̸� ��Ÿ��.
	//SOMAXCONN : �Ϻ� �������ݿ��� ���������� �ִ밪 ���
	//backlog���� �����Ϸ��� �ٽ� listen�Լ��� ȣ���ϸ� �ȴ�.
	retval = listen(listenSocket, SOMAXCONN);	
	if (retval == SOCKET_ERROR)
		ErrorQuit("listen()");

	//������ ��ſ� ����� ����
	SOCKET clientSocket;	//Ŭ���̾�Ʈ�� ����ϱ� ���� ����.
	SOCKADDR_IN clientAddr;		//Ŭ���̾�Ʈ�� �ּ�, port������ ����ִ� �ּҺ���
	int addrLen;		//Ŭ���̾�Ʈ�� IP�ּ� ���̸� �����ϴ� ����
	char buf[SIZE + 1];		//������ �ۼ��ſ� ����� �ۼ��� ����.

	while (1) {
		//accept()
		//client�� ����ϱ� ���� ���ο� socket�� ����, ������.
		//������ client�� �ּҿ� ��Ʈ��ȣ�� �˷���.(�Ű������� SOCKADDR or SOCKADDR_IN ������ �ּҰ��� IP�ּ� ���̸� ���� ������ �ּҰ��� �Ѱ������.)
		//client�� �ּҿ� ��Ʈ��ȣ�� �� �ʿ䰡 ���� ��� �Ѵ� NULL�� �Ѱ��ָ� �ȴ�.
		//������ client�� ���� ��� Server�� �̺κп��� Wait���·� ����.
		addrLen = sizeof(clientAddr);
		//�����û�� Ŭ���̾�Ʈ�� ������ ������� ����, IP�ּ�, Port�� �����´�.
		clientSocket = accept(listenSocket, reinterpret_cast<SOCKADDR*>(&clientAddr), &addrLen);
		if (clientSocket == INVALID_SOCKET) {
			ErrorDisplay("accpet()");
			continue;
		}

		printf("\n[TCP Server] Ŭ���̾�Ʈ ���� : IP�ּ� = %s, ��Ʈ��ȣ = %d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

		//Ŭ���̾�Ʈ�� ������ ���
		while (1) {
			//������ �ޱ�
			//retval : Ŭ���̾�Ʈ�κ��� ���� �������� ũ�⸦ �޴� ����
			retval = recv(clientSocket, buf, SIZE, 0);
			if (retval == SOCKET_ERROR) {
				ErrorDisplay("recv()");
				break;
			}
			else if (retval == 0)
				break;

			//���� ������ ���
			buf[retval] = '\0';
			printf("[TCP/%s:%d] %s\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port), buf);
			
			//������ ����
			//retval : ������ �������� ũ��
			retval = send(clientSocket, buf, retval, 0);
			if (retval = SOCKET_ERROR) {
				ErrorDisplay("send()");
				break;
			}

		}

		//Data transfer socket close
		closesocket(clientSocket);
		printf("[TCP Server] Ŭ���̾�Ʈ ���� : IP�ּ� = %s, ��Ʈ��ȣ = %d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

	}

	//listen socket close
	closesocket(listenSocket);

	//Winsock end.
	WSACleanup();

	return 0;
}