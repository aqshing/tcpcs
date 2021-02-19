#ifndef _TcpServer_hpp_
#define _TcpServer_hpp_

//socket header file
#ifdef _WIN32
#define FD_SETSIZE	1024
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <WinSock2.h>
#else
#include <unistd.h> //uni std
#include <arpa/inet.h>
#include <string.h>
#define SOCKET int
#define INVALID_SOCKET   (SOCKET)(~0)
#define SOCKET_ERROR             (-1)
#endif

#include "stdio.h"
#include <vector>
#include "MessageHeader.hpp"
#include "iostream"
#include "CELLTimestamp.hpp"

using std::vector;

//��������С��Ԫ��С
#ifndef RECV_BUFF_SIZE
#define RECV_BUFF_SIZE 10240
#endif

//����һ���µ�socketģ��
class ClientSocket {
public:
	ClientSocket(SOCKET sd = INVALID_SOCKET) :_sd(sd){
		memset(_szMsgBuf, 0, sizeof(_szMsgBuf));
		_lastPos = 0;
	}

	inline SOCKET Getsd() const {
		return _sd;
	}
	inline char* msgBuf(){
		return _szMsgBuf;
	}
	inline size_t getLastPos() const {
		return _lastPos;
	}
	inline void setLastPos(int pos) {
		this->_lastPos = pos;
	}
private:
	SOCKET _sd;
	//�ڶ������� ��Ϣ������
	char _szMsgBuf[RECV_BUFF_SIZE * 10];
	//��Ϣ������������β��λ��
	size_t _lastPos;
};

class EasyTcpServer {
public:
	EasyTcpServer() :sd(INVALID_SOCKET), _recvCount(0){
		InitSocket();
		if (INVALID_SOCKET == sd) {
			printf("error��connect socket fail...\n");
		}
	}

	virtual ~EasyTcpServer() {
		Close();
	}

	//��ʼ��Socket
	SOCKET InitSocket() {
#ifdef _WIN32
		/*����Windows socket 2.x����*/
		//�汾��
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		//socket��������������
		WSAStartup(ver, &dat);
#endif
		if (INVALID_SOCKET != sd) {
			printf("close old socket = %d...\n", (int)sd);
			//����������ȹر�
			Close();
		}
		//����һ��socket
		sd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		return sd;
	}

	//��IP �� �˿ں�
	int Bind(const char* ip, unsigned short port) {
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;//ipv4
		_sin.sin_port = htons(port);//�˿ں� ����������������������Ͳ�ͬ �����Ҫ����ת�� ʹ�� host to net unsigned short
#ifdef _WIN32
		if (ip) {
			_sin.sin_addr.S_un.S_addr = inet_addr(ip);
		} else {
			_sin.sin_addr.S_un.S_addr = INADDR_ANY;//inet_addr("127.0.0.1");//��������ip��ַ INADDR_ANY�������е�Ip��ַ�����Է��� һ������
		}
#else
		if (ip) {
			_sin.sin_addr.s_addr = inet_addr(ip);
		} else {
			_sin.sin_addr.s_addr = INADDR_ANY;//inet_addr("127.0.0.1");//��������ip��ַ INADDR_ANY�������е�Ip��ַ�����Է��� һ������
		}
#endif
		int ret = ::bind(sd, (sockaddr*)&_sin, sizeof(_sin));

		//�п��ܰ�ʧ��
		if (SOCKET_ERROR == ret) {
			printf("error, bind inet port:%d fail...\n", port);
		} else {
			printf("bind inet port:%d succeed...\n", port);
		}
		return ret;
	}

	//�����˿ں�
	int Listen(int n = 5) {
		//�����˿�
		int ret = listen(sd, n);
		if (SOCKET_ERROR == ret) {
			printf("error socket = %d��listen port fail...\n", (int)sd);
		} else {
			printf("socket = %d listen port succeed...\n", (int)sd);
		}
		return ret;
	}

