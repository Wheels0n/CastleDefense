#include "stdafx.h"
#include "PlayerManager.h"

PlayerManager* g_pPlayerManager = nullptr;
float g_x=0.0f;
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

	MoveState* pCurMoveState = pSharedCurPlayer->GetMoveState();
	const MoveState& newMoveState = playerRef.movestate();
	*pCurMoveState = newMoveState;
	
	Player* pCurPlayer = pSharedCurPlayer->GetPlayer();
	pCurPlayer->set_movestate(newMoveState);

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
	m_coord.set_x(g_x);
	m_coord.set_y(0);
	m_coord.set_z(90);
	m_player.set_allocated_coord(&m_coord);

	g_x+=250.0f;

	m_rot.set_x(0);
	m_rot.set_y(0);
	m_rot.set_z(90);
	m_player.set_allocated_rot(&m_rot);

	m_player.set_movestate(m_moveState);
}
