#include "EasyTcpServer.hpp"
#include <thread>
#include "iostream"
using namespace std;

bool g_bRun = true;
//����������������
void cmdThread1() {
	while (true)
	{
		char cmdBuf[256] = {};
		scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "exit"))
		{
			g_bRun = false;
			printf("�˳�cmdThread�߳�\n");
			break;
		}
		else
		{
			printf("��֧�ֵ�����\n");
		}
	}
}

int main()
{
	/*��ӹ��� -> �����߳�ʹ�÷����������� �ͻ���һ��ͨ������exit�˳�*/
	EasyTcpServer server;
	server.Bind(nullptr, 4567);
	server.Listen(5);
	thread t(cmdThread1);
	t.detach();

	/*EasyTcpServer server1;
	server1.InitSocket();
	server1.Bind(nullptr, 4568);
	server1.Listen(5);
	thread t1(cmdThread1, &server1);
	t1.detach();*/

	/*while (server.isRun() || server1.isRun())*/
	while (g_bRun)
	{
		server.OnRun();
		//server1.OnRun();
		//printf("����ʱ�䴦������ҵ��...\n");
	}
	server.Close();
	//server1.Close();
	printf("���˳���\n");
	getchar();
	return 0;
}

