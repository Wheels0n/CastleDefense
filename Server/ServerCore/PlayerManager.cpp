#include "stdafx.h"
#include "PlayerManager.h"
#include "Partitioning.h"
#include "NavigationManager.h"
#include "PacketHandler.h"

static  float g_playerSpawn_x = 1940;
static  int g_playerSpawn_y = 1140;
static  int g_playerSpawn_z = 90.21;

static const int	_PLAYER_MAX_HP = 100;
static const float _PLAYER_SIZE_X = 230/2;
static const float _PLAYER_SIZE_Y = 90/2;
static const float _PLAYER_SIZE_Z = 200/2;
static const float _PLAYER_SIZE_YAW = 90;
static const float _PLAYER_EXTENT[3] = { _PLAYER_SIZE_X, _PLAYER_SIZE_Z, _PLAYER_SIZE_Y };
static const float _PLAYER_OFFSET[3] = {0, 0, g_playerSpawn_z};

static const float _JUMP_VEL = 420.0f;
static const float _G_ACCEL = 980.0f;
void					PlayerManager::AddPlayerById(int id)
{
	WriteLockGuard wlock(m_playerLock);
	shared_ptr<PlayerInfo> pPlayer = MakeShared<PlayerInfo>(id);
	m_idToPlayer[id] = pPlayer;
}
void					PlayerManager::RemovePlayerById(int id)
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
shared_ptr<PlayerInfo>	PlayerManager::GetPlayerById(int id)
{
	return m_idToPlayer[id];
}

void					PlayerManager::SetNextLocation()
{

	_STD_TIME cur = std::chrono::high_resolution_clock::now();
	const std::chrono::duration<double> diff = cur - m_lastTime;
	m_lastTime = cur;

	m_brodcastTime -= diff.count();
	m_simTime -= diff.count();
	if (m_simTime > 0.0f)
	{
		return;
	}
	m_simTime = _SIMULATION_TIME;

	for (auto it = m_idToPlayer.begin();it!=m_idToPlayer.end();it++)
	{
		shared_ptr<PlayerInfo> pSharedCurPlayer = it->second;
		Coordiante* pCurPlayercoord = pSharedCurPlayer->GetCoord();

		const float noOffset[3] = { 0, 0, };
		Coordiante* pCurPlayerDir = pSharedCurPlayer->GetDir();
		float dir[3] = { 0, };
		MiscHelper::ConvertUE2Nav(pCurPlayerDir, dir, noOffset);
		//0.01s(client), 0.2s(server)
		
		Velocity* pCurPlayerVel = pSharedCurPlayer->GetVelocity();
		float vel[3] = { 0, };
		vel[ENUM_TO_INT(_NAV_COMP::forward)]	= abs(pCurPlayerVel->x());
		vel[ENUM_TO_INT(_NAV_COMP::right)]		 = abs(pCurPlayerVel->y());
		// vo sinθ − gt
		Player * pCurPlayer = pSharedCurPlayer->GetPlayer();
		vel[ENUM_TO_INT(_NAV_COMP::up)] =
			pCurPlayer->movestate() == JUMP ? _JUMP_VEL
			: pCurPlayerVel->z() - _G_ACCEL * _SIMULATION_TIME;
		pCurPlayerVel->set_z(vel[ENUM_TO_INT(_NAV_COMP::up)]);

		dir[ENUM_TO_INT(_NAV_COMP::right)]		*= vel[ENUM_TO_INT(_NAV_COMP::right)]* _SIMULATION_TIME;
		dir[ENUM_TO_INT(_NAV_COMP::forward)]	*= vel[ENUM_TO_INT(_NAV_COMP::forward)]* _SIMULATION_TIME;
		dir[ENUM_TO_INT(_NAV_COMP::up)]			= vel[ENUM_TO_INT(_NAV_COMP::up)] * _SIMULATION_TIME;

		//cout << vel[1] << endl;
		//점프시 높이 값
		float result[3] = { 0, };
		{
			float dst[3] = { 0, };
			unsigned int ref = -1;
			NavigationManager::GetInstance().GetNearestPoly(pCurPlayercoord, dst, &ref);
			NavigationManager::GetInstance().GetPosByRef(result, dst, &ref);
		}

		//Collision Test Via Octree
		AABB* pAABB = pSharedCurPlayer->GetAABB();
		Octree::GetInstance().RemoveFromNode(pAABB);

		pAABB->center[ENUM_TO_INT(_NAV_COMP::right)] += dir[ENUM_TO_INT(_NAV_COMP::right)];
		pAABB->center[ENUM_TO_INT(_NAV_COMP::forward)] += dir[ENUM_TO_INT(_NAV_COMP::forward)];
		pAABB->center[ENUM_TO_INT(_NAV_COMP::up)] += dir[ENUM_TO_INT(_NAV_COMP::up)];
		pAABB->center[ENUM_TO_INT(_NAV_COMP::up)] 
			= max(result[1] + 87-0.725008, pAABB->center[ENUM_TO_INT(_NAV_COMP::up)]);
		pAABB->SetExtent(_PLAYER_EXTENT);

		Octree::GetInstance().PlaceInNode(pAABB);

		MiscHelper::ConvertNav2UE(pCurPlayercoord, pAABB->center, noOffset);
		
	}
	
	if (m_brodcastTime > 0.0f)
	{
		return;
	}
	m_brodcastTime = _BRODCATE_TIME;
	m_lastTime = std::chrono::high_resolution_clock::now();
	PacketHandler::BrodcastS_Move();
}
void					PlayerManager::UpdatePlayer(const Player& playerRef)
{
	int id =playerRef.id();
	shared_ptr<PlayerInfo> pSharedCurPlayer = GetPlayerById(id);

	Rotation* pCurPlayerRot = pSharedCurPlayer->GetRot();
	const Rotation& newRot = playerRef.rot();
	pCurPlayerRot->set_x(newRot.x());
	pCurPlayerRot->set_y(newRot.y());
	pCurPlayerRot->set_z(newRot.z());

	Velocity* pCurPlayerVel = pSharedCurPlayer->GetVelocity();
	const Velocity& newVel = playerRef.vel();
	pCurPlayerVel->set_x(newVel.x());
	pCurPlayerVel->set_y(newVel.y());
	//pCurPlayerVel->set_z(newVel.z());

	Coordiante* pCurPlayerDir = pSharedCurPlayer->GetDir();
	const Coordiante& newDir = playerRef.dir();

	pCurPlayerDir->set_x(newDir.x());
	pCurPlayerDir->set_y(newDir.y());
	pCurPlayerDir->set_z(newDir.z());


	Player* pCurPlayer = pSharedCurPlayer->GetPlayer();
	pCurPlayer->set_movestate(playerRef.movestate());
	pCurPlayer->set_battack(playerRef.battack());

	m_brodcastTime = 0;
	m_simTime = 0;
}

void PlayerManager::AddPlayerToMovePacket(S_Move& pkt)
{
	for (auto it = m_idToPlayer.begin(); it != m_idToPlayer.end(); it++)
	{
		shared_ptr<PlayerInfo> pSharedCurPlayer = it->second;

		Player* pCurPlayer = pkt.add_player();
		*pCurPlayer = *pSharedCurPlayer->GetPlayer();
	}
}

PlayerManager::PlayerManager()
	:m_playerLock(4), m_brodcastTime(_BRODCATE_TIME), m_simTime(_SIMULATION_TIME)
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

	const float noOffset[3] = { 0, 0, };
	MiscHelper::ConvertUE2Nav(&m_coord, m_aabb.center, noOffset);

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
