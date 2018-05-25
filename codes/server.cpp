#include <stdio.h>
#include <winsock2.h>
#include <windows.h>
#include <process.h>

using namespace std;
#define SERVER_PORT 5208 //侦听端口

#pragma comment(lib,"wsock32.lib")

int ret, nLeft, length, usernum=0, tranflag=0, trans;
SOCKET sListen, sServer[65535]; //侦听套接字，连接套接字
struct sockaddr_in saServer, saClient[65535]; //地址信息
char tranMsg[65535];

UINT __stdcall RecvThread(LPVOID lParam)
{
	char recvMsg[65535];
	char userinfo[65535];
	int ret;
	int i=usernum-1;
	while(1)
	{
		ZeroMemory(recvMsg, sizeof(recvMsg));		
		ret = recv(sServer[i],recvMsg,sizeof(recvMsg),0);
		if(ret == 0)
		{
			printf("Receive failed\n");
			return -1;
		}
		if (ret > 0)
		{
			ZeroMemory(tranMsg, sizeof(tranMsg));
			strcpy(tranMsg,recvMsg);
			tranflag=1;
			trans=i;
			printf("%s\n",recvMsg);
		}
	}
	return 1;
}


UINT __stdcall ListenThread(LPVOID lParam)//监听线程
{
	while(1)
	{
		int length = sizeof(saClient[usernum]);
		sServer[usernum] = accept(sListen,(struct sockaddr*)&saClient[usernum],&length);
		if (sServer[usernum] == INVALID_SOCKET)
		{
			printf("accept() faild! code:%d\n", WSAGetLastError());
			closesocket(sListen); //关闭套接字
			WSACleanup();
			return -1;
		}
		else
		{
			printf("Client %d has entered the chatting room!\n",++usernum);
			_beginthreadex(NULL,NULL,RecvThread,NULL,NULL,NULL);
		}
	}
	return 1;
}

UINT __stdcall SendThread(LPVOID lParam)
{
	char sendMsg[65535];
	char serinfo[65535];
	ZeroMemory(sendMsg,sizeof(sendMsg));
	while(1)
	{
		gets(sendMsg);
		sprintf(serinfo,"Server : ");
		strcat(serinfo,sendMsg);
		for(int i=0; i<usernum; i++)
		{
			if(send(sServer[i],serinfo,sizeof(serinfo),0) == SOCKET_ERROR)
			{
				printf("Send failed\n");
				return -1;
			}
		}
	}
	return 1;
}

UINT __stdcall TranThread(LPVOID lParam)
{
	while(1)
	{
		if(tranflag)
		{
			for(int i=0; i<usernum; i++)
			{
				if(strlen(tranMsg)>0&&i!=trans)
				{
					if(send(sServer[i],tranMsg,sizeof(tranMsg),0) == SOCKET_ERROR)
					{
						printf("Send failed\n");
						return -1;
					}					
				}
			}
		tranflag=0;
		}
	}
	return 1;
}

int main()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	char *ptr;//用于遍历信息的指针
	//WinSock初始化
	wVersionRequested=MAKEWORD(2, 2); //希望使用的WinSock DLL 的版本
	ret=WSAStartup(wVersionRequested, &wsaData);
	if(ret!=0)
	{
		printf("WSAStartup() failed!\n");
		return -1;
	}
	//创建Socket,使用TCP协议
	sListen=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sListen == INVALID_SOCKET)
	{
		WSACleanup();
		printf("socket() faild!\n");
		return -1;
	}
	//构建本地地址信息
	saServer.sin_family = AF_INET; //地址家族
	saServer.sin_port = htons(SERVER_PORT); //注意转化为网络字节序
	saServer.sin_addr.S_un.S_addr = htonl(INADDR_ANY); //使用INADDR_ANY 指示任意地址

	//绑定
	ret = bind(sListen, (struct sockaddr *)&saServer, sizeof(saServer));
	if (ret == SOCKET_ERROR)
	{
		printf("bind() faild! code:%d\n", WSAGetLastError());
		closesocket(sListen); //关闭套接字
		WSACleanup();
		return -1;
	}

	//侦听连接请求
	ret = listen(sListen, 5);
	if (ret == SOCKET_ERROR)
	{
		printf("listen() faild! code:%d\n", WSAGetLastError());
		closesocket(sListen); //关闭套接字
		return -1;
	}

	printf("Waiting for client connecting!\n");
	printf("Tips: Ctrl+c to quit!\n");
	//阻塞等待接受客户端连接

	_beginthreadex(NULL,NULL,ListenThread,NULL,NULL,NULL);
	_beginthreadex(NULL,NULL,SendThread,NULL,NULL,NULL);
	_beginthreadex(NULL,NULL,TranThread,NULL,NULL,NULL);

	while(1)
	{
		Sleep(1);
	}
	
	closesocket(sListen);
	for(int i=0; i<usernum; i++)
		closesocket(sServer[usernum]);
	WSACleanup();
}
