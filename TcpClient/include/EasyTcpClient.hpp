#ifndef _TcpClient_hpp_
#define _TcpClient_hpp_
#ifdef _WIN32
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <WinSock2.h>
#pragma comment(lib,"ws2_32.lib")
#else
#include <unistd.h> //uni std
#include <arpa/inet.h>
#include <string.h>
#include "stdio.h"
#define SOCKET int
#define INVALID_SOCKET   (SOCKET)(~0)
#define SOCKET_ERROR             (-1)
#endif
#include "MessageHeader.hpp"

//缓冲区最小单元大小
#ifndef RECV_BUFF_SIZE
#define RECV_BUFF_SIZE  10240
#endif

class EasyTcpClient
{
public:
	EasyTcpClient() {
		_sd = INVALID_SOCKET;
		initSocket();
		if (INVALID_SOCKET == _sd) {
			printf("error，connect socket fail...\n");
		}
	}
	/*虚析构函数*/
	virtual ~EasyTcpClient() {
		Close();
	}

	/*初始化socket*/
	void initSocket() {
#ifdef _WIN32
		/*启动Windows socket 2.x环境*/
		//版本号
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		//socket网络编程启动函数
		WSAStartup(ver, &dat);
#endif
		if (INVALID_SOCKET != _sd) {
			printf("close old socket = %d...\n", _sd);
			//如果有链接先关闭
			Close();
		}
		//建立一个socket
		_sd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	}

	/*链接服务器*/
	//此处加一个常量标识符 为了避免在linux中的报错
	int Connect(const char* ip, unsigned short port) {
		sockaddr_in _sin = {};//初始化sockaddr_in结构体
		_sin.sin_family = AF_INET;//ipv4
		_sin.sin_port = htons(port);//将网络转换为成无符号类型
#ifdef _WIN32
		_sin.sin_addr.S_un.S_addr = inet_addr(ip);
#else
		_sin.sin_addr.s_addr = inet_addr(ip);
#endif
		//连接服务器 connect
		int ret = connect(_sd, (sockaddr*)&_sin, sizeof(sockaddr_in)); //使用sizeof(sockaddr_in)类型更安全
		if (SOCKET_ERROR == ret) {
			printf("error, socket=%d connect server ip:%s port:%d fail...\n", _sd, ip, port);
		}

		return ret;
	}

	/*关闭socket*/
	void Close()
	{
		//防止重复调用
		if (_sd != INVALID_SOCKET) {
			//关闭win socket 2.x环境
#ifdef _WIN32
			closesocket(_sock);//windows下使用
			WSACleanup();
#else
			close(_sd);
#endif
			_sd = INVALID_SOCKET;
		}
	}

	//int _nCount = 0;
	/*处理网络信息*/
	bool OnRun()
	{
		if (isRun())
		{
			fd_set fdReads;
			FD_ZERO(&fdReads);
			FD_SET(_sd, &fdReads);
			timeval t = { 0, 0 };
			int ret = select(_sd + 1, &fdReads, 0, 0, &t);
			//printf("select ret =  %d count = %d\n", ret, _nCount++);
			if (ret < 0)
			{
				printf("<socket = %d>任务结束1！！！\n", _sd);
				Close();
				return false;
			}
			if (FD_ISSET(_sd, &fdReads))
			{
				FD_CLR(_sd, &fdReads);
				if (RecvData(_sd) == -1)
				{
					printf("<socket = %d>任务结束2！！！\n", _sd);
					Close();
					return false;
				}
			}
			return true;
		}
		return false;
	}

	bool isRun() {
		return _sd != INVALID_SOCKET;
	}

