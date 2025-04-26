#include "stdafx.h"
#include "NavigationManager.h"
#include "Misc.h"
#include "Recastnavigation/Recast.h"
#include "Recastnavigation/RecastDebugDraw.h"
#include "Recastnavigation/DetourCommon.h"
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

random_device rd;
mt19937 gen(rd());
uniform_real_distribution<> dis(0, 1);

static const UINT MAX_POLYS = 256;
static const int MAX_SMOOTH = 2048;

float		randomFloat()
{
	return dis(gen);
}
inline bool inRange(const float* v1, const float* v2, const float r, const float h)
{
	const float dx = v2[0] - v1[0];
	const float dy = v2[1] - v1[1];
	const float dz = v2[2] - v1[2];
	return (dx * dx + dz * dz) < r * r && fabsf(dy) < h;
}
// This function checks if the path has a small U-turn, that is,
// a polygon further in the path is adjacent to the first polygon
// in the path. If that happens, a shortcut is taken.
// This can happen if the target (T) location is at tile boundary,
// and we're (S) approaching it parallel to the tile edge.
// The choice at the vertex can be arbitrary, 
//  +---+---+
//  |:::|:::|
//  +-S-+-T-+
//  |:::|   | <-- the step can end up in here, resulting U-turn path.
//  +---+---+
static int	fixupShortcuts(dtPolyRef* path, int npath, dtNavMeshQuery* navQuery)
{
	if (npath < 3)
		return npath;

	// Get connected polygons
	static const int maxNeis = 16;
	dtPolyRef neis[maxNeis];
	int nneis = 0;

	const dtMeshTile* tile = 0;
	const dtPoly* poly = 0;
	if (dtStatusFailed(navQuery->getAttachedNavMesh()->getTileAndPolyByRef(path[0], &tile, &poly)))
		return npath;

	for (unsigned int k = poly->firstLink; k != DT_NULL_LINK; k = tile->links[k].next)
	{
		const dtLink* link = &tile->links[k];
		if (link->ref != 0)
		{
			if (nneis < maxNeis)
				neis[nneis++] = link->ref;
		}
	}

	// If any of the neighbour polygons is within the next few polygons
	// in the path, short cut to that polygon directly.
	static const int maxLookAhead = 6;
	int cut = 0;
	for (int i = dtMin(maxLookAhead, npath) - 1; i > 1 && cut == 0; i--) {
		for (int j = 0; j < nneis; j++)
		{
			if (path[i] == neis[j]) {
				cut = i;
				break;
			}
		}
	}
	if (cut > 1)
	{
		int offset = cut - 1;
		npath -= offset;
		for (int i = 1; i < npath; i++)
			path[i] = path[i + offset];
	}

	return npath;
}
static bool getSteerTarget(dtNavMeshQuery* navQuery, const float* startPos, const float* endPos,
	const float minTargetDist,
	const dtPolyRef* path, const int pathSize,
	float* steerPos, unsigned char& steerPosFlag, dtPolyRef& steerPosRef,
	float* outPoints = 0, int* outPointCount = 0)
{
	// Find steer target.
	static const int MAX_STEER_POINTS = 3;
	float steerPath[MAX_STEER_POINTS * 3];
	unsigned char steerPathFlags[MAX_STEER_POINTS];
	dtPolyRef steerPathPolys[MAX_STEER_POINTS];
	int nsteerPath = 0;
	navQuery->findStraightPath(startPos, endPos, path, pathSize,
		steerPath, steerPathFlags, steerPathPolys, &nsteerPath, MAX_STEER_POINTS);
	if (!nsteerPath)
		return false;

	if (outPoints && outPointCount)
	{
		*outPointCount = nsteerPath;
		for (int i = 0; i < nsteerPath; ++i)
			dtVcopy(&outPoints[i * 3], &steerPath[i * 3]);
	}


	// Find vertex far enough to steer to.
	int ns = 0;
	while (ns < nsteerPath)
	{
		// Stop at Off-Mesh link or when point is further than slop away.
		if ((steerPathFlags[ns] & DT_STRAIGHTPATH_OFFMESH_CONNECTION) ||
			!inRange(&steerPath[ns * 3], startPos, minTargetDist, 1000.0f))
			break;
		ns++;
	}
	// Failed to find good point to steer to.
	if (ns >= nsteerPath)
		return false;

	dtVcopy(steerPos, &steerPath[ns * 3]);
	steerPos[1] = startPos[1];
	steerPosFlag = steerPathFlags[ns];
	steerPosRef = steerPathPolys[ns];

	return true;
}

