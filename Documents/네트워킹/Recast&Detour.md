## 서버에서의 적 경로 계산

가만 생각해보니 네비게이션 메시가 있었다. 경로 찾기야 어떻게 되었든 에디터 현재 레벨의 네비게이션 메시가 필요했다. 이를 서버에서 사용 하도록 해야한다. 현재 언리얼 엔진 공식 커뮤니티에서 보이는 글들이 죄다 ExportNavigationData라는 함수를 썼는 데 정작 맵 obj파일 자체가 파싱된 된다는 이야기 였다.

```c++
FWorldContext Context = GEngine->GetWorldContexts().Last();
    UWorld *World = Context.World();

    UNavigationSystem* NavSys = World->GetNavigationSystem();
    NavSys->GetMainNavData(FNavigationSystem::ECreateIfEmpty::Create)->GetGenerator()->ExportNavigationData(FString(TEXT("Test.obj")));
```

결국 NavigationSystem으로부터 메시를 직접 파싱 해야한다고 한다. 네비게이션시스템과 그 관련 클래스에 관한 내용은 다른 문서파일으로 작성했다. 분석하고 나니 어차피 엔진에서도 Recast&Detour라는 라이브러리를 쓰는 데 나도 이걸 서버에 가져와서 쓰려고한다. 네비게이셔 메시 파싱

### RecastNavigation 추출

이것도 CMAKE를 써야한다. 어차피 해당 repo에 설치 방법이 나와 있지만 유의사항으로

1. SDL2는 공식사이트 - Release- ~VC로 끝나는 zip을 받자.
2. CMAKE에서 대상 경로를 RecastDemo말고 그 상위 폴더로 하여 생성하면 솔루션이 잘 생성된다.
3. 데모 빌드전에, DebugUtils, Detour, DetourCrowd, DetourTileCache, Recast가 먼저 lib으로 빌드 되어 있어야 한다.

![RecastNavigation 데모](RecastDetour데모.JPG)
물론 이 데모는 잘돌아가는 지 확인 용이고 실제로는 앞서 만든 lib을 가져다가 서버 프로젝트에 추가해서 길찾기와 네비게이션 데이터 읽기 용으로 쓸 것이다.

데모를 쓰면 navMesh를 .bin 포맷으로 저장한다.

### RecastNavigation 연동

가만 생각해보니 프로젝트 세팅에서 Agent나 NavMesh관련 설정을 건드린 적이 없다. 그냥 기본 값으로 해서
레벨.obj를 파싱->데모로 navMesh추출->서버에서 사용  
이렇게 해야겠다.

게임모드에서 현재 맵을 파싱하도록 하였다. 유의사항으로 Build.cs에 'Navmesh'를 추가 안하면 Recast헤더를  
못 찾아서 컴파일이 안된다.

```c++
#include "Runtime/NavigationSystem/Public/NavigationSystem.h"
#include "Runtime/NavigationSystem/Public/NavMesh/RecastNavMesh.h"
#include "Runtime/NavigationSystem/Public/NavMesh/RecastNavMeshGenerator.h"
void ACastleDefenceGameMode::BeginPlay()
{
    Super::BeginPlay();
    auto NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    ANavigationData* pNavData =
        NavSys->GetDefaultNavDataInstance(FNavigationSystem::ECreateIfEmpty::DontCreate);

    ARecastNavMesh* pNavMesh = Cast<ARecastNavMesh>(pNavData);
    pNavMesh->GetGenerator()->ExportNavigationData(FString(TEXT("D:\\NavMesh.obj")));
}
```

~~ navMesh 생성이 static이면 제너레이터가 없어서 크래시, dynamic으로 해야한다. 근데 lib관련
정보만 기입되고 정점/색인 정보는 누락되었다. 추출 함수를 보면 static인것만 받는 다. ~~

```c++
        const TArray<FVector>* LevelGeom = Level->GetStaticNavigableGeometry();
        if (LevelGeom != NULL && LevelGeom->Num() > 0)
```

혹시나 싶어서 3인칭 기본 프로젝트를 만들어서 dynamic으로 추출했다. 결과는 성공이었다. 네비게이션 볼륨안의 들어간  
메시 부분 또는 전체가 하나의 obj파일로 나온다.
![정적 메시로 이로어진 맵](네비게이션%20메시%20추출%20성공.JPG)

참고로 mesh의 getNavMeshBoundsMin/max() 해도 네비 메시 말고 대상 맵 매시의 AABB 반환하니 주의. 진짜 네비게이션 메쉬의 AABB를 구하려면 타일의 폴리곤내의 정점을 반복문으로 돌려봐야한다. 자세한건 duDebugDrawNavMeshWithClosedList함수를 따라가면 된다.
아래의 코드는 데모의 디버깅용 네비게이션 메시의 색깔 타일 관련 코드이다.

