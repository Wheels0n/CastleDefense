#include "SendBuffer.h"
SendBuffer::SendBuffer(int size)
	:m_size(size)
{
	m_buffer.AddZeroed(size);
}