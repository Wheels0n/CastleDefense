#pragma once
#include "stdafx.h"
#include "Misc.h"
#include "Lock.h"
//������ octree������ �̷��� ��� ��Ƽ�Ŵ��̶�� ������.

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

	//�� ��źƮ ũ��
	AABB	aabb;
	shared_ptr<Octant> child[8];
	list<AABB*> objects;
	int		m_len;
	BOOL	IsLeaf;
};

//���� ���� xyz�� �ִ� ����. log_2(x) ����ŭ�� ���̰� ����
//���࿡���� ������ ������ ����!, ť�긦 �����ϸ� �ȵȴ�.
constexpr int _SIDE_LEN = 3500;

//�̵� ���Žÿ� ��带 ã���� �� ��� �ȿ� �ִ� �ų� ��ġ�� �͵��ϰ� ��  
class Octree
{	
private:
	//���� ����� �ٷ� �Ʒ� �ڽĸ� ����
	void CreateNode(shared_ptr<Octant> pRoot, float center[3], int len);

		 Octree();
		 Octree(const Octree& obj) = delete;
public :
	//AABB�� �޾� ���¾� ���� ��� �߽߰� �ְ� ����. ����/����/�̵��� ȣ��
	//���Ե� Octant ������ ��ȯ
	//AI/�÷��̾� �̵����� ȣ���Ͽ�, �� ��忡 �̹� ���� �ִٸ� �׶� �浹 �˻�   
	//�浹�Ѵٸ� �Ѵ� ����. AI�� ����� ���ű���

	//TODO:������ �Է¸� �޴� �� ��Ȯ�� �浹 ���� �����???

	shared_ptr<Octant> PlaceInNode(AABB*);
	//���Ž� ������κ��� ����
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