dtNavMesh*	NavigationManager::LoadAll(const char* path)

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
void		NavigationManager::BuildNodes(UINT n, unsigned int* pPath, float* pDst, float* src, OUT unsigned int* nPath, OUT float* smoothPath)
{
	
	{
		// Iterate over the path to find smooth path on the detail mesh surface.
		
		dtPolyRef polys[MAX_POLYS];
		memcpy(polys, pPath, sizeof(dtPolyRef) * n);
		int npolys = n;

		float iterPos[3], targetPos[3];
		m_navQuery->closestPointOnPoly(pPath[0], src, iterPos, 0);
		m_navQuery->closestPointOnPoly(polys[npolys - 1], pDst, targetPos, 0);

		static const float STEP_SIZE = 5.0f;
		static const float SLOP = 0.01f;

		UINT nsmoothPath = 0;

		dtVcopy(&smoothPath[nsmoothPath * 3], iterPos);
		nsmoothPath++;

		// Move towards target a small advancement at a time until target reached or
		// when ran out of memory to store the path.
		while (npolys && nsmoothPath < MAX_SMOOTH)
		{
			// Find location to steer towards.
			float steerPos[3];
			unsigned char steerPosFlag;
			dtPolyRef steerPosRef;

			if (!getSteerTarget(m_navQuery, iterPos, targetPos, SLOP,
				polys, npolys, steerPos, steerPosFlag, steerPosRef))
				break;

			bool endOfPath = (steerPosFlag & DT_STRAIGHTPATH_END) ? true : false;
			bool offMeshConnection = (steerPosFlag & DT_STRAIGHTPATH_OFFMESH_CONNECTION) ? true : false;

			// Find movement delta.
			float delta[3], len;
			dtVsub(delta, steerPos, iterPos);
			len = dtMathSqrtf(dtVdot(delta, delta));
			// If the steer target is end of path or off-mesh link, do not move past the location.
			if ((endOfPath || offMeshConnection) && len < STEP_SIZE)
				len = 1;
			else
				len = STEP_SIZE / len;
			float moveTgt[3];
			dtVmad(moveTgt, iterPos, delta, len);

			// Move
			float result[3];
			dtPolyRef visited[16];
			int nvisited = 0;

			dtQueryFilter* pFilter = xnew<dtQueryFilter>();

			m_navQuery->moveAlongSurface(polys[0], iterPos, moveTgt, pFilter,
				result, visited, &nvisited, 16);

			npolys = dtMergeCorridorStartMoved(polys, npolys, MAX_POLYS, visited, nvisited);
			npolys = fixupShortcuts(polys, npolys, m_navQuery);

			float h = 0;
			m_navQuery->getPolyHeight(polys[0], result, &h);
			result[1] = h;
			dtVcopy(iterPos, result);

			// Handle end of path and off-mesh links when close enough.
			if (endOfPath && inRange(iterPos, steerPos, SLOP, 1.0f))
			{
				// Reached end of path.
				dtVcopy(iterPos, targetPos);
				if (nsmoothPath < MAX_SMOOTH)
				{
					dtVcopy(&smoothPath[nsmoothPath * 3], iterPos);
					nsmoothPath++;
				}
				break;
			}
			else if (offMeshConnection && inRange(iterPos, steerPos, SLOP, 1.0f))
			{
				// Reached off-mesh connection.
				float startPos[3], endPos[3];

				// Advance the path up to and over the off-mesh connection.
				dtPolyRef prevRef = 0, polyRef = polys[0];
				int npos = 0;
				while (npos < npolys && polyRef != steerPosRef)
				{
					prevRef = polyRef;
					polyRef = polys[npos];
					npos++;
				}
				for (int i = npos; i < npolys; ++i)
					polys[i - npos] = polys[i];
				npolys -= npos;

				// Handle the connection.
				dtStatus status = m_navMesh->getOffMeshConnectionPolyEndPoints(prevRef, polyRef, startPos, endPos);
				if (dtStatusSucceed(status))
				{
					if (nsmoothPath < MAX_SMOOTH)
					{
						dtVcopy(&smoothPath[nsmoothPath * 3], startPos);
						nsmoothPath++;
						// Hack to make the dotted path not visible during off-mesh connection.
						if (nsmoothPath & 1)
						{
							dtVcopy(&smoothPath[nsmoothPath * 3], startPos);
							nsmoothPath++;
						}
					}
					// Move position at the other side of the off-mesh link.
					dtVcopy(iterPos, endPos);
					float eh = 0.0f;
					m_navQuery->getPolyHeight(polys[0], iterPos, &eh);
					iterPos[1] = eh;
				}
			}

			// Store results.
			if (nsmoothPath < MAX_SMOOTH)
			{
				dtVcopy(&smoothPath[nsmoothPath * 3], iterPos);
				nsmoothPath++;
			}
		}
		*nPath = nsmoothPath;
	}
	
}
bool		NavigationManager::FindPath(unsigned int sRef, unsigned int eRef, float* s, float* e, OUT dtPolyRef* path, OUT int* nPath)
{
	dtQueryFilter* pFilter = xnew<dtQueryFilter>();
	dtStatus result = m_navQuery->findPath((dtPolyRef)sRef, (dtPolyRef)eRef, s, e, pFilter,
		path, nPath, MAX_POLYS);

	return result == DT_SUCCESS;
}

