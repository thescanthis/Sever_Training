#pragma once
#include "job.h"
class Room
{
	friend class EnterJob;
	friend class LeaveJob;
	friend class BroadcastJob;
private:
	//싱글쓰레드 환경인 마냥 코딩함
	void Enter(PlayerRef player);
	void Leave(PlayerRef player);
	void Broadcast(SendBufferRef sendBuffer);

public:
	//멀티쓰레드 환경에서는 일감으로 접금
	void PushJob(JobRef job) { _jobs.Push(job); }
	void FlushJob();

private:
	map<uint64, PlayerRef> _players;

	JobQueue _jobs;
};

extern Room GRoom;

//Room Jobs

class LeaveJob : public IJob
{
public:
	LeaveJob(Room& room, PlayerRef player) : _room(room),_player(player)
	{

	}

	virtual void Execute() override
	{
		_room.Leave(_player);
	}

public:
	Room& _room;
	PlayerRef _player;
};

class EnterJob : public IJob
{
public:
	EnterJob(Room& room, PlayerRef player) : _room(room), _player(player)
	{

	}

	virtual void Execute() override
	{
		_room.Enter(_player);
	}

public:
	Room& _room;
	PlayerRef _player;
};

class BroadcastJob : public IJob
{
public:
	BroadcastJob(Room& room, SendBufferRef sendBuffer) : _room(room), _sendBuffer(sendBuffer)
	{

	}

	virtual void Execute() override
	{
		_room.Broadcast(_sendBuffer);
	}

public:
	Room& _room;
	SendBufferRef _sendBuffer;
};