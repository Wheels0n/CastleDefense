#include "stdafx.h"
#include "EnemyManager.h"
#include "PlayerManager.h"
#include "NavigationManager.h"

EnemyManager* g_pEnemyManager = nullptr;
float enemySpawn_x = -7300;
float enemySpawn_y = 1500;
float enemySpawn_z = 100;
float enemySize_x = 120;
float enemySize_y = 45;
float enemySize_z = 175;
float detectionRadius = 1000.0f;

bool IncByDist(const pair<int, float>& a, const pair<int, float>& b)
{
	return a.second < b.second;
}

EnemyInfo::EnemyInfo(int id)
	:m_nEdges(0), m_curEdge(0)
{
	m_enemy.set_id(id);
	m_enemy.set_hp(100);


	float pos[3];
	if (g_pNavManager->FindRandomPos(&m_curCoord, pos, &m_curPolyRef))
	{
		//Set Yaw
		float dx = pos[0];
		float dy = pos[2];
		float yaw = atan2(dy, dx);
		yaw = yaw * 180.0f / 3.14f;

		m_rot.set_x(0);
		m_rot.set_y(yaw);
		m_rot.set_z(0);

		//Set Coord
		m_curCoord.set_x(-pos[0]);
		m_curCoord.set_y(-pos[2]);
		m_curCoord.set_z(pos[1]+ enemySpawn_z);

		cout << pos[0] << " " << pos[1] << " " << pos[2] << endl;
	}
	m_enemy.set_allocated_coord(&m_curCoord);
	m_enemy.set_allocated_rot(&m_rot);

	m_enemy.set_movestate(IDLE);
}

void EnemyManager::AddEnemyToPacket(S_EnemySpawn& pkt)
{
	for (int i = 0; i < sizeof(m_enemies)/sizeof(void*); ++i)
	{
		Enemy* pCurEnemy = pkt.add_enemy();
		*pCurEnemy = *(m_enemies[i]->GetEnemy());
	}
	
}
void EnemyManager::AddEnemyToPacket(S_EnemyMove& pkt)
{
	for (int i = 0; i < sizeof(m_enemies) / sizeof(void*); ++i)
	{
		Enemy* pCurEnemy = pkt.add_enemy();
		*pCurEnemy = *(m_enemies[i]->GetEnemy());
	}

}

void EnemyManager::SetNextLocation()
{
	for (int i = 0; i < sizeof(m_enemies) / sizeof(void*); ++i)
	{
		
		int nEdges = m_enemies[i]->GetNumEdges();
		float next[3] = { 0, };

		Coordiante* pEnemyCoord = m_enemies[i]->GetCoord();
			
		if (!nEdges)
		{
			m_enemies[i]->ResetCurEdge();
				
			SetRandomDest(i);
			float s[3] = { -pEnemyCoord->x(), pEnemyCoord->z() - enemySpawn_z,-pEnemyCoord->y() };

			g_pNavManager->FindPath(m_enemies[i]->GetCurPolyRef(), m_enemies[i]->GetDestPolyRef(), s, m_enemies[i]->GetDest(),
				m_enemies[i]->GetPath(), &nEdges);

		}
		
		
		m_enemies[i]->MoveAlongPath();
		m_enemies[i]->SetNumEdges(--nEdges);
		g_pNavManager->GetPosByRef(next, m_enemies[i]->GetDest(), m_enemies[i]->GetPath());
		pEnemyCoord->set_x(-next[0]);
		pEnemyCoord->set_y(-next[2]);
		pEnemyCoord->set_z(next[1] + enemySpawn_z);
	
		cout << next[0] << " " << next[1] << " " << next[2] << endl;
	}
}


void EnemyManager::SetRandomDest(int idx)
{
	
	{
		Coordiante* pCoord = m_enemies[idx]->GetCoord();
		unsigned int curPolyRef = m_enemies[idx]->GetCurPolyRef();
		unsigned int dstPolyRef = -1;
		float* dst=m_enemies[idx]->GetDest();
		if (g_pNavManager->FindRandomPosWithinRange(pCoord, dst, curPolyRef, &dstPolyRef))
		{
			//Set Yaw
			float dx = dst[0] - pCoord->x();
			float dy = dst[2] - pCoord->y();
			float yaw = atan2(dy , dx);
			yaw = yaw * 180.0f / 3.14f;

			Rotation* pRot = m_enemies[idx]->GetRot();
			pRot->set_y(yaw);

			//Set Coord
			pCoord->set_x(-dst[0]);
			pCoord->set_y(-dst[2]);
			pCoord->set_z(dst[1] + enemySpawn_z);

			m_enemies[idx]->SetDestPolyRef(dstPolyRef);

			cout << dst[0] << " " << dst[1] << " " << dst[2] << endl;
		}
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
