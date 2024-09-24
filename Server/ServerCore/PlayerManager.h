#pragma once

#include "stdafx.h"
#include "Lock.h"
#include "test.pb.h"
class PlayerInfo : public enable_shared_from_this<PlayerInfo>
{
public:
	Player* GetPlayer() {return &m_player;};
	Coordiante* GetCoord() { return &m_coord; };
	Rotation* GetRot() { return &m_rot; };
	MoveState* GetMoveState() { return &m_moveState; };
	PlayerInfo(int id);
private:
	Player m_player;
	Coordiante m_coord;
	Rotation m_rot;
	MoveState m_moveState;
	bool m_bAlive;
};

class Session;
class PlayerManager
{
public:
	void AddPlayerById(int);
	void RemovePlayerById(int);
	void UpdatePlayerCoordByPlayer(const Player& playerRef);
	shared_ptr<PlayerInfo> GetPlayerById(int);
	xmap<int, shared_ptr<PlayerInfo>>& GetIdToPlayerMap() { return m_idToPlayer; };

	PlayerManager();

private:
	xmap<int,shared_ptr<PlayerInfo>> m_idToPlayer;
	RWLock m_playerLock;
};
extern PlayerManager* g_pPlayerManager;