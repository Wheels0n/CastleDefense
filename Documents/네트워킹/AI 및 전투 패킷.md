# AI 및 전투 패킷

잠시 없앴던 적도 다시 복구해야한다. 근데 서버에서 BT와 충돌도 구현해야한다.  
쉽지 않다.

패킷은 스폰, 디스폰(사망), 공격, 이동 크게 4개로 잡았다.

## AI 스폰

스폰은 유저가 처음 입장할 떄 한 번 해주고 죽을 떄(디스폰)마다. 일정 주기로 다시 스폰하기로 했다.  
5마리를 서버에서 관리한다. PlayerInfo와 마찬가지로 서버에 EnemyInfo와 그 매니저를 만들었다.  
그전에 패킷을 먼저 추가 했다.

Enemy는 Player와 같다. 다만 id는 배열 색인을 담고, moveState는 점프가 제외 된다. 추적할때만  
Sprint가 된다.

```c++
message Enemy
{
	required int32 id=1;
	required int32 hp=2;
	required Coordiante coord=3;
	required Rotation rot=4;
	required MoveState moveState=5;
}
```

AI 동작은 순수히 서버에서 판단하므로 클라는 그저 서버에서 받아오기만 한다.

```c++
message S_EnemySpawn
{
	repeated Enemy enemy=1;
}

message S_EnemyDespawn
{
	required int32 id=1;
}
```

플레이어가 적을 공격하여 정말로 충돌이 발생하면 공격 결과를 실제 Enemy값으로 전송한다.  
만약 허공에 공격하거나 그냥 다른 플레이어에 유효타가 발생한 경우는 Enemy의 아이디 값에  
 -1과 같은 유효하지 않은 값을 넣어서 구분짓는다.

```c++
message C_Attack
{
	required int32 id=1;
}

message S_Attack
{
	required Enemy enemy=1;
}
```

서버가 켜지면 매니저 클래스가 미리 enemyInfo 배열을 채운다.

```c++
EnemyManager::EnemyManager()
	:m_enemyLock(4)
{
	for (int i = 0; i < sizeof(m_enemies)/sizeof(void*); ++i)
	{
		shared_ptr<EnemyInfo> pEnemyInfo = MakeShared<EnemyInfo>(i);
        m_enemies[i] = pEnemyInfo;
	}
}

```

플레이어가 플레이어 스폰 패킷을 보내면 응답으로 S_Spawn과 함께 적 스폰 패킷도 보내 준다.  
패킷을 채우는 건 정보를 들고 있는 매니저에서 한다.

```c++
void EnemyManager::MakeEnemySpawnPacket(S_EnemySpawn& pkt)
{
	for (int i = 0; i < sizeof(m_enemies)/sizeof(void*); ++i)
	{
		Enemy* pCurEnemy = pkt.add_enemy();
		*pCurEnemy = *(m_enemies[i]->GetEnemy());
	}

}

PacketHandler::ProcessC_Spawn(PacketHeader* pHeader)
{
	S_EnemySpawn pkt;
	g_pEnemyManager->MakeEnemySpawnPacket(pkt);
	int packetSize = sizeof(PacketHeader) + pkt.ByteSizeLong();
	shared_ptr<SendBuffer> pSendBuffer = make_shared<SendBuffer>(packetSize);
	PacketHandler::SerializeS_EnemySpawn(pkt, pSendBuffer->GetBuffer());
	shared_ptr<Session> pSession = g_pSessionManager->GetSessionById(c_spawn.id());
	pSession->RequestSend(pSendBuffer);
}
```

클라쪽에서는 받아서 게임스테이트의 에너미 추가 함수를 불러준다. 에너미의 색인 위치에 Spawn  
함수 결과를 대입한다.

```c++

void CPacketHandler::ProcessS_EnemySpawn(CPacketHeader* pHeader)
{
	S_EnemySpawn pkt = ParseS_EnemySpawn(reinterpret_cast<char*>(pHeader));
	UWorld* pCurWorld = m_pGameInstance->GetWorld();
	ACastleDefenseGameState* pGameState = pCurWorld->GetGameState<ACastleDefenseGameState>();

	for (int i = 0; i < pkt.enemy_size(); ++i)
	{
		Enemy curEnemy= pkt.enemy(i);
		pGameState->AddEnemy(&curEnemy);
	}
}

void ACastleDefenseGameState::AddEnemy(Enemy* pEnemyInfo)
{
	int32 idx = pEnemyInfo->id();

	Coordiante coord = pEnemyInfo->coord();
	FVector location = FVector(coord.x(), coord.y(), coord.z());
	FRotator rotation = FRotator::ZeroRotator;

	UWorld* pCurWorld = GetWorld();
	m_enemies[idx] = pCurWorld->SpawnActor<ASkeletonEnemy>(location,rotation);
}
```

