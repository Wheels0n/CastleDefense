#pragma once
#include "CoreMinimal.h"

class SendBuffer :public TSharedFromThis<SendBuffer>
{
public:
	SendBuffer(int size);
	~SendBuffer() = default;

	char* GetBuffer() { return &m_buffer[0]; };
	int GetSize() { return m_size; };

private:
	TArray<char> m_buffer;
	int m_size;
};

