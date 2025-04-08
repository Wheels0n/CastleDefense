#include "stdafx.h"
#include "PlayerManager.h"

PlayerManager* g_pPlayerManager = nullptr;
float g_playerSpawn_x = 1940;
float g_playerSpawn_y = 1140;
float g_playerSpawn_z = 0;

const int	_PLAYER_MAX_HP = 100;
const float _PLAYER_SIZE_X = 230;
const float _PLAYER_SIZE_Y = 90;
const float _PLAYER_SIZE_Z = 200;
const float _PLAYER_SIZE_YAW = 90;

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
	pPlayer->release_vel();
	pPlayer->release_dir();
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

	Velocity* pCurPlayerVel = pSharedCurPlayer->GetVelocity();
	const Velocity& newVel = playerRef.vel();
	pCurPlayerVel->set_x(newVel.x());
	pCurPlayerVel->set_y(newVel.y());
	pCurPlayerVel->set_z(newVel.z());

	Coordiante* pCurPlayerDir = pSharedCurPlayer->GetDir();
	const Coordiante& newDir = playerRef.dir();

	pCurPlayerDir->set_x(newDir.x());
	pCurPlayerDir->set_y(newDir.y());
	pCurPlayerDir->set_z(newDir.z());


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
	m_coord.set_x(g_playerSpawn_x);
	m_coord.set_y(g_playerSpawn_y);
	m_coord.set_z(g_playerSpawn_z);
	m_player.set_allocated_coord(&m_coord);
	m_player.set_hp(_PLAYER_MAX_HP);

	g_playerSpawn_x += _PLAYER_SIZE_X;
	g_playerSpawn_y += _PLAYER_SIZE_Y;

	m_rot.set_x(0);
	m_rot.set_y(0);
	m_rot.set_z(_PLAYER_SIZE_YAW);
	m_player.set_allocated_rot(&m_rot);

	m_vel.set_x(0);
	m_vel.set_y(0);
	m_vel.set_z(0);
	m_player.set_allocated_vel(&m_vel);

	m_dir.set_x(0);
	m_dir.set_y(0);
	m_dir.set_z(0);
	m_player.set_allocated_dir(&m_dir);

	m_player.set_movestate(m_moveState);
	m_player.set_battack(false);
}
