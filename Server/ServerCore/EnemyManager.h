#pragma once
#include "stdafx.h"
#include "test.pb.h"
#include "Lock.h"
#include "Misc.h"

#define _BRODCATE_TIME			0.2f
#define _SIMULATION_TIME		0.03f
#define _MAX_PATH_REF			256
#define _MAX_NODE				6144//2O48 * 3
class EnemyInfo : public enable_shared_from_this<EnemyInfo>
{
public:

	Enemy*								GetEnemy() { return &m_enemy; };
	Coordiante*							GetCoord() { return &m_curCoord; };
	float*								GetDest() { return m_dst; };
	Coordiante*							GetDir() { return &m_dir; };
	Rotation*							GetRot() { return &m_rot; };
	MoveState*							GetMoveState() { return &m_moveState; };
	AABB*								GetAABB() { return &m_aabb; };

	bool								CheckReachedEnd() { return m_curNode + 1 == m_nNodes; };
	float*								GetNodes() { return m_nodes; };
	float*								GetNextNode() { return &m_nodes[m_curNode*3]; };
	unsigned int*						GetPath() { return m_pPath; };
	unsigned int						GetDestPolyRef() { return m_dstPolyRef; };
	unsigned int						GetCurPolyRef() { return m_curPolyRef; };
	int									GetNumNodes() { return m_nNodes; };

	
	void								SetIsNewDest(BOOL bNewDest);
	void								SetDestPolyRef(unsigned int polyRef)	{ m_dstPolyRef = polyRef; };
	void								SetCurPolyRef(unsigned int polyRef)		{ m_curPolyRef = polyRef; };
	void								SetNumNodes(unsigned int n)				{ m_nNodes = n; };
	void								ResetNode()								{ m_curNode = 0; };
	void								MoveAlongPath()							{ m_curNode++; };
	void								UpdateCurPoly();

										EnemyInfo(int id);
private:
	AABB								m_aabb;

	Enemy								m_enemy;
	Coordiante							m_curCoord;
	Coordiante							m_dir;
	Rotation							m_rot;
	MoveState							m_moveState;
	BOOL								m_bNewDest;

	unsigned int						m_pPath[_MAX_PATH_REF];
	unsigned int						m_dstPolyRef;
	unsigned int						m_curPolyRef;

	float								m_dst[3];
	float								m_nodes[_MAX_NODE];

	int									m_curNode;
	int									m_nNodes;

};


class EnemyManager
{
private:

										EnemyManager();
										EnemyManager(const EnemyManager & obj) = delete;
public:
	void								AddEnemyToSpawnPacket(S_EnemySpawn& pkt);
	void								AddEnemyToMovePacket(S_EnemyMove& pkt);
						

	void								CalculateDir(float src[], float dst[], float dir[]);
	float								CalculateYawByPoints(float dst[], Coordiante* pCoord);
	float								CalculateYawByDir(float dir[]);

	void								DecreaseHp(int idx);

	void								BuildPath(int idx, float curCoord[3]);
	void								SetNextLocation();
	void								SetRandomDest(int idx);

	void								ResetBrodcastTime();
	shared_ptr<EnemyInfo>				GetEnemyById(int id) { return m_enemies[id]; };

	static EnemyManager& GetInstance()
	{
		static EnemyManager instance;
		return instance;
	}
private:

	xvector<shared_ptr<EnemyInfo>>		m_enemies;
	RWLock								m_enemyLock;
	float								m_brodcastTime;
	float								m_simTime;
	_STD_TIME							m_lastTime;
};