```c++
dd->begin(DU_DRAW_TRIS);
for (int i = 0; i < tile->header->polyCount; ++i)
{
    const dtPoly* p = &tile->polys[i];
    if (p->getType() == DT_POLYTYPE_OFFMESH_CONNECTION)	// Skip off-mesh links.
        continue;

    const dtPolyDetail* pd = &tile->detailMeshes[i];

    unsigned int col;
    if (query && query->isInClosedList(base | (dtPolyRef)i))
        col = duRGBA(255,196,0,64);
    else
    {
        if (flags & DU_DRAWNAVMESH_COLOR_TILES)
            col = tileColor;
        else
            col = duTransCol(dd->areaToCol(p->getArea()), 64);
    }

    for (int j = 0; j < pd->triCount; ++j)
    {
        const unsigned char* t = &tile->detailTris[(pd->triBase+j)*4];
        for (int k = 0; k < 3; ++k)
        {
            if (t[k] < p->vertCount)
                dd->vertex(&tile->verts[p->verts[t[k]]*3], col);
            else
                dd->vertex(&tile->detailVerts[(pd->vertBase+t[k]-p->vertCount)*3], col);
        }
    }
}
dd->end();
```

결국 랜드스케이프 환경이 문제라는 것이다. 그래서 Static 메시로 된 애셋으로 대체하여 메시
추출에 성공했다. 근데 문제는 처리할 정보가 너무 많아서인지 정보가 추출이 안된다.

![메시 추출 성공](다른%20레벨에서%20네비게이션%20메시%20추출.JPG)  
![메시 추출 성공_2](다른%20레벨에서%20네비게이션%20메시%20추출_2.JPG)

코드를 보면 ImGui로 UI를 처리했는데 최댓값만 바꾸었다.

```c++
    imguiSeparator();
    imguiLabel("Detail Mesh");
    imguiSlider("Sample Distance", &m_detailSampleDist, 0.0f, 1600.0f, 1.0f);
    imguiSlider("Max Sample Error", &m_detailSampleMaxError, 0.0f, 1600.0f, 1.0f);

    imguiSeparator();
```

어차피 서버에서 이동 판정 할거라서 매개변수를 바꿔가면서 navMesh를 만들겠다. 결과는 에디터내에서  
 본것과 얼추 비슷하다. 매개변수를 에디터와 완전 같게 하니 눈에 띄게 이상해져서 살짝 바꾸었다.

![메시 추출 성공_3](다른%20레벨에서%20네비게이션%20메시%20추출_3.JPG)  
![에디터 내 네비게이션 메시](에디터%20내%20네비게이션%20메시.JPG)

### RecastNavigation 추출

해당 데모에서 네비게이션 메쉬를 저장/불러오기가 가능한데 bin으로 관리한다. fopen/write/read로 "\*b"옵션을 주는 데  
이진파일 플래그이다. hex 에디터 같은 걸로 열어야 된다. 자기만의 방식으로 직렬화를 하는 모양이다. 이 코드들을 응용해서  
앞서 만든 네비 메시를 불러와 도달 여부만 파악해서 이동하라고 패킷으로 뿌릴 것이다.

```c++
    if (imguiButton("Save"))
    {
        Sample::saveAll("all_tiles_navmesh.bin", m_navMesh);
    }

    if (imguiButton("Load"))
    {
        dtFreeNavMesh(m_navMesh);
        m_navMesh = Sample::loadAll("all_tiles_navmesh.bin");
        m_navQuery->init(m_navMesh, 2048);
    }

dtNavMesh* Sample::loadAll(const char* path)
{
    FILE* fp = fopen(path, "rb");
    if (!fp) return 0;
//...
```

lib와 헤더들을 모두 다 서버에 연동해야한다. 둘 다 서버코어 폴더로 복붙했다. 그리고 네비게이션 매니저를 만들었다. 매니저는 메시 정보를
담은 dtNavMesh와 이를 기반으로 길찾기를 처리해주는 dtNavMeshQuery를 초기화 한다. 모든 길찾기 관련 로직은 이곳을 통할 것이다.

```c++
class dtNavMesh;
class dtNavMeshQuery;
class NavigationManager
{
    private:
        dtNavMesh* LoadAll(const char*);
    public:
        NavigationManager();
        ~NavigationManager();

    private:
        dtNavMesh* m_navMesh;
        dtNavMeshQuery* m_navQuery;
};

NavigationManager::NavigationManager()
    :m_navMesh(nullptr), m_navQuery(nullptr)
{
    m_navMesh = LoadAll("all_tiles_navmesh.bin");
    m_navQuery->init(m_navMesh, 2048);
}
```

