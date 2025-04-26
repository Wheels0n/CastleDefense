#pragma once
const int BUF_SIZE = 256;
class Buffer
{
public:
	char*	GetBufEnd()		{ return &m_pBuf[m_writePos]; };
	char*	GetBufBegin()	{ return &m_pBuf[m_readPos]; };

	bool	Write(string& data);

	void	MoveWritePos(int len);
	void	MoveReadPos(int len);

	void	Reset() 
	{ 
		if (m_writePos == m_readPos) 
		{	
			m_writePos = 0; 
			m_readPos = 0; 
		}
	};
	void	ShiftBufferForward();

	int		CalFreeSpace()	{ return m_capacity - m_writePos; };
	int		CalSize()		{ return m_writePos - m_readPos; };
	
			Buffer();
			~Buffer();
private:
	xvector<char>	m_pBuf;

	int				m_readPos;
	int				m_writePos;
	
	int				m_capacity;
	int				m_curSize;

};

