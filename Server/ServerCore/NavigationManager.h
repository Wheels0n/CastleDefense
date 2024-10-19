#pragma once
#include "test.pb.h"
static const int NAVMESHSET_MAGIC = 'M' << 24 | 'S' << 16 | 'E' << 8 | 'T'; //'MSET';
static const int NAVMESHSET_VERSION = 1;


class dtNavMesh;
class dtNavMeshQuery;
class NavigationManager
{
private:
	dtNavMesh* LoadAll(const char*);
public:
	bool FindRandomPos(Coordiante*, float[3], unsigned int*);
	bool FindRandomPosWithinRange(Coordiante*, float[3], unsigned int, unsigned int*);
	bool FindPath(unsigned int sRef, unsigned int eRef, float* s, float* e, unsigned int* path, int* nPath);
	bool GetNearestPoly(Coordiante* pCoord, float*, unsigned int*);
	bool GetPosByRef(float*, float*, unsigned int*);

	NavigationManager();
	~NavigationManager();

private:
	dtNavMesh* m_navMesh;
	dtNavMeshQuery* m_navQuery;
};

extern NavigationManager* g_pNavManager;