	//���տͻ�������
	int Accept()
	{
		//accept �ȴ����ܿͻ�������
		sockaddr_in clientAddr = {};
		int nAddrLen = sizeof(sockaddr_in);
		SOCKET cSock = INVALID_SOCKET;
		//nAddrLen������windows��linux֮�䲻һ����
#ifdef _WIN32
		cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
#else
		cSock = accept(sd, (sockaddr*)&clientAddr, (socklen_t*)&nAddrLen);
#endif
		if (INVALID_SOCKET == cSock) {
			printf("socket<%d>���󣬽��յ���Ч�ͻ���SOCKET...\n", (int)sd);
		} else {
			//NewUserJoin userJoin;
			//SendDataToAll(&userJoin);
			_clients.push_back(new ClientSocket(cSock));
			//printf("socket<%d>�¿ͻ���<%d>����: socket = %d IP = %s \n", (int)_sock, _clients.size(), (int)cSock, inet_ntoa(clientAddr.sin_addr));//inet_ntoaת��Ϊ�ɶ���ַ
		}
		return cSock;
	}

	//�ر�Socket
	void Close()
	{
		if (sd != INVALID_SOCKET)
		{
#ifdef _WIN32
			for (int n = (int)_clients.size() - 1; n >= 0; n--)
			{
				closesocket(_clients[n]->sockfd());
				delete _clients[n];
			}
			closesocket(_sock);
			_sock = INVALID_SOCKET;
			WSACleanup();
#else
			for (int n = (int)_clients.size() - 1; n >= 0; n--)
			{
				close(_clients[n]->Getsd());
				delete _clients[n];
			}
			close(sd);
			sd = INVALID_SOCKET;
#endif
			//����һ��
			_clients.clear();
		}
	}
	//int _nCount = 0;
	//����������Ϣ
	bool OnRun()
	{
		if (isRun())
		{
			//cout << isRun() << endl;
			fd_set fdRead;//������(socket)����
			fd_set fdWrite;
			fd_set fdExp;
			FD_ZERO(&fdRead);//���fd_set�������͵����� ��ʵ���ǽ�fd_count ��Ϊ0
			FD_ZERO(&fdWrite);//������
			FD_ZERO(&fdExp);
			FD_SET(sd, &fdRead);//�����������뼯����
			FD_SET(sd, &fdWrite);
			FD_SET(sd, &fdExp);
			SOCKET maxSock = sd;
			for (int n = (int)_clients.size() - 1; n >= 0; n--)
			{
				FD_SET(_clients[n]->Getsd(), &fdRead);//����ɶ������в�ѯ �Ƿ��пɶ�����
				if (maxSock < _clients[n]->Getsd())
				{
					//�ҵ������������е����������
					maxSock = _clients[n]->Getsd();
				}
			}

			timeval t = { 1, 0 };//ʱ����� &t ���Ϊ1��
			int ret = select(maxSock + 1, &fdRead, &fdWrite, &fdExp, &t);//NULL��������
																		 //printf("select ret =  %d count = %d\n", ret, _nCount ++);
																		 /*_sock + 1�ܹ� ʹ����linux������ʹ��*/													 /*���Ϸ�ʽΪ������ʽ�����û�пͻ��˽��뽫�����ڴ˴�*/
			if (ret < 0)
			{
				printf("<socket = %d>���������\n", sd);
				Close();
				return false;//��ʾ���� ����ѭ��
			}
			//������socket�ɶ��Ļ���ʾ �пͻ����Ѿ����ӽ�����
			if (FD_ISSET(sd, &fdRead))//�ж��������Ƿ��ڼ�����
			{
				//����
				FD_CLR(sd, &fdRead);
				Accept();
				//�����ӵ�ʱ��ȥ�������� �����ӿ������ٶ�
				return true;
			}
			for (int n = (int)_clients.size() - 1; n >= 0; n--)
			{
				if (FD_ISSET(_clients[n]->Getsd(), &fdRead))
				{
					if (RecvData(_clients[n]) == -1)
					{
						auto iter = _clients.begin() + n;
						if (iter != _clients.end())
						{
							delete _clients[n];
							_clients.erase(iter);
						}
					}
				}
			}
			return true;
		}
		return false;
	}

	int getSock() {
		return this->sd;
	}

	//�Ƿ�����
	bool isRun()
	{
		return sd != INVALID_SOCKET;
	}
	//ʹ�û���������������
	char _szRecv[RECV_BUFF_SIZE] = {};

