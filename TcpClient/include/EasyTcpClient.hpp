#ifndef _TcpClient_hpp_
#define _TcpClient_hpp_
#ifdef _WIN32
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <WinSock2.h>
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

//��������С��Ԫ��С
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
			printf("error��connect socket fail...\n");
		}
	}
	/*����������*/
	virtual ~EasyTcpClient() {
		Close();
	}

	/*��ʼ��socket*/
	void initSocket() {
#ifdef _WIN32
		/*����Windows socket 2.x����*/
		//�汾��
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		//socket��������������
		WSAStartup(ver, &dat);
#endif
		if (INVALID_SOCKET != _sd) {
			printf("close old socket = %d...\n", _sd);
			//����������ȹر�
			Close();
		}
		//����һ��socket
		_sd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	}

	/*���ӷ�����*/
	//�˴���һ��������ʶ�� Ϊ�˱�����linux�еı���
	int Connect(const char* ip, unsigned short port) {
		sockaddr_in _sin = {};//��ʼ��sockaddr_in�ṹ��
		_sin.sin_family = AF_INET;//ipv4
		_sin.sin_port = htons(port);//������ת��Ϊ���޷�������
#ifdef _WIN32
		_sin.sin_addr.S_un.S_addr = inet_addr(ip);
#else
		_sin.sin_addr.s_addr = inet_addr(ip);
#endif
		//���ӷ����� connect
		int ret = connect(_sd, (sockaddr*)&_sin, sizeof(sockaddr_in)); //ʹ��sizeof(sockaddr_in)���͸���ȫ
		if (SOCKET_ERROR == ret) {
			printf("error, socket=%d connect server ip:%s port:%d fail...\n", _sd, ip, port);
		}

		return ret;
	}

	/*�ر�socket*/
	void Close()
	{
		//��ֹ�ظ�����
		if (_sd != INVALID_SOCKET) {
			//�ر�win socket 2.x����
#ifdef _WIN32
			closesocket(_sock);//windows��ʹ��
			WSACleanup();
#else
			close(_sd);
#endif
			_sd = INVALID_SOCKET;
		}
	}

	//int _nCount = 0;
	/*����������Ϣ*/
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
				printf("<socket = %d>�������1������\n", _sd);
				Close();
				return false;
			}
			if (FD_ISSET(_sd, &fdReads))
			{
				FD_CLR(_sd, &fdReads);
				if (RecvData(_sd) == -1)
				{
					printf("<socket = %d>�������2������\n", _sd);
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

	//���ܻ�����
	char _szRecv[RECV_BUFF_SIZE] = {};
	//�ڶ������� ��Ϣ������
	char _szMsgBuf[RECV_BUFF_SIZE * 10] = {};
	//��Ϣ������������β��λ��
	int _lastPos = 0;
	/*�������� ����ճ�� ��ְ�*/
	int RecvData(SOCKET cSocket)
	{
		//5 ���տͻ�����������
		//recv�ĵ�һ��������ָ���׽��� �ڶ����������������� ���������������û�������С
		int nLen = (int)recv(cSocket, _szRecv, RECV_BUFF_SIZE, 0);
		if (nLen <= 0)
		{
			printf("<socket = %d>��������Ͽ�����, ���������\n", cSocket);
			return -1;
		}
		//����ȡ�����ݿ�������Ϣ������
		memcpy(_szMsgBuf + _lastPos, _szRecv, nLen);
		//��Ϣ������������β��λ�ú���
		_lastPos += nLen;
		//�ж���Ϣ�����������ݳ��ȴ�����ϢͷDataHeader����
		while (_lastPos >= sizeof(DataHeader))//ѭ����Ϊ�˽��ճ��
		{
			//��ʱ�Ϳ���֪����ǰ��Ϣ�ĳ���
			DataHeader* header = (DataHeader*)_szMsgBuf;
			//�ж���Ϣ�����������ݳ��ȴ�����Ϣ����
			if (_lastPos >= header->dataLength)
			{
				//ʣ��δ������Ϣ���������ݵĳ���
				int nSize = _lastPos - header->dataLength;
				//����������Ϣ
				onNetMsg(header);
				//����Ϣ������ʣ��δ��������ǰ��
				memcpy(_szMsgBuf, _szMsgBuf + header->dataLength, _lastPos - header->dataLength);
				//��Ϣ��������β��λ��ǰ��
				_lastPos = nSize;
			}
			else
			{
				//��Ϣ������ʣ�����ݲ���һ��������Ϣ
				break;
			}
		}
		//printf("nLen = %d\n", nLen);
		//��� �� �ְ�
		/*����ͷְ���������Ҫ�����ڷ���˽�������ʱһ�ν������ݹ��� �� ���̵����*/
		/*	DataHeader* header = (DataHeader*)szRecv;
		if (nLen <= 0)
		{
		printf("<socket = %d>��������Ͽ�����, ���������\n", _cSocket);
		return -1;
		}
		recv(_cSocket, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		onNetMsg(header);*/
		return 0;
	}

	/*��Ӧ*/
	virtual void onNetMsg(DataHeader* header)
	{
		//printf("�յ�����: %d ���ݳ��ȣ�%d\n", header.cmd, header.dataLength);
		/*�ж����յ�������*/ //��ͻ��˽����շ����ݵ������ʹ��
		switch (header->cmd)
		{
		case CMD_LOGIN_RESULT:
		{
			LoginResult* login = (LoginResult*)header;
			//printf("<socket = %d>�յ��������Ϣ:CMD_LOGIN_RESULT ���ݳ��ȣ�%d \n", _sock, login->dataLength);
		}
		break;
		case CMD_LOGINOUT_RESULT:
		{
			LoginOutResult* loginout = (LoginOutResult*)header;
			//printf("<socket = %d>�յ��������Ϣ:CMD_LOGINOUT_RESULT ���ݳ��ȣ�%d \n", _sock, loginout->dataLength);
		}
		break;
		case CMD_NEW_USER_JOIN:
		{
			NewUserJoin* userJoin = (NewUserJoin*)header;
			//printf("<socket = %d>�յ��������Ϣ:CMD_NEW_USER_JOIN ���ݳ��ȣ�%d \n", _sock, userJoin->dataLength);
		}
		break;
		case CMD_ERROR:
		{
			printf("<socket = %d>�յ��������Ϣ:CMD_ERROR ���ݳ��ȣ�%d \n", _sd, header->dataLength);
		}
		break;
		default:
		{
			printf("<socket = %d>�յ�δ������Ϣ ���ݳ��ȣ�%d \n", _sd, header->dataLength);
		}
		break;
		}
	}

	//��������
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