	//接受缓冲区
	char _szRecv[RECV_BUFF_SIZE] = {};
	//第二缓冲区 消息缓冲区
	char _szMsgBuf[RECV_BUFF_SIZE * 10] = {};
	//消息缓冲区的数据尾部位置
	int _lastPos = 0;
	/*接收数据 处理粘包 拆分包*/
	int RecvData(SOCKET cSocket)
	{
		//5 接收客户端请求数据
		//recv的第一个参数是指明套接字 第二个参数代表缓冲区 第三个参数代表该缓冲区大小
		int nLen = (int)recv(cSocket, _szRecv, RECV_BUFF_SIZE, 0);
		if (nLen <= 0)
		{
			printf("<socket = %d>与服务器断开连接, 任务结束。\n", cSocket);
			return -1;
		}
		//将收取的数据拷贝到消息缓冲区
		memcpy(_szMsgBuf + _lastPos, _szRecv, nLen);
		//消息缓冲区的数据尾部位置后移
		_lastPos += nLen;
		//判断消息缓冲区的数据长度大于消息头DataHeader长度
		while (_lastPos >= sizeof(DataHeader))//循环是为了解决粘包
		{
			//这时就可以知道当前消息的长度
			DataHeader* header = (DataHeader*)_szMsgBuf;
			//判断消息缓冲区的数据长度大于消息长度
			if (_lastPos >= header->dataLength)
			{
				//剩余未处理消息缓冲区数据的长度
				int nSize = _lastPos - header->dataLength;
				//处理网络消息
				onNetMsg(header);
				//将消息缓冲区剩余未处理数据前移
				memcpy(_szMsgBuf, _szMsgBuf + header->dataLength, _lastPos - header->dataLength);
				//消息缓冲区的尾部位置前移
				_lastPos = nSize;
			}
			else
			{
				//消息缓冲区剩余数据不够一条完整消息
				break;
			}
		}
		//printf("nLen = %d\n", nLen);
		//拆包 和 分包
		/*拆包和分包的作用主要是用在服务端接受数据时一次接受数据过长 和 过短的情况*/
		/*	DataHeader* header = (DataHeader*)szRecv;
		if (nLen <= 0)
		{
		printf("<socket = %d>与服务器断开连接, 任务结束。\n", _cSocket);
		return -1;
		}
		recv(_cSocket, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		onNetMsg(header);*/
		return 0;
	}

	/*响应*/
	virtual void onNetMsg(DataHeader* header)
	{
		//printf("收到命令: %d 数据长度：%d\n", header.cmd, header.dataLength);
		/*判断所收到的数据*/ //多客户端进行收发数据的情况下使用
		switch (header->cmd)
		{
		case CMD_LOGIN_RESULT:
		{
			LoginResult* login = (LoginResult*)header;
			//printf("<socket = %d>收到服务端消息:CMD_LOGIN_RESULT 数据长度：%d \n", _sock, login->dataLength);
		}
		break;
		case CMD_LOGINOUT_RESULT:
		{
			LoginOutResult* loginout = (LoginOutResult*)header;
			//printf("<socket = %d>收到服务端消息:CMD_LOGINOUT_RESULT 数据长度：%d \n", _sock, loginout->dataLength);
		}
		break;
		case CMD_NEW_USER_JOIN:
		{
			NewUserJoin* userJoin = (NewUserJoin*)header;
			//printf("<socket = %d>收到服务端消息:CMD_NEW_USER_JOIN 数据长度：%d \n", _sock, userJoin->dataLength);
		}
		break;
		case CMD_ERROR:
		{
			printf("<socket = %d>收到服务端消息:CMD_ERROR 数据长度：%d \n", _sd, header->dataLength);
		}
		break;
		default:
		{
			printf("<socket = %d>收到未定义消息 数据长度：%d \n", _sd, header->dataLength);
		}
		break;
		}
	}

	//发送数据
	int SendData(DataHeader* header)
	{
		if (isRun() && header)
		{
			return send(_sd, (const char*)header, header->dataLength, 0);
		}
		return SOCKET_ERROR;
	}

private:
	SOCKET _sd;
};
#endif
