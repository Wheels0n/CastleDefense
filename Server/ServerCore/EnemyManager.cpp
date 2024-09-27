#include "stdafx.h"
#include "EnemyManager.h"
EnemyManager* g_pEnemyManager = nullptr;
float enemySpawn_x = 0;
float enemySpawn_y = 0;

float enemySize_x = 120;
float enemySize_y = 45;
float enemySize_z = 175;
EnemyInfo::EnemyInfo(int id)
{
	m_enemy.set_id(id);
	m_enemy.set_hp(100);

	m_coord.set_x(enemySpawn_x + enemySize_x);
	m_coord.set_y(enemySpawn_y + enemySize_y);
	m_coord.set_z(90);
	m_enemy.set_allocated_coord(&m_coord);

	enemySpawn_x += enemySize_x;
	enemySpawn_y += enemySize_y;

	m_rot.set_x(0);
	m_rot.set_y(0);
	m_rot.set_z(90);
	m_enemy.set_allocated_rot(&m_rot);

	m_enemy.set_movestate(IDLE);
}

void EnemyManager::MakeEnemySpawnPacket(S_EnemySpawn& pkt)
{
	for (int i = 0; i < sizeof(m_enemies)/sizeof(void*); ++i)
	{
		Enemy* pCurEnemy = pkt.add_enemy();
		*pCurEnemy = *(m_enemies[i]->GetEnemy());
	}
	
}

void EnemyManager::DecreaseHp(int idx)
{
	Enemy* pEnemy = m_enemies[idx]->GetEnemy();
	int curHp = pEnemy->hp();
	pEnemy->set_hp(curHp - 50);
}

EnemyManager::EnemyManager()
	:m_enemyLock(4)
{
	m_enemies.resize(5);
	for (int i = 0; i < sizeof(m_enemies) / sizeof(void*); ++i)
	{
		shared_ptr<EnemyInfo> pEnemyInfo = MakeShared<EnemyInfo>(i);
		m_enemies[i] = pEnemyInfo;
	}
}