데모의 함수를 그대로 가져왔다. 분석을 하자면, 이진 파일을 이진 읽기 모드로 연다. 메시셋의  
헤더가 올바른 지 체크하고 dtNavMesh를 할당한다. 그리고 헤더의 네비메쉬 패러미터로 초기화  
한다. 원점, 타일 크기, 최대 타일수, 타일 내 최대 폴리곤 수를 담고 있다.

```c++
static const int NAVMESHSET_MAGIC = 'M' << 24 | 'S' << 16 | 'E' << 8 | 'T'; //'MSET';
static const int NAVMESHSET_VERSION = 1;


struct NavMeshSetHeader
{
    int magic;
    int version;
    int numTiles;
    dtNavMeshParams params;
};

struct dtNavMeshParams
{
    float orig[3];					///< The world space origin of the navigation mesh's tile space. [(x, y, z)]
    float tileWidth;				///< The width of each tile. (Along the x-axis.)
    float tileHeight;				///< The height of each tile. (Along the z-axis.)
    int maxTiles;					///< The maximum number of tiles the navigation mesh can contain. This and maxPolys are used to calculate how many bits are needed to identify tiles and polygons uniquely.
    int maxPolys;					///< The maximum number of polygons each tile can contain. This and maxTiles are used to calculate how many bits are needed to identify tiles and polygons uniquely.
};

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

```

이어서 담겨진 타일 수 만큼 반복하며 타일 번호(tileRef)와 타일 데이터 크기가 올바르다면  
그 만큼 메모리를 할당하여 그 주소로 이진 파일을 fread한다. 그리고 메시에 타일을 추가한다.

```c++
    //dtNavMesh* NavigationManager::LoadAll(const char* path)
    struct NavMeshTileHeader
    {
        dtTileRef tileRef;
        int dataSize;
    };

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
```

dtNavMeshQuery도 초기화를 하는데 사용할 메시와 탐색할 최대 노드수를 받는다. 결과값은
unsigned int로 성공 여부와 실패 원인을 담는다.

```c++
/// Initializes the query object.
///  @param[in]		nav			Pointer to the dtNavMesh object to use for all queries.
///  @param[in]		maxNodes	Maximum number of search nodes. [Limits: 0 < value <= 65535]
/// @returns The status flags for the query.
dtStatus init(const dtNavMesh* nav, const int maxNodes);
```

이 매니저를 통해서만 lib 관련 코드들을 처리할 것이다.

### findRandomPoint()

이 라이브러리에서 사용 할 기능은 세 개이다.

- 도달 가능한 임의의 지점 찾기
- 임의의 두 지점간 최단 경로 찾기
- 플레이어가 속한 타일 찾기

우선 임의의 지점을 찾아서 그곳으로 이동하는 것까지만 구현 해보겠다.

````c++
/// Returns random location on navmesh.
/// Polygons are chosen weighted by area. The search runs in linear related to number of polygon.
///  @param[in]		filter			The polygon filter to apply to the query.
///  @param[in]		frand			Function returning a random number [0..1).
///  @param[out]	randomRef		The reference id of the random location.
///  @param[out]	randomPt		The random location.
/// @returns The status flags for the query.
dtStatus findRandomPoint(const dtQueryFilter* filter, float (*frand)(),
                         dtPolyRef* randomRef, float* randomPt) const;

여기서 randomRef는 단순히 양의 정수이다.

