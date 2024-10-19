#pragma once
#include "stdafx.h"
#include "Lock.h"
#include "test.pb.h"

class EnemyInfo : public enable_shared_from_this<EnemyInfo>
{
public:
	Enemy* GetEnemy() { return &m_enemy; };
	Coordiante* GetCoord() { return &m_curCoord; };
	float* GetDest() { return m_dst; };
	Rotation* GetRot() { return &m_rot; };
	MoveState* GetMoveState() { return &m_moveState; };

	unsigned int* GetPath() { return &m_pPath[m_curEdge]; };
	unsigned int GetDestPolyRef() { return m_dstPolyRef; };
	unsigned int GetCurPolyRef() { return m_curPolyRef; };
	int GetNumEdges() { return m_nEdges; };

	void SetDestPolyRef(unsigned int polyRef) { m_dstPolyRef = polyRef; };
	void SetCurPolyRef(unsigned int polyRef) { m_curPolyRef = polyRef; };
	void SetNumEdges(int n) { m_nEdges = n; };
	void ResetCurEdge() { m_curEdge = 0; }
	void MoveAlongPath() { m_curEdge++; };

	EnemyInfo(int id);
private:
	Enemy m_enemy;
	Coordiante m_curCoord;
	Rotation m_rot;
	MoveState m_moveState;
	unsigned int m_pPath[10];
	
	unsigned int m_dstPolyRef;
	unsigned int m_curPolyRef;
	float m_dst[3];

	int m_hp;
	int m_curEdge;
	int m_nEdges;
};


class EnemyManager
{
public:
	void AddEnemyToPacket(S_EnemySpawn& pkt);
	void AddEnemyToPacket(S_EnemyMove& pkt);
	void SetNextLocation();
	void SetRandomDest(int idx);
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