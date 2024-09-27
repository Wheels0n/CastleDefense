#pragma once
#include "stdafx.h"
#include "Lock.h"
#include "test.pb.h"
class EnemyInfo : public enable_shared_from_this<EnemyInfo>
{
public:
	Enemy* GetEnemy() { return &m_enemy; };
	Coordiante* GetCoord() { return &m_coord; };
	Rotation* GetRot() { return &m_rot; };
	MoveState* GetMoveState() { return &m_moveState; };
	EnemyInfo(int id);
private:
	Enemy m_enemy;
	Coordiante m_coord;
	Rotation m_rot;
	MoveState m_moveState;
	int m_hp;
};

class EnemyManager
{
public:
	void MakeEnemySpawnPacket(S_EnemySpawn& pkt);
	void DecreaseHp(int idx);
	//void AttackEnemyById(int);
	//void UpdateEnemyCoordByPlayer(const Player& playerRef);
	shared_ptr<EnemyInfo> GetEnemyById(int id) { return m_enemies[id]; };

	EnemyManager();
private:
	xvector<shared_ptr<EnemyInfo>> m_enemies;
	RWLock m_enemyLock;
};
extern EnemyManager* g_pEnemyManager;