![적 스폰_충돌x](적%20스폰_충돌X.JPG)

## 공격

게임스테이트에서 포인터와 색인을 연결 시켜주는 TMap을 만든다. 만약 플레이어 클라이언트 단의 충돌
판정이 유효하다면 적 색인을 패킷으로 담아 전송한다.

```c++
int ACastleDefenseGameState::GetEnemyIndexByPtr(ASkeletonEnemy* pEnemy)
{
	return m_enemyPtrToIdx.Find(pEnemy) == nullptr ?
		-1 : m_enemyPtrToIdx[pEnemy];
}


void AWizard::CheckPlayerAttack()
{
	UWorld* pWorld = GetWorld();
	if (pWorld)
	{
		FHitResult hitResult;
		FVector start = GetActorLocation();
		FVector end = start + GetActorForwardVector() * 500.f;
		FCollisionObjectQueryParams queryObjParams;
		queryObjParams.AddObjectTypesToQuery(ECollisionChannel::ECC_Pawn);
		FCollisionQueryParams queryParams;
		queryParams.AddIgnoredActor(this);
		pWorld->LineTraceSingleByObjectType(hitResult, start, end, queryObjParams, queryParams);
		DrawDebugLine(pWorld, start, end, FColor::Red, false, 5.0f);
		if (hitResult.bBlockingHit)
		{
			AActor* pOther = hitResult.GetActor();
			if (pOther&&pOther->IsA(ASkeletonEnemy::StaticClass()))
			{
				UE_LOG(LogTemp, Display, TEXT("EnemyGotHit"));
				ASkeletonEnemy* pEnemy = Cast<ASkeletonEnemy>(pOther);

				UWorld* pCurWorld = GetWorld();
				ACastleDefenseGameState* pGameState = pCurWorld->GetGameState<ACastleDefenseGameState>();
				int idx = pGameState->GetEnemyIndexByPtr(pEnemy);

				UCastleDefenseGameInstance* pGameInstance = pCurWorld->GetGameInstance<UCastleDefenseGameInstance>();
				TSharedPtr<ClientSession> pSession = pGameInstance->GetSession();
				pSession->SendC_Attack(idx);
			}
		}
	}
}
```

서버에서는 메모리에 있는 적 개체의 hp를 감소하고 S_Attack으로 해당 적 개체를 직렬화해서  
보낸다.

```c++
void PacketHandler::ProcessC_Attack(PacketHeader* pHeader)
{
	C_Attack pkt = ParseC_Attack((char*)pHeader);
	g_pEnemyManager->DecreaseHp(pkt.id());

	S_Attack response;
	shared_ptr<EnemyInfo> pEnemyInfo = g_pEnemyManager->GetPlayerById(pkt.id());
	response.set_allocated_enemy(pEnemyInfo->GetEnemy());

	int packetSize = sizeof(PacketHeader) + pkt.ByteSizeLong();
	shared_ptr<SendBuffer> pSendBuffer = make_shared<SendBuffer>(packetSize);
	SerializeS_Attack(response, pSendBuffer->GetBuffer());
	g_pSessionManager->Brodcast(pSendBuffer, nullptr);

	response.set_allocated_enemy(pEnemyInfo->GetEnemy());
}

```

이 패킷을 받은 클라이언트는 게임스테이트에서 idx값을 통해서 적 체력 감소 함수를 호출한다. 지금 보니 id값만 보내고,  
Despawn패킷은 필요 없다. 변경 해야겠다.

```c++
void ACastleDefenseGameState::UpdateEnemyHp(Enemy* pEnemy)
{
	int idx = pEnemy->id();
	m_enemies[idx]->DecreaseHp();
}
```

### ~~공격 판정~~

클라이언트 측에서 라인 트레이스 방식을 썼는 데 서버는 만약 정해진 거리를 넘어서면 그냥 패킷을 씹으려고 한다.
원래는 강제 종료 시키도록 해야하는 게 맞다고 한다. 정석이라면 AABB에 행렬에, 직선과 평면이 교차하는 지까지  
해줘야하지만 그냥 현재 플레이어와의 거리가 510, 500이니까 약간의 오차를 감안해서, 이 넘어가면 무시할 것이다.

```c++
FVector start = GetActorLocation();
FVector end = start + GetActorForwardVector() * 500.f;
```

클라이언트 측에서 공격자와 피격자의 id 값을 패킷에 넣도록 다시 변경했다.

