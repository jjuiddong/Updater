//
// 2019-04-02, jjuiddong
// Global Message System
//
#pragma once


template<class T, const bool isDelete = false>
class cSyncQueue
{
public:
	cSyncQueue();
	virtual ~cSyncQueue();

	void push(const T data);
	bool front(OUT T &out);
	void pop();
	bool empty();
	uint size();
	void clear();


public:
	CriticalSection m_cs;
	vector<T> m_q;
};


template<class T, const bool isDelete = false>
inline cSyncQueue<T,isDelete>::cSyncQueue()
{
}

template<class T, const bool isDelete = false>
inline cSyncQueue<T,isDelete>::~cSyncQueue()
{
	clear();
}

template<class T, const bool isDelete = false>
inline void cSyncQueue<T,isDelete>::push(const T data)
{
	AutoCSLock cs(m_cs);
	m_q.push_back(data);
}

template<class T, const bool isDelete = false>
inline bool cSyncQueue<T,isDelete>::front(OUT T &out)
{
	if (m_q.empty())
		return false;

	AutoCSLock cs(m_cs);
	out = m_q.front();
	return true;
}

template<class T, const bool isDelete = false>
inline bool cSyncQueue<T, isDelete>::empty()
{
	AutoCSLock cs(m_cs);
	return m_q.empty();
}

template<class T, const bool isDelete = false>
inline void cSyncQueue<T,isDelete>::pop()
{
	if (m_q.empty())
		return;
	AutoCSLock cs(m_cs);
	if (isDelete)
		delete m_q.front();
	common::rotatepopvector(m_q, 0);
}

template<class T, const bool isDelete = false>
inline uint cSyncQueue<T,isDelete>::size()
{
	AutoCSLock cs(m_cs);
	return m_q.size();
}

template<class T, const bool isDelete = false>
inline void cSyncQueue<T,isDelete>::clear()
{
	AutoCSLock cs(m_cs);
	if (isDelete)
	{
		for (auto &p : m_q)
			delete p;
		m_q.clear();
	}
	else
	{
		m_q.clear();
	}
}
