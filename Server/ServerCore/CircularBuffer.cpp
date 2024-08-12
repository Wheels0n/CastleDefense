#include "stdafx.h"
#include "CircularBuffer.h"
#include "Allocator.h"

bool CircularBuffer::Write(string& data)
{		
	if (data.size() > CalFreeSpace())
	{
		return false;
	}

	memcpy(&m_pBuf[m_writePos], data.c_str(), data.size());
	MoveWritePos(data.size());
	return true;
}

void CircularBuffer::MoveWritePos(int len)
{	
	m_writePos +=len;
	m_writePos %= m_capacity;
	return;
}

void CircularBuffer::MoveReadPos(int len)
{
	m_readPos += len;
	m_readPos %= m_capacity;
	return;
}


CircularBuffer::CircularBuffer()
	:m_readPos(0), m_writePos(0), m_capacity(BUF_SIZE), m_curSize(0)
{
	m_pBuf.resize(BUF_SIZE);
}

CircularBuffer::~CircularBuffer()
{
}