```c++
message C_Attack
{
	required int32 Attacker=1;
	required int32 Target=2;
}
```

간단히 유클리드 공식만 썼다.

```c++
void PacketHandler::ProcessC_Attack(PacketHeader* pHeader)
{
	C_Attack pkt = ParseC_Attack((char*)pHeader);

	//서버 판정
	shared_ptr<PlayerInfo> pPlayerInfo = g_pPlayerManager->GetPlayerById(pkt.attacker());
	Coordiante* pPlayerCoord = pPlayerInfo->GetCoord();
	shared_ptr<EnemyInfo> pEnemyInfo = g_pEnemyManager->GetEnemyById(pkt.target());
	Coordiante* pEnemyCoord = pEnemyInfo->GetCoord();
	int xLen = abs(pEnemyCoord->x() - pPlayerCoord->x());
	int yLen = abs(pEnemyCoord->y() - pPlayerCoord->y());

	bool bInRange = sqrt(xLen * xLen + yLen * yLen)<510;
	if (bInRange)
	{
		g_pEnemyManager->DecreaseHp(pkt.target());
		cout << pkt.attacker() << " attack " << pkt.target() << endl;
		S_Attack response;

		response.set_allocated_enemy(pEnemyInfo->GetEnemy());

		int packetSize = sizeof(PacketHeader) + response.ByteSizeLong();
		shared_ptr<SendBuffer> pSendBuffer = make_shared<SendBuffer>(packetSize);
		SerializeS_Attack(response, pSendBuffer->GetBuffer());
		g_pSessionManager->Brodcast(pSendBuffer, nullptr);

		response.release_enemy();
	}

}
```

다른 플레이어의 공격 모션이 보이지 않는 데 player패킷에 공격여부를 bool로 처리하게했다. S_Attack에 넣어 버리려 했는 데 이러면 공격 모션 종료가 문제다. move 갱신시에 같이 처리하도록 했다. actionState도 넣을 까 했는 데 공격 말고 없다.

```c++
void ACastleDefenseGameState::UpdatePlayerPos(Player* pPlayer)
{
	AWizard* pCurPlayer = m_idToPlayer[pPlayer->id()];
	pCurPlayer->SetNewDest(pPlayer);

	if (pPlayer->battack())
	{
		pCurPlayer->StartAttack();
	}
	else
	{
		pCurPlayer->StopAttack();
	}
}
```

## AI이동

스레드 개수를 더 늘리고 EnemyMove패킷을 브로드 캐스트 하도록 하였다. 30fps 0.03s(33ms)주기로 시뮬레이션, 0.2초 주기로  
클라이언트로 갱신 패킷을 보낸다. 클라이언트에있는 BT는 아예 사용하지 않기로 했다.

```c++
void	IocpManager::RunIOMain()
{
	SessionManager::GetInstance().PrepareSessions();

	while (1)
	{
		SessionManager::GetInstance().AcceptSessions();
		//TODO : IOCP 매니저 코드로부터 적 코드 분리
		EnemyManager::GetInstance().SetNextLocation();
		PlayerManager::GetInstance().SetNextLocation();
	}

};


void PacketHandler::BrodcastS_EnemyMove()
{
	S_EnemyMove pkt;
	g_pEnemyManager->SetRandomDest();
	g_pEnemyManager->AddEnemyToPacket(pkt);
	int packetSize = sizeof(PacketHeader) + pkt.ByteSizeLong();
	shared_ptr<SendBuffer> pSendBuffer = make_shared<SendBuffer>(packetSize);
	PacketHandler::SerializeS_EnemyMovement(pkt, pSendBuffer->GetBuffer());
	g_pSessionManager->Brodcast(pSendBuffer, nullptr);
}

void EnemyManager::SetNextLocation()
{
	//적상태 tick단위 갱신. 일정 주기 move패킷 전송
	//시간 갱신시 알아서 보내도록 함
	//시뮬이랑 전송 타이머 분리
	_STD_TIME cur = std::chrono::high_resolution_clock::now();
	const std::chrono::duration<double> diff = cur - m_lastTime;
	m_lastTime = cur;

	m_brodcastTime -= diff.count();
	m_simTime -= diff.count();
	if (m_simTime > 0.0f)
	{
		return;
	}
	m_simTime = _SIMULATION_TIME;
	//...
}
```

이동 시뮬 코드는 아래와 같다. 목적지에 도달했다면 경로를 새로 BuildNodes()로 새 경로를 만든다. 그후 dir벡터 _ 걷기 속도 _ dt(시뮬주기) 만큼 이동하고 중간 노드에 도착 했는 지 확인한다. 중간 노드에 도착 했다면 dir와 yaw를 갱신한다. 이동할떄마다 AABB와 옥트리도 갱신한다.

