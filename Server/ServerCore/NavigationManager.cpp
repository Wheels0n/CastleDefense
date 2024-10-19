#include "stdafx.h"
#include "NavigationManager.h"
#include "Recastnavigation/Recast.h"
#include "Recastnavigation/RecastDebugDraw.h"
#include "Recastnavigation/DetourDebugDraw.h"
#include "Recastnavigation/DetourNavMesh.h"
#include "Recastnavigation/DetourNavMeshQuery.h"
#include "Recastnavigation/DetourCrowd.h"
struct NavMeshSetHeader
{
	int magic;
	int version;
	int numTiles;
	dtNavMeshParams params;
};

struct NavMeshTileHeader
{
	dtTileRef tileRef;
	int dataSize;
};

NavigationManager* g_pNavManager = nullptr;
float randomFloat()
{
	return (float)(rand()) / (float)(RAND_MAX);
}

dtNavMesh* NavigationManager::LoadAll(const char* path)

{
	FILE* fp = fopen(path, "rb");
	if (!fp) return 0;

	// Read header.
	NavMeshSetHeader header;
	size_t readLen = fread(&header, sizeof(NavMeshSetHeader), 1, fp);
	if (readLen != 1)
	{
		fclose(fp);
		return 0;
	}
	if (header.magic != NAVMESHSET_MAGIC)
	{
		fclose(fp);
		return 0;
	}
	if (header.version != NAVMESHSET_VERSION)
	{
		fclose(fp);
		return 0;
	}

	dtNavMesh* mesh = dtAllocNavMesh();
	if (!mesh)
	{
		fclose(fp);
		return 0;
	}
	dtStatus status = mesh->init(&header.params);
	if (dtStatusFailed(status))
	{
		fclose(fp);
		return 0;
	}

	// Read tiles.
	for (int i = 0; i < header.numTiles; ++i)
	{
		NavMeshTileHeader tileHeader;
		readLen = fread(&tileHeader, sizeof(tileHeader), 1, fp);
		if (readLen != 1)
		{
			fclose(fp);
			return 0;
		}

		if (!tileHeader.tileRef || !tileHeader.dataSize)
			break;

		unsigned char* data = (unsigned char*)dtAlloc(tileHeader.dataSize, DT_ALLOC_PERM);
		if (!data) break;
		memset(data, 0, tileHeader.dataSize);
		readLen = fread(data, tileHeader.dataSize, 1, fp);
		if (readLen != 1)
		{
			dtFree(data);
			fclose(fp);
			return 0;
		}

		mesh->addTile(data, tileHeader.dataSize, DT_TILE_FREE_DATA, tileHeader.tileRef, 0);
	}

	fclose(fp);
	
	return mesh;
}

bool NavigationManager::FindRandomPos(Coordiante* pCoord, float pDestPos[3], unsigned int* dstPolyRef)
{
	dtQueryFilter* pFilter = xnew<dtQueryFilter>();
	dtStatus result = m_navQuery->findRandomPoint(pFilter, &randomFloat, (dtPolyRef*)dstPolyRef, pDestPos);
	return result == DT_SUCCESS;
}

bool NavigationManager::FindRandomPosWithinRange(Coordiante* pCoord, float pDestPos[3], unsigned int curPolyRef, unsigned int* dstPolyRef)
{
	dtQueryFilter* pFilter = xnew<dtQueryFilter>();
	float pSrcPos[3] = { -pCoord->x(), pCoord->z()-100.0f, -pCoord->y()};
	dtStatus result = m_navQuery->findRandomPointAroundCircle((dtPolyRef)curPolyRef, pSrcPos,
		1000.0f, pFilter, &randomFloat, (dtPolyRef*)dstPolyRef, pDestPos);
	return result == DT_SUCCESS;
}

bool NavigationManager::FindPath(unsigned int sRef, unsigned int eRef, float* s, float* e, OUT dtPolyRef* path, OUT int* nPath)
{
	dtQueryFilter* pFilter = xnew<dtQueryFilter>();
	dtStatus result = m_navQuery->findPath((dtPolyRef)sRef, (dtPolyRef)eRef, s, e, pFilter,
		path, nPath, 10);

	return result == DT_SUCCESS;
}

bool NavigationManager::GetNearestPoly(Coordiante* pCoord, OUT float* pos, OUT unsigned int* pRef)
{
	float extents[3] = { 1.0f, 200.0f, 1.0f };
	float center[3] = { -pCoord->x(), pCoord->z(), -pCoord->y() };
	dtQueryFilter* pFilter = xnew<dtQueryFilter>();
	dtStatus result = m_navQuery->findNearestPoly(center, extents, pFilter,
		(dtPolyRef*)pRef, pos);

	return result == DT_SUCCESS;
}

bool NavigationManager::GetPosByRef(float* pos, float* dst, unsigned int* pPath)
{
	dtStatus result = m_navQuery->closestPointOnPoly(*pPath, dst, pos, 0);
	return result==DT_SUCCESS;
}

NavigationManager::NavigationManager()
	:m_navMesh(nullptr), m_navQuery(nullptr)
{
	m_navMesh = LoadAll("D:\\Unreal\\CastleDefense\\Server\\ServerCore\\all_tiles_navmesh.bin");
	m_navQuery = dtAllocNavMeshQuery();
	m_navQuery->init(m_navMesh, 2048);
}

NavigationManager::~NavigationManager()
{
}