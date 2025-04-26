#pragma once
#include "stdafx.h"
#include "Misc.h"
#include "Lock.h"
//당장은 octree이지만 미래를 대비 파티셔닝이라고 지었다.

struct Octant : enable_shared_from_this<Octant>
{
	Octant(float  center[3], int len) :IsLeaf(TRUE), m_len(len)
	{
		aabb.center[0] = center[0];
		aabb.center[1] = center[1];
		aabb.center[2] = center[2];


		aabb.bmin[0] = center[0] + len;
		aabb.bmin[1] = center[1] + len;
		aabb.bmin[2] = center[2] + len;


		aabb.bmax[0] = center[0] - len;
		aabb.bmax[1] = center[1] - len;
		aabb.bmax[2] = center[2] - len;
	};

	//현 옥탄트 크기
	AABB	aabb;
	shared_ptr<Octant> child[8];
	list<AABB*> objects;
	int		m_len;
	BOOL	IsLeaf;
};

//현재 맵의 xyz중 최대 길이. log_2(x) 번만큼의 깊이가 나옴
//각축에대해 반으로 나눔에 유의!, 큐브를 생각하면 안된다.
constexpr int _SIDE_LEN = 3500;

//이동 갱신시에 노드를 찾으면 그 노드 안에 있는 거나 걸치는 것들하고만 비교  
class Octree
{	
private:
	//현재 노드의 바로 아래 자식만 생성
	void CreateNode(shared_ptr<Octant> pRoot, float center[3], int len);

		 Octree();
		 Octree(const Octree& obj) = delete;
public :
	//AABB를 받아 딱맞아 들어가는 노드 발견시 넣고 종료. 스폰/디스폰/이동시 호출
	//포함된 Octant 포인터 반환
	//AI/플레이어 이동전에 호출하여, 새 노드에 이미 무언가 있다면 그때 충돌 검사   
	//충돌한다면 둘다 멈춤. AI는 새경로 갱신까지

	//TODO:서버는 입력만 받는 데 정확한 충돌 지점 계산이???

	shared_ptr<Octant> PlaceInNode(AABB*);
	//갱신시 현재노드로부터 제거
	void	RemoveFromNode(AABB*);

	static Octree& GetInstance()
	{
		static  Octree instance;
		return instance;
	}
private:
	shared_ptr<Octant> m_pRoot;
	RWLock				m_lock;
};