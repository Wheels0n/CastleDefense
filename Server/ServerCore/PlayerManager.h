#pragma once

#include "stdafx.h"
#include "test.pb.h"
#include "Lock.h"
#include "Misc.h"

#define _BRODCATE_TIME			0.2f
#define _SIMULATION_TIME		0.03f
class PlayerInfo : public enable_shared_from_this<PlayerInfo>
{
public:
	Player*		GetPlayer() {return &m_player;};
	Coordiante* GetCoord() { return &m_coord; };
	Coordiante* GetDir() { return &m_dir; };
	Velocity*	GetVelocity() { return &m_vel; };
	Rotation*	GetRot() { return &m_rot; };
	MoveState*	GetMoveState() { return &m_moveState; };
	AABB*		GetAABB() { return &m_aabb; };
				PlayerInfo(int id);
private:
	Player		m_player;
	Coordiante	m_coord;
	Velocity	m_vel;
	Rotation	m_rot;
	Coordiante	m_dir;
	MoveState	m_moveState;
	bool		m_bAlive;
	AABB		m_aabb;
};

class Session;
class PlayerManager
{
private:

										PlayerManager();
										PlayerManager(const PlayerManager& obj) = delete;

public:
	void								AddPlayerById(int);
	void								RemovePlayerById(int);
	shared_ptr<PlayerInfo>				GetPlayerById(int);
	xmap<int, shared_ptr<PlayerInfo>>&	GetIdToPlayerMap() { return m_idToPlayer; };

	void								SetNextLocation();
	void								UpdatePlayer(const Player& playerRef);
	void								AddPlayerToMovePacket(S_Move& pkt);


	static PlayerManager& GetInstance()
	{
		static PlayerManager instance;
		return instance;
	}

private:
	xmap<int,shared_ptr<PlayerInfo>>	m_idToPlayer;
	RWLock								m_playerLock;

	float								m_brodcastTime;
	float								m_simTime;
	_STD_TIME							m_lastTime;
};