컴퓨터에서 난수는 진정한 난수가 아니라서 같은 시드값을 넣으면 같은 일련의 값이 나온다. random_device()는 플랫폼 전용방식으로 난수를
추출하는 클래스이다.(왜 랜덤 디바이스 자체를 그냥 난수로 사용하면 안 되는지는 의문)이를 시드 값으로 하여 mt199937로 난수를 생성하면   2^199937 주기로 난수가 반복된다. 이 난수 생성기를 균등 분포에 넣어 사용한다.
```c++
random_device rd;
mt19937 gen(rd());
uniform_real_distribution<> dis(0, 1);

static const UINT MAX_POLYS = 256;
static const int MAX_SMOOTH = 2048;

float		randomFloat()
{
    return dis(gen);
}
````

쿼리 필터라는 클래스를 받는 데, 방문 가능한 폴리곤을 뜻하는 플래그 값들과 영역 타입별  
방문 비용을 정의 하는 배열이 있다. 이 클래스의 멤버함수들은 setter/getter뿐이다. 생성시에는  
모두 방문 가능하며 영역은 종류 상관없이 동일하다. 데모에서는 서로다르게 되어있다.

```c++
/// Defines polygon filtering and traversal costs for navigation mesh query operations.
/// @ingroup detour
class dtQueryFilter
{
    float m_areaCost[DT_MAX_AREAS];		///< Cost per area type. (Used by default implementation.)
    unsigned short m_includeFlags;		///< Flags for polygons that can be visited. (Used by default implementation.)
    unsigned short m_excludeFlags;		///< Flags for polygons that should not be visited. (Used by default implementation.)
//...

dtQueryFilter::dtQueryFilter() :
    m_includeFlags(0xffff),
    m_excludeFlags(0)
{
    for (int i = 0; i < DT_MAX_AREAS; ++i)
        m_areaCost[i] = 1.0f;
}

void NavMeshTesterTool::init(Sample* sample)
{
    m_sample = sample;
    m_navMesh = sample->getNavMesh();
    m_navQuery = sample->getNavMeshQuery();
    recalc();

    if (m_navQuery)
    {
        // Change costs.
        m_filter.setAreaCost(SAMPLE_POLYAREA_GROUND, 1.0f);
        m_filter.setAreaCost(SAMPLE_POLYAREA_WATER, 10.0f);
        m_filter.setAreaCost(SAMPLE_POLYAREA_ROAD, 1.0f);
        m_filter.setAreaCost(SAMPLE_POLYAREA_DOOR, 1.0f);
        m_filter.setAreaCost(SAMPLE_POLYAREA_GRASS, 2.0f);
        m_filter.setAreaCost(SAMPLE_POLYAREA_JUMP, 1.5f);
    }
//...
```

## 서버에서의 적 경로 계산

가만 생각해보니 네비게이션 메시가 있었다. 경로 찾기야 어떻게 되었든 에디터 현재 레벨의 네비게이션 메시가 필요했다. 이를 서버에서 사용 하도록 해야한다.  
현재 언리얼 엔진 공식 커뮤니티에서 보이는 글들이 죄다 ExportNavigationData라는 함수를 썼는 데 정작 맵 obj파일 자체가 파싱된 된다는 이야기 였다.

```c++
FWorldContext Context = GEngine->GetWorldContexts().Last();
    UWorld *World = Context.World();

    UNavigationSystem* NavSys = World->GetNavigationSystem();
    NavSys->GetMainNavData(FNavigationSystem::ECreateIfEmpty::Create)->GetGenerator()->ExportNavigationData(FString(TEXT("Test.obj")));
```

결국 NavigationSystem으로부터 메시를 직접 파싱 해야한다고 한다. 네비게이션시스템과 그 관련 클래스에 관한 내용은 다른 문서파일으로 작성했다. 분석하고 나니 어차피 엔진에서도 Recast&Detour라는 라이브러리를 쓰는 데 나도 이걸 서버에 가져와서 쓰려고한다. 네비게이셔 메시 파싱

### RecastNavigation 추출

이것도 CMAKE를 써야한다. 어차피 해당 repo에 설치 방법이 나와 있지만 유의사항으로

1. SDL2는 공식사이트 - Release- ~VC로 끝나는 zip을 받자.
2. CMAKE에서 대상 경로를 RecastDemo말고 그 상위 폴더로 하여 생성하면 솔루션이 잘 생성된다.
3. 데모 빌드전에, DebugUtils, Detour, DetourCrowd, DetourTileCache, Recast가 먼저 lib으로 빌드 되어 있어야 한다.

![RecastNavigation 데모](RecastDetour데모.JPG)
물론 이 데모는 잘돌아가는 지 확인 용이고 실제로는 앞서 만든 lib을 가져다가 서버 프로젝트에 추가해서 길찾기와 네비게이션 데이터 읽기 용으로 쓸 것이다.

데모를 쓰면 navMesh를 .bin 포맷으로 저장한다.

### RecastNavigation 연동

가만 생각해보니 프로젝트 세팅에서 Agent나 NavMesh관련 설정을 건드린 적이 없다. 그냥 기본 값으로 해서
레벨.obj를 파싱->데모로 navMesh추출->서버에서 사용  
이렇게 해야겠다.

게임모드에서 현재 맵을 파싱하도록 하였다. 유의사항으로 Build.cs에 'Navmesh'를 추가 안하면 Recast헤더를  
못 찾아서 컴파일이 안된다.

```c++
#include "Runtime/NavigationSystem/Public/NavigationSystem.h"
#include "Runtime/NavigationSystem/Public/NavMesh/RecastNavMesh.h"
#include "Runtime/NavigationSystem/Public/NavMesh/RecastNavMeshGenerator.h"
void ACastleDefenceGameMode::BeginPlay()
{
    Super::BeginPlay();
    auto NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    ANavigationData* pNavData =
        NavSys->GetDefaultNavDataInstance(FNavigationSystem::ECreateIfEmpty::DontCreate);

    ARecastNavMesh* pNavMesh = Cast<ARecastNavMesh>(pNavData);
    pNavMesh->GetGenerator()->ExportNavigationData(FString(TEXT("D:\\NavMesh.obj")));
}
```

~~ navMesh 생성이 static이면 제너레이터가 없어서 크래시, dynamic으로 해야한다. 근데 lib관련
정보만 기입되고 정점/색인 정보는 누락되었다. 추출 함수를 보면 static인것만 받는 다. ~~

```c++
        const TArray<FVector>* LevelGeom = Level->GetStaticNavigableGeometry();
        if (LevelGeom != NULL && LevelGeom->Num() > 0)
```

혹시나 싶어서 3인칭 기본 프로젝트를 만들어서 dynamic으로 추출했다. 결과는 성공이었다. 네비게이션 볼륨안의 들어간  
메시 부분 또는 전체가 하나의 obj파일로 나온다.
![정적 메시로 이로어진 맵](네비게이션%20메시%20추출%20성공.JPG)

참고로 mesh의 getNavMeshBoundsMin/max() 해도 네비 메시 말고 대상 맵 매시의 AABB 반환하니 주의. 진짜 네비게이션 메쉬의 AABB를 구하려면 타일의 폴리곤내의 정점을 반복문으로 돌려봐야한다. 자세한건 duDebugDrawNavMeshWithClosedList함수를 따라가면 된다.
아래의 코드는 데모의 디버깅용 네비게이션 메시의 색깔 타일 관련 코드이다.

```c++
dd->begin(DU_DRAW_TRIS);
for (int i = 0; i < tile->header->polyCount; ++i)
{
    const dtPoly* p = &tile->polys[i];
    if (p->getType() == DT_POLYTYPE_OFFMESH_CONNECTION)	// Skip off-mesh links.
        continue;

    const dtPolyDetail* pd = &tile->detailMeshes[i];

    unsigned int col;
    if (query && query->isInClosedList(base | (dtPolyRef)i))
        col = duRGBA(255,196,0,64);
    else
    {
        if (flags & DU_DRAWNAVMESH_COLOR_TILES)
            col = tileColor;
        else
            col = duTransCol(dd->areaToCol(p->getArea()), 64);
    }

    for (int j = 0; j < pd->triCount; ++j)
    {
        const unsigned char* t = &tile->detailTris[(pd->triBase+j)*4];
        for (int k = 0; k < 3; ++k)
        {
            if (t[k] < p->vertCount)
                dd->vertex(&tile->verts[p->verts[t[k]]*3], col);
            else
                dd->vertex(&tile->detailVerts[(pd->vertBase+t[k]-p->vertCount)*3], col);
        }
    }
}
dd->end();
```

결국 랜드스케이프 환경이 문제라는 것이다. 그래서 Static 메시로 된 애셋으로 대체하여 메시
추출에 성공했다. 근데 문제는 처리할 정보가 너무 많아서인지 정보가 추출이 안된다.

![메시 추출 성공](다른%20레벨에서%20네비게이션%20메시%20추출.JPG)  
![메시 추출 성공_2](다른%20레벨에서%20네비게이션%20메시%20추출_2.JPG)

코드를 보면 ImGui로 UI를 처리했는데 최댓값만 바꾸었다.

```c++
    imguiSeparator();
    imguiLabel("Detail Mesh");
    imguiSlider("Sample Distance", &m_detailSampleDist, 0.0f, 1600.0f, 1.0f);
    imguiSlider("Max Sample Error", &m_detailSampleMaxError, 0.0f, 1600.0f, 1.0f);

    imguiSeparator();
```

어차피 서버에서 이동 판정 할거라서 매개변수를 바꿔가면서 navMesh를 만들겠다. 결과는 에디터내에서  
 본것과 얼추 비슷하다. 매개변수를 에디터와 완전 같게 하니 눈에 띄게 이상해져서 살짝 바꾸었다.

![메시 추출 성공_3](다른%20레벨에서%20네비게이션%20메시%20추출_3.JPG)  
![에디터 내 네비게이션 메시](에디터%20내%20네비게이션%20메시.JPG)

### RecastNavigation 추출

해당 데모에서 네비게이션 메쉬를 저장/불러오기가 가능한데 bin으로 관리한다. fopen/write/read로 "\*b"옵션을 주는 데  
이진파일 플래그이다. hex 에디터 같은 걸로 열어야 된다. 자기만의 방식으로 직렬화를 하는 모양이다. 이 코드들을 응용해서  
앞서 만든 네비 메시를 불러와 도달 여부만 파악해서 이동하라고 패킷으로 뿌릴 것이다.

```c++
    if (imguiButton("Save"))
    {
        Sample::saveAll("all_tiles_navmesh.bin", m_navMesh);
    }

    if (imguiButton("Load"))
    {
        dtFreeNavMesh(m_navMesh);
        m_navMesh = Sample::loadAll("all_tiles_navmesh.bin");
        m_navQuery->init(m_navMesh, 2048);
    }

dtNavMesh* Sample::loadAll(const char* path)
{
    FILE* fp = fopen(path, "rb");
    if (!fp) return 0;
//...
```

lib와 헤더들을 모두 다 서버에 연동해야한다. 둘 다 서버코어 폴더로 복붙했다. 그리고 네비게이션 매니저를 만들었다. 매니저는 메시 정보를
담은 dtNavMesh와 이를 기반으로 길찾기를 처리해주는 dtNavMeshQuery를 초기화 한다. 모든 길찾기 관련 로직은 이곳을 통할 것이다.

```c++
class dtNavMesh;
class dtNavMeshQuery;
class NavigationManager
{
    private:
        dtNavMesh* LoadAll(const char*);
    public:
        NavigationManager();
        ~NavigationManager();

    private:
        dtNavMesh* m_navMesh;
        dtNavMeshQuery* m_navQuery;
};

NavigationManager::NavigationManager()
    :m_navMesh(nullptr), m_navQuery(nullptr)
{
    m_navMesh = LoadAll("all_tiles_navmesh.bin");
    m_navQuery->init(m_navMesh, 2048);
}
```

데모의 함수를 그대로 가져왔다. 분석을 하자면, 이진 파일을 이진 읽기 모드로 연다. 메시셋의  
헤더가 올바른 지 체크하고 dtNavMesh를 할당한다. 그리고 헤더의 네비메쉬 패러미터로 초기화  
한다. 원점, 타일 크기, 최대 타일수, 타일 내 최대 폴리곤 수를 담고 있다.

```c++
static const int NAVMESHSET_MAGIC = 'M' << 24 | 'S' << 16 | 'E' << 8 | 'T'; //'MSET';
static const int NAVMESHSET_VERSION = 1;


struct NavMeshSetHeader
{
    int magic;
    int version;
    int numTiles;
    dtNavMeshParams params;
};

struct dtNavMeshParams
{
    float orig[3];					///< The world space origin of the navigation mesh's tile space. [(x, y, z)]
    float tileWidth;				///< The width of each tile. (Along the x-axis.)
    float tileHeight;				///< The height of each tile. (Along the z-axis.)
    int maxTiles;					///< The maximum number of tiles the navigation mesh can contain. This and maxPolys are used to calculate how many bits are needed to identify tiles and polygons uniquely.
    int maxPolys;					///< The maximum number of polygons each tile can contain. This and maxTiles are used to calculate how many bits are needed to identify tiles and polygons uniquely.
};

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

```

이어서 담겨진 타일 수 만큼 반복하며 타일 번호(tileRef)와 타일 데이터 크기가 올바르다면  
그 만큼 메모리를 할당하여 그 주소로 이진 파일을 fread한다. 그리고 메시에 타일을 추가한다.

```c++
    //dtNavMesh* NavigationManager::LoadAll(const char* path)
    struct NavMeshTileHeader
    {
        dtTileRef tileRef;
        int dataSize;
    };

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
```

dtNavMeshQuery도 초기화를 하는데 사용할 메시와 탐색할 최대 노드수를 받는다. 결과값은
unsigned int로 성공 여부와 실패 원인을 담는다.

```c++
/// Initializes the query object.
///  @param[in]		nav			Pointer to the dtNavMesh object to use for all queries.
///  @param[in]		maxNodes	Maximum number of search nodes. [Limits: 0 < value <= 65535]
/// @returns The status flags for the query.
dtStatus init(const dtNavMesh* nav, const int maxNodes);
```

이 매니저를 통해서만 lib 관련 코드들을 처리할 것이다.

### findRandomPoint()

이 라이브러리에서 사용 할 기능은 세 개이다.

- 도달 가능한 임의의 지점 찾기
- 임의의 두 지점간 최단 경로 찾기
- 플레이어가 속한 타일 찾기

우선 임의의 지점을 찾아서 그곳으로 이동하는 것까지만 구현 해보겠다.

````c++
/// Returns random location on navmesh.
/// Polygons are chosen weighted by area. The search runs in linear related to number of polygon.
///  @param[in]		filter			The polygon filter to apply to the query.
///  @param[in]		frand			Function returning a random number [0..1).
///  @param[out]	randomRef		The reference id of the random location.
///  @param[out]	randomPt		The random location.
/// @returns The status flags for the query.
dtStatus findRandomPoint(const dtQueryFilter* filter, float (*frand)(),
                         dtPolyRef* randomRef, float* randomPt) const;

여기서 randomRef는 단순히 양의 정수이다. 그리고 랜덤함수는 ~~rand()를 부동소수점 최댓값으로 나눈 값을 반환하는 걸로 임시처리했다.~~
컴퓨터에서 난수는 진정한 난수가 아니라서 같은 시드값을 넣으면 같은 일련의 값이 나온다. random_device()는 플랫폼 전용방식으로 난수를
추출하는 클래스이다.(왜 랜덤 디바이스 자체를 그냥 난수로 사용하면 안 되는지는 의문)이를 시드 값으로 하여 mt199937로 난수를 생성하면   2^199937 주기로 난수가 반복된다. 이 난수 생성기를 균등 분포에 넣어 사용한다.
```c++
random_device rd;
mt19937 gen(rd());
uniform_real_distribution<> dis(0, 1);

static const UINT MAX_POLYS = 256;
static const int MAX_SMOOTH = 2048;

float		randomFloat()
{
    return dis(gen);
}
````

쿼리 필터라는 클래스를 받는 데, 방문 가능한 폴리곤을 뜻하는 플래그 값들과 영역 타입별
방문 비용을 정의 하는 배열이 있다. 이 클래스의 멤버함수들은 setter/getter뿐이다. 생성시에는
모두 방문 가능하며 영역은 종류 상관없이 동일하다. 데모에서는 서로다르게 되어있다.

```c++
/// Defines polygon filtering and traversal costs for navigation mesh query operations.
/// @ingroup detour
class dtQueryFilter
{
    float m_areaCost[DT_MAX_AREAS];		///< Cost per area type. (Used by default implementation.)
    unsigned short m_includeFlags;		///< Flags for polygons that can be visited. (Used by default implementation.)
    unsigned short m_excludeFlags;		///< Flags for polygons that should not be visited. (Used by default implementation.)
//...

dtQueryFilter::dtQueryFilter() :
    m_includeFlags(0xffff),
    m_excludeFlags(0)
{
    for (int i = 0; i < DT_MAX_AREAS; ++i)
        m_areaCost[i] = 1.0f;
}

void NavMeshTesterTool::init(Sample* sample)
{
    m_sample = sample;
    m_navMesh = sample->getNavMesh();
    m_navQuery = sample->getNavMeshQuery();
    recalc();

    if (m_navQuery)
    {
        // Change costs.
        m_filter.setAreaCost(SAMPLE_POLYAREA_GROUND, 1.0f);
        m_filter.setAreaCost(SAMPLE_POLYAREA_WATER, 10.0f);
        m_filter.setAreaCost(SAMPLE_POLYAREA_ROAD, 1.0f);
        m_filter.setAreaCost(SAMPLE_POLYAREA_DOOR, 1.0f);
        m_filter.setAreaCost(SAMPLE_POLYAREA_GRASS, 2.0f);
        m_filter.setAreaCost(SAMPLE_POLYAREA_JUMP, 1.5f);
    }
//...
```

## PS:

```c++
g_pNavManager->FindPath(m_enemies[i]->GetCurPolyRef(), m_enemies[i]->GetDestPolyRef(), s, m_enemies[i]->GetDest(),
	m_enemies[i]->GetPath(), &nEdges);

bool NavigationManager::FindPath(unsigned int sRef, unsigned int eRef, float* s, float* e, OUT dtPolyRef* path, OUT int* nPath)
{
	dtQueryFilter* pFilter = xnew<dtQueryFilter>();
	dtStatus result = m_navQuery->findPath((dtPolyRef)sRef, (dtPolyRef)eRef, s, e, pFilter,
	path, nPath, MAX_POLYS);

    return result == DT_SUCCESS;

}
```

여태까지 edge라는 표현을 썼는 데 틀렸다. FindPath()는 두 지점간에 포함되는 폴리곤들의 목록을 반환한다이다.

## 경로 생성

그리고 closestPointOnPoly가 polyRef를 다른 값을 보내줘도 같은 지점을 반환하는 경우가 생긴다. 최종목적지랑 가까운
지점을 연속으로 찾아서 직선으로 이동하려던 게 실패했다. 결국은 FindPath()만으로 처리못해서 폴리곤 목록으로부터
실제 경로(x,y,z)를 계산해주는 데모에 있는 함수를 가져왔다.

```c++
void EnemyManager::BuildPath(int idx, float curCoord[3])
{
	m_enemies[idx]->ResetNode();
	m_enemies[idx]->UpdateCurPoly();
	SetRandomDest(idx);

    int nPoly = 0;
    NavigationManager::GetInstance().FindPath(m_enemies[idx]->GetCurPolyRef(), m_enemies[idx]->GetDestPolyRef(), curCoord, m_enemies[idx]->GetDest(), m_enemies[idx]->GetPath(), &nPoly);
    unsigned int nNodes = 0;
    NavigationManager::GetInstance().BuildNodes(nPoly, m_enemies[idx]->GetPath(), m_enemies[idx]->GetDest(), curCoord, &nNodes, m_enemies[idx]->GetNodes());
    m_enemies[idx]->SetNumNodes(nNodes);

}


void		NavigationManager::BuildNodes(UINT n, unsigned int* pPath, float* pDst, float* src, OUT unsigned int* nPath, OUT float* smoothPath)
{
    if (m_npolys)
    {
        // Iterate over the path to find smooth path on the detail mesh surface.
        dtPolyRef polys[MAX_POLYS];
        memcpy(polys, m_polys, sizeof(dtPolyRef)*m_npolys);
        int npolys = m_npolys;

        float iterPos[3], targetPos[3];
        m_navQuery->closestPointOnPoly(m_startRef, m_spos, iterPos, 0);
        m_navQuery->closestPointOnPoly(polys[npolys-1], m_epos, targetPos, 0);

        static const float STEP_SIZE = 0.5f;
        static const float SLOP = 0.01f;

        m_nsmoothPath = 0;

        dtVcopy(&m_smoothPath[m_nsmoothPath*3], iterPos);
        m_nsmoothPath++;

        // Move towards target a small advancement at a time until target reached or
        // when ran out of memory to store the path.
        while (npolys && m_nsmoothPath < MAX_SMOOTH)
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
            m_navQuery->moveAlongSurface(polys[0], iterPos, moveTgt, &m_filter,
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
                if (m_nsmoothPath < MAX_SMOOTH)
                {
                    dtVcopy(&m_smoothPath[m_nsmoothPath*3], iterPos);
                    m_nsmoothPath++;
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
                    polys[i-npos] = polys[i];
                npolys -= npos;

                // Handle the connection.
                dtStatus status = m_navMesh->getOffMeshConnectionPolyEndPoints(prevRef, polyRef, startPos, endPos);
                if (dtStatusSucceed(status))
                {
                    if (m_nsmoothPath < MAX_SMOOTH)
                    {
                        dtVcopy(&m_smoothPath[m_nsmoothPath*3], startPos);
                        m_nsmoothPath++;
                        // Hack to make the dotted path not visible during off-mesh connection.
                        if (m_nsmoothPath & 1)
                        {
                            dtVcopy(&m_smoothPath[m_nsmoothPath*3], startPos);
                            m_nsmoothPath++;
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
            if (m_nsmoothPath < MAX_SMOOTH)
            {
                dtVcopy(&m_smoothPath[m_nsmoothPath*3], iterPos);
                m_nsmoothPath++;
            }
        }
    }



    if (m_nsmoothPath)
    {
        dd.depthMask(false);
        const unsigned int spathCol = duRGBA(0,0,0,220);
        dd.begin(DU_DRAW_LINES, 3.0f);
        for (int i = 0; i < m_nsmoothPath; ++i)
            dd.vertex(m_smoothPath[i*3], m_smoothPath[i*3+1]+0.1f, m_smoothPath[i*3+2], spathCol);
        dd.end();
        dd.depthMask(true);
    }
}
```

폴리곤 목록이이 있다면 일단 사본을 만들고, 시작/목적지를 closestPointOnPoly()로 구한다. 그 직후 경로간 허용 기울기,
거리를 설정한다. 첫 시작점을 복사한다.

```c++

 if (m_npolys)
{
    // Iterate over the path to find smooth path on the detail mesh surface.
    dtPolyRef polys[MAX_POLYS];
    memcpy(polys, m_polys, sizeof(dtPolyRef)*m_npolys);
    int npolys = m_npolys;

    float iterPos[3], targetPos[3];
    m_navQuery->closestPointOnPoly(m_startRef, m_spos, iterPos, 0);
    m_navQuery->closestPointOnPoly(polys[npolys-1], m_epos, targetPos, 0);

    static const float STEP_SIZE = 0.5f;
    static const float SLOP = 0.01f;

    m_nsmoothPath = 0;

    dtVcopy(&m_smoothPath[m_nsmoothPath*3], iterPos);
    m_nsmoothPath++;
    //..
}
```

남은 폴리곤이 있고 최대 경로를 넘기지 않는 선에서 경로 생성 작업을 반복한다. 현위치에서 목적지를 향해 나아갈  
지점을 찾는다 해당 지점이 경로 끝이거나 메시 밖(?)이고 STEP보다 거리가 짧다면 해당 지점을 지나지 않는다.  
moveTgt을 현위치+delta값으로 초기화한다.

```c++

 while (npolys && m_nsmoothPath < MAX_SMOOTH)
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
    //...
}
```

이동을 하고 결과를 iterpos에 대입 한다. 경로 끝이거나 steerPos와 차이가 크지 않다면 종료한다.
메시 밖이면서 steerPos와 차이가 크지 않아도 처리가 되는 것 같은데 이부분을 잘 모르겠다.

```c++
 // Move
    float result[3];
    dtPolyRef visited[16];
    int nvisited = 0;
    m_navQuery->moveAlongSurface(polys[0], iterPos, moveTgt, &m_filter,
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

```
