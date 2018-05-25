#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <process.h>
using namespace std;
#define SERVER_PORT 5208 //�����˿�

#pragma comment(lib,"wsock32.lib")

char headMsg[65535];
char username[65535];

SOCKET sClient; //�����׽���

UINT __stdcall SendThread(LPVOID lParam)
{
	char sendMsg[65535];
	while(1)
	{
		ZeroMemory(sendMsg,sizeof(sendMsg));
		ZeroMemory(headMsg,sizeof(headMsg));
		sprintf(headMsg,"%s : ",username);
		gets(sendMsg);
		strcat(headMsg,sendMsg);
		int ret = send (sClient, (char *)&headMsg, sizeof(headMsg), 0);
		if (ret == SOCKET_ERROR)
		{
			printf("send failed!\n");
		}
	}
	return 1;
}

//�����߳�
UINT __stdcall RecvThread(LPVOID lParam)
{
	while(1)
	{
		char recvMsg[65535];
		ZeroMemory(recvMsg, sizeof(recvMsg));
		int ret;
		ret = recv(sClient,recvMsg,sizeof(recvMsg),0);
		if(ret == 0)
		{
			printf("receive failed!\n");
			return -1;
		}
		if (ret > 0)
		{
			printf("%s\n",recvMsg);
		}
	}
	return 1;
}

int main()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int ret;
	struct sockaddr_in saServer; //��ַ��Ϣ
	char *ptr;
	BOOL fSuccess = TRUE;
	//WinSock��ʼ��
	wVersionRequested = MAKEWORD(2, 2); //ϣ��ʹ�õ�WinSock DLL�İ汾
	ret = WSAStartup(wVersionRequested, &wsaData);
	if(ret!=0)
	{
		printf("WSAStartup() failed!\n");
		return -1;
	}
	//ȷ��WinSock DLL֧�ְ汾2.2
	if(LOBYTE(wsaData.wVersion)!=2 || HIBYTE(wsaData.wVersion)!=2)
	{
		WSACleanup();
		printf("Invalid WinSock version!\n");
		return -1;
	}
	//����Socket,ʹ��TCPЭ��
	sClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sClient == INVALID_SOCKET)
	{
		WSACleanup();
		printf("socket() failed!\n");
		return -1;
	}
	//������������ַ��Ϣ
	saServer.sin_family = AF_INET; //��ַ����
	saServer.sin_port = htons(SERVER_PORT); //ע��ת��Ϊ�������
	saServer.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	//���ӷ�����
	ret = connect(sClient, (struct sockaddr *)&saServer, sizeof(saServer));
	if (ret == SOCKET_ERROR)
	{
		printf("connect() failed!\n");
		closesocket(sClient); //�ر��׽���
		WSACleanup();
		return -1;
	}
	else printf("You have entered the chatting room!\nPlease input your username: ");
	gets(username);
	_beginthreadex(NULL,NULL,RecvThread,NULL,NULL,NULL);
	_beginthreadex(NULL,NULL,SendThread,NULL,NULL,NULL);
	
	while(1)
	{
		Sleep(1);
	}

	closesocket(sClient); //�ر��׽���
	WSACleanup();
}
