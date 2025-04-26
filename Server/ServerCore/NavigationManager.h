#pragma once
#include "test.pb.h"
static const int NAVMESHSET_MAGIC = 'M' << 24 | 'S' << 16 | 'E' << 8 | 'T'; //'MSET';
static const int NAVMESHSET_VERSION = 1;


class dtNavMesh;
class dtNavMeshQuery;
class NavigationManager
{
private:
	dtNavMesh*	LoadAll(const char*);

private :
				NavigationManager();
				NavigationManager(const NavigationManager& obj) = delete;
public:
	void		BuildNodes(UINT n, unsigned int* pPath,float* pDst, float* src, OUT unsigned int* nsmoothPath, OUT float* smoothPath);
	bool		FindPath(unsigned int sRef, unsigned int eRef, float* s, float* e, OUT unsigned int* path, OUT int* nPath);

	bool		FindRandomPos(Coordiante*, float[3], unsigned int*);
	bool		FindRandomPosWithinRange(Coordiante*, float[3], unsigned int, unsigned int*);

	bool		GetNearestPoly(Coordiante* pCoord, OUT float* pos, OUT unsigned int* pRef);
	bool		GetPosByRef(OUT float* pCoord, float* pDst, unsigned int* pPath);

	static NavigationManager& GetInstance()
	{
		static NavigationManager instance;
		return instance;
	}
				~NavigationManager();

private:
	dtNavMesh*		m_navMesh;
	dtNavMeshQuery* m_navQuery;
};

