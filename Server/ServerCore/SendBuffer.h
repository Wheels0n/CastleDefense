#pragma once
#include "Allocator.h"

class SendBuffer : public enable_shared_from_this<SendBuffer>
{
public:
	CHAR*	GetBuffer() { return &m_buffer[0]; };
	int		GetSize() { return m_size; };

			SendBuffer(int size);
			~SendBuffer() = default;

private:
	xvector<CHAR>	m_buffer;
	int				m_size;
};

