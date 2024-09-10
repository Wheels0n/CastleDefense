#pragma once
#include "Allocator.h"

class SendBuffer : public enable_shared_from_this<SendBuffer>
{
public:
	SendBuffer(int size);
	~SendBuffer()=default;

	CHAR* GetBuffer() { return &m_buffer[0]; };
	int GetSize() { return m_size; };

private:
	xvector<CHAR> m_buffer;
	int m_size;
};