bool		NavigationManager::FindRandomPos(Coordiante* pCoord, float pDestPos[3], unsigned int* dstPolyRef)
{
	dtQueryFilter* pFilter = xnew<dtQueryFilter>();
	dtStatus result = m_navQuery->findRandomPoint(pFilter, &randomFloat, (dtPolyRef*)dstPolyRef, pDestPos);
	return result == DT_SUCCESS;
}
bool		NavigationManager::FindRandomPosWithinRange(Coordiante* pCoord, float pDestPos[3], unsigned int curPolyRef, unsigned int* dstPolyRef)
{
	dtQueryFilter* pFilter = xnew<dtQueryFilter>(); 
	float offset[3] = { 0.0f, 0.0f, 81.0f };
	float src[3] = {};
	MiscHelper::ConvertUE2Nav(pCoord, src, offset);
	dtStatus result = m_navQuery->findRandomPointAroundCircle((dtPolyRef)curPolyRef, src,
		1000.0f, pFilter, &randomFloat, (dtPolyRef*)dstPolyRef, pDestPos);
	return result == DT_SUCCESS;
}

bool		NavigationManager::GetNearestPoly(Coordiante* pCoord, OUT float* pos, OUT unsigned int* pRef)
{
	float extents[3] = { 1.0f, 100.0f, 1.0f };
	float offset[3] = { 0.0f, 0.0f, 81.0f };
	float center[3] = {};
	MiscHelper::ConvertUE2Nav(pCoord, center, offset );

	dtQueryFilter* pFilter = xnew<dtQueryFilter>();
	dtStatus result = m_navQuery->findNearestPoly(center, extents, pFilter,
		(dtPolyRef*)pRef, pos);

	return result == DT_SUCCESS;
}
bool		NavigationManager::GetPosByRef(OUT float* pCoord, float* pDst, unsigned int* pPath)
{
	dtStatus result = m_navQuery->closestPointOnPoly(*pPath, pDst, pCoord, 0);
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