# AI 및 전투 패킷

잠시 없앴던 적도 다시 복구해야한다. 근데 서버에서 BT와 충돌도 구현해야한다.  
쉽지 않다.

## AI 패킷

스폰, 디스폰(사망), 공격, 이동 크게 4개로 잡았다.

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

플레이어가 플레이어 스폰 패킷을 보내면 응답으로 S_Spawn과 함께 적 스폰 패킷도 보내준다.  
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

~~적 리스폰은 단순 무식하게 클라에서 보내고 중복 여부만 처리하게 했다.~~

## 서버 충돌 판정

적 공격 여부를 클라에서 판단하는 건 부정행위의 위험이있다. 또한 스폰시에 서버단에서 충돌을 감안 못해서  
 서버와 클라간의 정보가 어긋날수 있다. 그렇다고 클라에 충돌처리를 맡겨 버리면 클라마다 다르게 나올수
있어서 서버에서 꼭 처리해 줘야한다.

XYZ 각각 플레이어는 230/90/200, 적은 120/45/175 이다. 애니메이션 파일이던 메시 파일이던 에디터에서  
뷰포트에 ApproxSize로 대략적인 크기를 알수있다. 이를 위치 값에 더해서 계산할 것이다.

근데 이것도 정책이 매우 갈린다. 본인은 매우 개괄적으로 할 생각이다. 당장 급한건 BT라서 이건 시간이 나면 추가 하겠다.

## 공격 판정

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

간단히 피타고라스 공식만 썼다.

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

다른 플레이어의 공격 모션이 보이지 않는 데 player패킷에 공격여부를 bool로 처리하게했다. S_Attack에 넣어 버리려 했는 데 이러면 공격 모션 종료가 문제다.
move 갱신시에 같이 처리하도록 했다. actionState도 넣을 까 했는 데 공격 말고 없다.

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

![플레이어 공격 동기화](https://youtu.be/Lyf8seQJAPc)

적이 죽으면 리스폰 할 지를 결정 못해서 당장은 서버 메모리에 그냥 좀비 상태로 남아있다.

#### 기타 변경사항

- RequestSend()에서 sending 원자 불변수를 dequeue가 다 끝난 직후에 함. 안그러면 정작 해당 큐를 쓰는 스레드가 없는 데 일감만 넣고 빠져나오는 사태가 발생.
