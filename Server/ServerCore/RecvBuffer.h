#pragma once

/*-----------------
	RecvBuffer
-----------------*/

//1. 순환버퍼 사용하기. 7분 17초
//2. r=w가 같으면 다시 시작지점으로 그게아니면 현재 위치에서 시작지점+칸만큼 옮기는방법

class RecvBuffer
{
	enum {BUFFER_COUNT =10};
public:
	RecvBuffer(int32 bufferSize);
	~RecvBuffer();

	void Clean();
	bool OnRead(int32 numOfBytes);
	bool OnWrite(int32 numOfBytes);

	BYTE* ReadPos() { return &_buffer[_readPos]; }
	BYTE* WritePos() { return &_buffer[_writePos]; }
	int32 DataSize() { return _writePos - _readPos; }
	int32 FreeSize() {return _capacity - _writePos;}

private:

private:
	int32			_capacity = 0;
	int32			_bufferSize = 0;
	int32			_readPos = 0;
	int32			_writePos = 0;
	Vector<BYTE>	_buffer;
};

