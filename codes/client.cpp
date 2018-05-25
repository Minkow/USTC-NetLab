#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <process.h>
using namespace std;
#define SERVER_PORT 5208 //侦听端口

#pragma comment(lib,"wsock32.lib")

char headMsg[65535];
char username[65535];

SOCKET sClient; //连接套接字

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

//接收线程
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
	struct sockaddr_in saServer; //地址信息
	char *ptr;
	BOOL fSuccess = TRUE;
	//WinSock初始化
	wVersionRequested = MAKEWORD(2, 2); //希望使用的WinSock DLL的版本
	ret = WSAStartup(wVersionRequested, &wsaData);
	if(ret!=0)
	{
		printf("WSAStartup() failed!\n");
		return -1;
	}
	//确认WinSock DLL支持版本2.2
	if(LOBYTE(wsaData.wVersion)!=2 || HIBYTE(wsaData.wVersion)!=2)
	{
		WSACleanup();
		printf("Invalid WinSock version!\n");
		return -1;
	}
	//创建Socket,使用TCP协议
	sClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sClient == INVALID_SOCKET)
	{
		WSACleanup();
		printf("socket() failed!\n");
		return -1;
	}
	//构建服务器地址信息
	saServer.sin_family = AF_INET; //地址家族
	saServer.sin_port = htons(SERVER_PORT); //注意转化为网络节序
	saServer.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	//连接服务器
	ret = connect(sClient, (struct sockaddr *)&saServer, sizeof(saServer));
	if (ret == SOCKET_ERROR)
	{
		printf("connect() failed!\n");
		closesocket(sClient); //关闭套接字
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

	closesocket(sClient); //关闭套接字
	WSACleanup();
}