```c++

void EnemyManager::SetNextLocation()
{
	for (int i = 0; i < sizeof(m_enemies) / sizeof(void*); ++i)
	{

		float	curCoord[3] = { 0, };
		Coordiante* pCoord	= m_enemies[i]->GetCoord();
		MiscHelper::ConvertUE2Nav(pCoord, curCoord, _ENEMY_OFFSET);

		if (m_enemies[i]->CheckReachedEnd())
		{
			BuildPath(i, curCoord);
			m_enemies[i]->MoveAlongPath();
			float* nextNode = m_enemies[i]->GetNextNode();

			//Dir갱신
			float dir[3] = { 0, };
			CalculateDir(curCoord, nextNode, dir);
			float noOffset[3] = {0,};
			Coordiante* pEnemyDir = m_enemies[i]->GetDir();
			MiscHelper::ConvertNav2UE(pEnemyDir, dir, noOffset);
			continue;
		}

		float* nextNode = m_enemies[i]->GetNextNode();
		if (nextNode[ENUM_TO_INT(_NAV_COMP::right)] == curCoord[ENUM_TO_INT(_NAV_COMP::right)]
			&& nextNode[ENUM_TO_INT(_NAV_COMP::forward)] == curCoord[ENUM_TO_INT(_NAV_COMP::forward)])
		{
			m_enemies[i]->SetIsNewDest(true);
			if (m_enemies[i]->CheckReachedEnd())
			{
				BuildPath(i, curCoord);
			}

			m_enemies[i]->MoveAlongPath();
			float* nextNode			 = m_enemies[i]->GetNextNode();
			MiscHelper::ConvertNav2UE(pCoord, nextNode, _ENEMY_OFFSET);

			AABB* pAABB = m_enemies[i]->GetAABB();
			Octree::GetInstance().RemoveFromNode(m_enemies[i]->GetAABB());

			pAABB->center[ENUM_TO_INT(_NAV_COMP::right)] = nextNode[ENUM_TO_INT(_NAV_COMP::right)];
			pAABB->center[ENUM_TO_INT(_NAV_COMP::forward)] = nextNode[ENUM_TO_INT(_NAV_COMP::forward)];
			pAABB->center[ENUM_TO_INT(_NAV_COMP::up)] = nextNode[ENUM_TO_INT(_NAV_COMP::up)];
			pAABB->SetExtent(_ENEMY_EXTENT);

			Octree::GetInstance().PlaceInNode(m_enemies[i]->GetAABB());

			//Dir갱신
			float dir[3]			= { 0, };
			CalculateDir(curCoord, nextNode, dir);

			Coordiante* pEnemyDir	= m_enemies[i]->GetDir();
			MiscHelper::ConvertNav2UE(pEnemyDir, dir, noOffset);

			//Yaw갱신
			float yaw		= CalculateYawByDir(dir);
			Rotation* pRot	= m_enemies[i]->GetRot();
			pRot->set_y(yaw);
			//cout << m_enemies[i]->GetEnemy()->id() << " Yaw : " << yaw <<"dir "<<dir[0]<<" " << dir[2]<<"\n";

		}
		else
		{
			Coordiante* pEnemyDir = m_enemies[i]->GetDir();
			float dir[3] = { 0};
			MiscHelper::ConvertUE2Nav(pEnemyDir, dir, noOffset);
			//0.01s(client), 0.02s(server)
			dir[ENUM_TO_INT(_NAV_COMP::forward)] *= _ENEMY_WALK_VEL *_SIMULATION_TIME;
			dir[ENUM_TO_INT(_NAV_COMP::right)] *= _ENEMY_WALK_VEL *_SIMULATION_TIME;

			float* nextNode = m_enemies[i]->GetNextNode();
			// 음수좌표에 유의
			float right = dir[ENUM_TO_INT(_NAV_COMP::right)] < 0
				? max(nextNode[ENUM_TO_INT(_NAV_COMP::right)], curCoord[ENUM_TO_INT(_NAV_COMP::right)] + dir[ENUM_TO_INT(_NAV_COMP::right)])
				: min(nextNode[ENUM_TO_INT(_NAV_COMP::right)], curCoord[ENUM_TO_INT(_NAV_COMP::right)] + dir[ENUM_TO_INT(_NAV_COMP::right)]);
			float forward = dir[ENUM_TO_INT(_NAV_COMP::forward)] < 0
				? max(nextNode[ENUM_TO_INT(_NAV_COMP::forward)], curCoord[ENUM_TO_INT(_NAV_COMP::forward)] + dir[ENUM_TO_INT(_NAV_COMP::forward)])
				: min(nextNode[ENUM_TO_INT(_NAV_COMP::forward)], curCoord[ENUM_TO_INT(_NAV_COMP::forward)] + dir[ENUM_TO_INT(_NAV_COMP::forward)]);

			float newCoord[3] = { 0, };
			newCoord[ENUM_TO_INT(_NAV_COMP::right)]		= right;
			newCoord[ENUM_TO_INT(_NAV_COMP::forward)]	= forward;
			newCoord[ENUM_TO_INT(_NAV_COMP::up)]		= curCoord[ENUM_TO_INT(_NAV_COMP::up)];

			AABB* pAABB = m_enemies[i]->GetAABB();
			Octree::GetInstance().RemoveFromNode(m_enemies[i]->GetAABB());
			pAABB->center[ENUM_TO_INT(_NAV_COMP::right)]	= newCoord[ENUM_TO_INT(_NAV_COMP::right)];
			pAABB->center[ENUM_TO_INT(_NAV_COMP::forward)]	= newCoord[ENUM_TO_INT(_NAV_COMP::forward)];
			pAABB->center[ENUM_TO_INT(_NAV_COMP::up)]		= newCoord[ENUM_TO_INT(_NAV_COMP::up)];
			pAABB->SetExtent(_ENEMY_EXTENT);

			Octree::GetInstance().PlaceInNode(m_enemies[i]->GetAABB());
			MiscHelper::ConvertNav2UE(pCoord, newCoord, _ENEMY_OFFSET);
			m_enemies[i]->SetIsNewDest(false);
		}
	}

	if (m_brodcastTime > 0.0f)
	{
		return;
	}
	ResetBrodcastTime();
	PacketHandler::BrodcastS_EnemyMove();
}
```

