#include "stdafx.h"
#include "CircularBuffer.h"
#include "Allocator.h"

bool Buffer::Write(string& data)
{		
	if (data.size() > CalFreeSpace())
	{
		return false;
	}

	memcpy(&m_pBuf[m_writePos], data.c_str(), data.size());
	MoveWritePos(data.size());
	return true;
}

void Buffer::MoveWritePos(int len)
{	
	m_writePos +=len;
	assert(m_writePos <= BUF_SIZE);
	return;
}
void Buffer::MoveReadPos(int len)
{
	m_readPos += len;
	return;
}

void Buffer::ShiftBufferForward()
{

	xvector<char> temp(BUF_SIZE);
	for (int i = m_readPos, j=0; i < m_writePos; ++i,++j)
	{
		temp[j] = m_pBuf[i];
	}

	int size = CalSize();
	for (int i = 0;i<size;++i)
	{
		m_pBuf[i] = temp[i];
	}
	m_writePos -= m_readPos;
	m_readPos = 0;
}

Buffer::Buffer()
	:m_readPos(0), m_writePos(0), m_capacity(BUF_SIZE), m_curSize(0)
{
	m_pBuf.resize(BUF_SIZE);
}
Buffer::~Buffer()
{
}
