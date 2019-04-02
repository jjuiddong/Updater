//
// 2019-04-02, jjuiddong
// Global Message System
//
#pragma once


template<class T>
class cSyncQueue
{
public:
	cSyncQueue();
	virtual ~cSyncQueue();

	void Push(const T data);
	bool Pop(OUT T data);
	void Clear();


public:
	CriticalSection m_cs;
	vector<T> m_q;
};