	//�������� ����ճ��
	int RecvData(ClientSocket* pClient)
	{
		//5 ���տͻ�����������
		int nLen = (int)recv(pClient->Getsd(), _szRecv, RECV_BUFF_SIZE, 0);
		if (nLen <= 0)
		{
			printf("�ͻ���<Socket = %d>�Ѿ��˳�, ���������\n", pClient->Getsd());
			return -1;
		}
		//����ȡ�����ݿ�������Ϣ������
		memcpy(pClient->msgBuf() + pClient->getLastPos(), _szRecv, nLen);
		//����Ϣ������������β��λ�ú���
		pClient->setLastPos(pClient->getLastPos() + nLen);

		//�ж���Ϣ�����������ݳ��ȴ�����ϢͷDataHeader����
		while (pClient->getLastPos() >= sizeof(DataHeader))//ѭ����Ϊ�˽��ճ��
		{
			//��ʱ�Ϳ���֪����ǰ��Ϣ�ĳ���
			DataHeader* header = (DataHeader*)pClient->msgBuf();
			//�ж���Ϣ�����������ݳ��ȴ�����Ϣ����
			if (pClient->getLastPos() >= header->dataLength)
			{
				//ʣ��δ������Ϣ���������ݵĳ���
				int nSize = pClient->getLastPos() - header->dataLength;
				//����������Ϣ
				onNetMsg(pClient->Getsd(), header);
				//����Ϣ������ʣ��δ��������ǰ��
				memcpy(pClient->msgBuf(), pClient->msgBuf() + header->dataLength, nSize);
				//��Ϣ��������β��λ��ǰ��
				pClient->setLastPos(nSize);
			}
			else
			{
				//��Ϣ������ʣ�����ݲ���һ��������Ϣ
				break;
			}
		}
		return 0;
	}

	//��Ӧ������Ϣ
	virtual void onNetMsg(SOCKET cSock, DataHeader* header)
	{
		_recvCount++;
		auto t1 = _tTime.getElapsedSecond();
		//���ﵽһ����ʱ�����
		if (t1 >= 1.0)
		{
			printf("time<%lf>, socket<%d>, clients<%lu>, _recvCount<%d>\n", t1, sd, _clients.size(), _recvCount);
			_recvCount = 0;
			_tTime.update();
		}
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{

			//Login* login = (Login*)header;
			//printf("�յ��ͻ���<Socket = %d>����:CMD_LOGIN ���ݳ��ȣ�%d userName = %s passWord = %s\n", cSock, login->dataLength, login->userName, login->PassWord);
			//LoginResult ret;
			//send(cSock, (char*)&ret, sizeof(LoginResult), 0);
			//SendData(cSock, &ret);
		}
		break;
		case CMD_LOGOUT:
		{
			//Logout *logout = (Logout*)header;
			//printf("�յ�����:CMD_LOGINOUT ���ݳ��ȣ�%d userName = %s\n", loginout->dataLength, loginout->userName);
			//LoginOutResult ret;
			//send(cSock, (char*)&ret, sizeof(LoginOutResult), 0);
			//SendData(cSock, &ret);
		}
		break;
		default:
		{
			printf("<socket = %d>�յ�δ������Ϣ ���ݳ��ȣ�%d \n", cSock, header->dataLength);
			//DataHeader ret;
			//send(cSock, (char*)&header, sizeof(DataHeader), 0);
			//SendData(cSock, &ret);
		}
		break;
		}
	}

	//����ָ��sock����
	int SendData(SOCKET _cSock, DataHeader* header)
	{
		if (isRun() && header)
		{
			return send(_cSock, (const char*)header, header->dataLength, 0);
		}
		return SOCKET_ERROR;
	}

	//Ⱥ����Ϣ
	void SendDataToAll(DataHeader* header)
	{
		//���¿ͻ��˼���Ⱥ���������û� ������������ ��������ɱ����
		for (int n = (int)_clients.size() - 1; n >= 0; n--)
		{
			SendData(_clients[n]->Getsd(), header);
		}
	}

private:
	SOCKET sd;
	std::vector<ClientSocket*> _clients;
	//����һ����ʱ��
	CELLTimestamp _tTime;
	int _recvCount;
};
#endif