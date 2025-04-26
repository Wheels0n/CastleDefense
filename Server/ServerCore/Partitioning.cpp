#include "stdafx.h"
#include "Allocator.h"
#include "Partitioning.h"

static const int dx[2] = {1,-1};
static const int dy[2] = {1,-1};
static const int dz[2] = {1,-1};

void Octree::CreateNode(shared_ptr<Octant> pRoot, float center[3], int len)
{
	
	if (len > 1)
	{
		pRoot->IsLeaf = FALSE;

		int childLen = len / 2;
		int i = 0;
		{
			for (int x = 0; x < 2; ++x)
			{
				for (int y = 0; y < 2; ++y)
				{
					for (int z = 0; z < 2; ++z)
					{
						float childCenter[3] = {
							center[0] + childLen * dx[x],
							center[1] + childLen * dy[y],
							center[2] + childLen * dz[z],
						};

						pRoot->child[i++] = MakeShared<Octant>(childCenter, childLen);
						
					}
				}
			}
		}
	}

	return;
}

Octree::Octree()
	:m_lock(1)
{
	float o[3] = { 0, };
	m_pRoot = MakeShared<Octant>(o, _SIDE_LEN);
	CreateNode(m_pRoot, o, _SIDE_LEN);

}

shared_ptr<Octant> Octree::PlaceInNode(AABB* pAABB)
{
	shared_ptr<Octant> result = m_pRoot;
	WriteLockGuard lockGuard(m_lock);
	for (int i = 0; i < 8; )
	{
		if (result->IsLeaf)
		{
			CreateNode(result, result->aabb.center, result->m_len);
		}
		shared_ptr<Octant> pChild = result->child[i];
		//담은 수 있는 octant나오면 결과값에 대입 및 i=0, 아니면 i++
		//가져온 obj가 음의 방향으로만 증가함에 유의
		if (pChild->aabb.bmax[0] <= pAABB->bmax[0] && pChild->aabb.bmax[1] <= pAABB->bmax[1] && pChild->aabb.bmax[2] <= pAABB->bmax[2] &&
			pChild->aabb.bmin[0] >= pAABB->bmin[0] && pChild->aabb.bmin[1] >= pAABB->bmin[1] && pChild->aabb.bmin[2] >= pAABB->bmin[2])
		{
			result = pChild;
			i = 0;
		}
		else
		{
			i++;
		}
	}

	for (auto it = result->objects.begin(); it != result->objects.end(); ++it)
	{
		AABB* pObject = *it;
		if((pObject->bmax[0] < pAABB->bmin[0] && pObject->bmax[1] < pAABB->bmin[1] && pObject->bmax[2] < pAABB->bmin[2]&&
			pObject->bmin[0] > pAABB->bmax[0] && pObject->bmin[1] > pAABB->bmax[1] && pObject->bmin[2] > pAABB->bmax[2])
			||
			(pAABB->bmax[0] < pObject->bmin[0] && pAABB->bmax[1] < pObject->bmin[1] && pAABB->bmax[2] < pObject->bmin[2] &&
				pAABB->bmin[0] > pObject->bmax[0] && pAABB->bmin[1] > pObject->bmax[1] && pAABB->bmin[2] > pObject->bmax[2]))
		{
			cout << "Collision Detected\n";
		}
		
	}
	result->objects.push_back(pAABB);

	return result;
}

void Octree::RemoveFromNode(AABB* pAABB)
{
	shared_ptr<Octant> pRoot = m_pRoot;
	shared_ptr<Octant> pPre = nullptr;

	WriteLockGuard lockGuard(m_lock);
	if (!pRoot->IsLeaf)
	{
		for (int i = 0; i < 8; )
		{
			shared_ptr<Octant> pChild = pRoot->child[i];
			//담은 수 있는 octant나오면 결과값에 대입 및 i=0, 아니면 i++
			if (pChild->aabb.bmax[0] <= pAABB->bmax[0] && pChild->aabb.bmax[1] <= pAABB->bmax[1] && pChild->aabb.bmax[2] <= pAABB->bmax[2] &&
				pChild->aabb.bmin[0] >= pAABB->bmin[0] && pChild->aabb.bmin[1] >= pAABB->bmin[1] && pChild->aabb.bmin[2] >= pAABB->bmin[2])
			{
				pPre = pRoot;
				pRoot = pChild;
				if (pChild->IsLeaf)
				{
					break;
				}
				else
				{
					i = 0;
				}
				
			}
			else
			{
				++i;
			}
		}
	}
	


	for (auto it = pRoot->objects.begin(); it != pRoot->objects.end(); ++it)
	{
		if (*it == pAABB)
		{
			pRoot->objects.erase(it);
			break;
		}
	}

	if (pRoot->objects.empty())
	{
		pRoot=nullptr;
		if (pPre)
		{
			int nChild = 0;
			for (int i = 0; i < 8; ++i)
			{
				nChild = pPre->child[i] != nullptr ? nChild + 1 : nChild;
			}
			if (nChild == 0)
			{
				pPre->IsLeaf = true;
			}
		}
	}


	return;
}
