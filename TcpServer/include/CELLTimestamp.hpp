//֧��c++ 11 �ҿ�ƽ̨
/*�߾���ʱ���ʱ��*/
#pragma once
#ifndef _CELLTimestamp_hpp_
#define _CELLTimestamp_hpp_

#include <chrono>
using namespace std::chrono;

class CELLTimestamp
{
public:
	CELLTimestamp()
	{
		update();
	}
	~CELLTimestamp()
	{

	}
	void update()
	{
		_begin = high_resolution_clock::now();
	}
	/*��ȡ��ǰ��*/
	double getElapsedSecond()
	{
		return this->getElapsedTimeInMicrosec() * 0.000001;
	}
	/*��ȡ��ǰ����*/
	double getElapsedTimeInMillSec()
	{
		return this->getElapsedTimeInMicrosec() * 0.001;
	}
	/*��ȡ��ǰ΢��*/
	long long getElapsedTimeInMicrosec()
	{
		return duration_cast<microseconds>(high_resolution_clock::now() - _begin).count();
	}
protected:
	//high_resolution_clock �߾��ȼ�ʱ��
	time_point<high_resolution_clock> _begin;
};
#endif

