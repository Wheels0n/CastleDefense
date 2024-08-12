#pragma once
const int BUF_SIZE = 16;
class CircularBuffer
{
public:
	bool Write(string& data);
	void MoveWritePos(int len);
	void Read(int len);
	void MoveReadPos(int len);
	
	char* GetBuf() { return &m_pBuf[m_readPos]; };
	int CalFreeSpace() { return m_readPos>m_writePos? m_readPos-m_writePos:
		m_capacity - m_writePos + m_readPos; };

	CircularBuffer();
	~CircularBuffer();
private:
	xvector<char> m_pBuf;

	int m_readPos;
	int m_writePos;
	
	int m_capacity;
	int m_curSize;

};