언리얼은 +z, 왼손좌표계인데, recast&detour는 +y, 오른손 좌표계라 골치아프니 주의.

yaw계산은 atan을 쓰면 비율만 반환해서 소용이 없고 atan2()를 써야한다. atan2의 반환 값은 라디안이라서 각도로 변환 했다.  
 하드코딩된 값은 level 애셋의 landScape 위치 값을 감안 한것이다.

```c++

void EnemyManager::SetRandomDest()
{
    for (int i = 0; i < sizeof(m_enemies) / sizeof(void*); ++i)
    {
        Coordiante* pCoord = m_enemies[i]->GetCoord();
        float dst[3];
        if (g_pNavManager->FindRandomPos(pCoord, dst))
        {
            //Set Yaw
            float dx = dst[0] - pCoord->x();
            float dy = dst[2] - pCoord->y();
            float yaw = atan2(dy , dx);
            yaw = yaw * 180.0f / 3.14f;

            Rotation* pRot = m_enemies[i]->GetRot();
            pRot->set_y(yaw);

            //Set Coord
            pCoord->set_x(dst[0]);
            pCoord->set_y(dst[2] );
            pCoord->set_z(dst[1] + 1.0f);
        }

    }

}
```

경로 생성 코드와 충돌 코드 자체는 다른 문서로 분리했다.

#### 기타 변경사항

- RequestSend()에서 sending 원자 불변수를 dequeue가 다 끝난 직후에 함. 안그러면 정작 해당 큐를 쓰는 스레드가 없는 데 일감만 넣고 빠져나오는 사태가 발생.
- 서버코어에 CRT_SECURE_NO_WARNINGS 선언
- 서버 초기화시에 실제 연결 없는데도 카운팅하던 코드 제거

#### 참조

- [언리얼 커뮤니티 : How can I export NavMesh Data](https://forums.unrealengine.com/t/how-can-i-export-navmesh-data/1846323)
- [언리얼 커뮤니티 : Problem with exporting nav mesh data](https://forums.unrealengine.com/t/problem-with-exporting-nav-mesh-data/94273)
- [언리얼 커뮤니티 : How can I export navmesh data to file?](https://forums.unrealengine.com/t/how-can-i-export-navmesh-data-to-file/306663/2)
- [[0903 구경원] recast 네비메쉬](https://www.slideshare.net/slideshow/0903-recast/10655153)
- [c++ : atan2](https://cplusplus.com/reference/cmath/atan2/)
- [언리얼 공식문서 : Landscape Technical Guide](https://dev.epicgames.com/documentation/en-us/unreal-engine/landscape-technical-guide-in-unreal-engine)
