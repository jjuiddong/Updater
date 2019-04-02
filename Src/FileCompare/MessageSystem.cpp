
#include "stdafx.h"
#include "MessageSystem.h"


cMessageSystem::cMessageSystem()
{
}

cMessageSystem::~cMessageSystem()
{
	Clear();
}


void cMessageSystem::Push(const sMsg &msg)
{

}


bool cMessageSystem::Pop(OUT sMsg &msg)
{
	return true;
}


void cMessageSystem::Clear()
{

}
