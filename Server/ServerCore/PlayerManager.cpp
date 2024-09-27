#include "stdafx.h"
#include "PlayerManager.h"

PlayerManager* g_pPlayerManager = nullptr;
float playerSpawn_x = 0;
float playerSpawn_y = 0;

float playerSizeX = 230;
float playerSizeY = 90;
float playerSizeZ = 200;
void PlayerManager::AddPlayerById(int id)
{
	WriteLockGuard wlock(m_playerLock);
	shared_ptr<PlayerInfo> pPlayer = MakeShared<PlayerInfo>(id);
	m_idToPlayer[id] = pPlayer;
}

void PlayerManager::RemovePlayerById(int id)
{
	WriteLockGuard wlock(m_playerLock);
	shared_ptr<PlayerInfo> pSharedCurPlayer = m_idToPlayer[id]; 
	Player* pPlayer = pSharedCurPlayer->GetPlayer();
	pPlayer->release_coord();
	pPlayer->release_rot();
	m_idToPlayer.erase(id);
}

void PlayerManager::UpdatePlayerCoordByPlayer(const Player& playerRef)
{
	int id =playerRef.id();
	shared_ptr<PlayerInfo> pSharedCurPlayer = GetPlayerById(id);
	
	Coordiante* pCurPlayercoord = pSharedCurPlayer->GetCoord();
	const Coordiante& newCoord = playerRef.coord();

	pCurPlayercoord->set_x(newCoord.x());
	pCurPlayercoord->set_y(newCoord.y());
	pCurPlayercoord->set_z(newCoord.z());

	Rotation* pCurPlayerRot = pSharedCurPlayer->GetRot();
	const Rotation& newRot = playerRef.rot();
	pCurPlayerRot->set_x(newRot.x());
	pCurPlayerRot->set_y(newRot.y());
	pCurPlayerRot->set_z(newRot.z());


	Player* pCurPlayer = pSharedCurPlayer->GetPlayer();
	pCurPlayer->set_movestate(playerRef.movestate());
	pCurPlayer->set_battack(playerRef.battack());
}

shared_ptr<PlayerInfo> PlayerManager::GetPlayerById(int id)
{
	return m_idToPlayer[id];
}

PlayerManager::PlayerManager()
	:m_playerLock(4)
{
}

PlayerInfo::PlayerInfo(int id)
	:m_bAlive(true), m_moveState(IDLE)
{
	m_player.set_id(id);
	m_coord.set_x(playerSpawn_x);
	m_coord.set_y(playerSpawn_y);
	m_coord.set_z(90);
	m_player.set_allocated_coord(&m_coord);
	m_player.set_hp(100);

	playerSpawn_x += playerSizeX;
	playerSpawn_y += playerSizeY;

	m_rot.set_x(0);
	m_rot.set_y(0);
	m_rot.set_z(90);
	m_player.set_allocated_rot(&m_rot);

	m_player.set_movestate(m_moveState);
	m_player.set_battack(false);
}
