// Source/SuperSimple/Core/SSGameMode.cpp
#include "SSGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerStart.h"
#include "SuperSimple/Character/SSCharacter.h" // 플레이어 힐 용도
#include "SuperSimple/Character/SSEnemy.h"     // 적 무기 장착 용도
#include "SuperSimple/System/SSWeaponData.h"   // 무기 데이터

ASSGameMode::ASSGameMode()
{
    PrimaryActorTick.bCanEverTick = true;
    CurrentWave = 0;
    EnemiesAlive = 0;
    TotalScore = 0;
    CurrentLevel = 1;
    KillStreakForHeal = 0;
    BaseXPToLevelUp = 100;
}

void ASSGameMode::StartPlay()
{
    Super::StartPlay();
    StartWave();
}

void ASSGameMode::StartWave()
{
    CurrentWave++;
    OnWaveChanged.Broadcast(CurrentWave);

    // 기본 + (웨이브 * 2)
    int32 EnemiesToSpawn = BaseEnemyCount + (CurrentWave * 2);
    
    // 최대 마릿수 제한 (Clamp)
    // 아무리 웨이브가 높아져도 MaxEnemiesPerWave 이상은 안 나옴
    EnemiesToSpawn = FMath::Min(EnemiesToSpawn, MaxEnemiesPerWave);
    
    EnemiesAlive = EnemiesToSpawn;

    for (int32 i = 0; i < EnemiesToSpawn; i++)
    {
        SpawnEnemy();
    }
}

void ASSGameMode::SpawnEnemy()
{
    if (!EnemyClass) return;

    // 플레이어 위치 가져오기
    AActor* PlayerActor = UGameplayStatics::GetPlayerPawn(this, 0);
    FVector PlayerLoc = (PlayerActor) ? PlayerActor->GetActorLocation() : FVector::ZeroVector;

    TArray<AActor*> SpawnPoints;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), SpawnPoints);

    if (SpawnPoints.Num() > 0)
    {
        // [수정] 유효한 스폰 포인트를 찾을 때까지 최대 10번 시도
        AActor* ChosenSpawn = nullptr;
        for (int32 i = 0; i < 10; i++) 
        {
            int32 RandomIndex = FMath::RandRange(0, SpawnPoints.Num() - 1);
            AActor* Candidate = SpawnPoints[RandomIndex];
            
            // 플레이어와 거리가 15미터(1500) 이상인 곳만 선택
            if (PlayerActor && Candidate->GetDistanceTo(PlayerActor) > 1500.0f)
            {
                ChosenSpawn = Candidate;
                break; // 찾았으니 루프 탈출
            }
        }

        // 10번 시도해도 없으면 그냥 랜덤(억까 방지용 안전장치)
        if (!ChosenSpawn) 
        {
            ChosenSpawn = SpawnPoints[FMath::RandRange(0, SpawnPoints.Num() - 1)];
        }

        FVector SpawnLocation = ChosenSpawn->GetActorLocation();
        SpawnLocation.X += FMath::RandRange(-200.0f, 200.0f);
        SpawnLocation.Y += FMath::RandRange(-200.0f, 200.0f);
        SpawnLocation.Z += 50.0f; // 바닥 걸림 방지

        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        // 적 스폰
        ACharacter* SpawnedActor = GetWorld()->SpawnActor<ACharacter>(EnemyClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);

        if (ASSEnemy* Enemy = Cast<ASSEnemy>(SpawnedActor))
        {
            TArray<USSWeaponData*> CandidateWeapons;
            TArray<float> Weights;
            float TotalWeight = 0.0f;

            // 1. 후보 추리기 및 가중치 계산
            for (USSWeaponData* Weapon : AllEnemyWeapons)
            {
                if (Weapon && CurrentLevel >= Weapon->MinLevelToSpawn)
                {
                    CandidateWeapons.Add(Weapon);
                
                    // 레벨이 높을수록 고티어 무기의 확률 보정
                    // 기본 가중치 + (현재 레벨이 높으면 추가 가산점)
                    // 예: 샷건(가중치 10)은 레벨 5가 되면 가중치가 더 늘어남
                    float EffectiveWeight = Weapon->SpawnWeight + (CurrentLevel * 0.5f);
                
                    Weights.Add(EffectiveWeight);
                    TotalWeight += EffectiveWeight;
                }
            }

            // 2. 가중치 뽑기 (룰렛 돌리기)
            if (CandidateWeapons.Num() > 0)
            {
                float RandomValue = FMath::FRandRange(0.0f, TotalWeight);
                float CurrentSum = 0.0f;
                USSWeaponData* SelectedWeapon = CandidateWeapons[0]; // 기본값

                for (int32 i = 0; i < CandidateWeapons.Num(); i++)
                {
                    CurrentSum += Weights[i];
                    if (RandomValue <= CurrentSum)
                    {
                        SelectedWeapon = CandidateWeapons[i];
                        break;
                    }
                }
            
                // 3. 장착
                Enemy->EquipWeapon(SelectedWeapon);
            }
        }
    }
}

void ASSGameMode::OnEnemyKilled(int32 XPAmount, AActor* Killer)
{
    // 점수는 누가 죽였든 올라갈 수 있음 (선택 사항)
    // 하지만 TotalScore 로직은 BP_Enemy에서 0점으로 막아뒀다면 여기서도 안 오름
    TotalScore += XPAmount;
    EnemiesAlive--;
    
    // [핵심 수정] 킬러가 "플레이어"일 때만 힐 스트릭 증가
    // (GetPlayerPawn과 Killer가 같은지 비교)
    if (Killer == UGameplayStatics::GetPlayerPawn(this, 0))
    {
        KillStreakForHeal++;
        
        // 5킬 달성 시 힐
        if (KillStreakForHeal >= 5)
        {
            KillStreakForHeal = 0;
            if (ASSCharacter* SSPlayer = Cast<ASSCharacter>(Killer))
            {
                SSPlayer->Heal(1); 
            }
        }
    }

    // UI 갱신
    OnScoreChanged.Broadcast(TotalScore, CurrentLevel);

    // 레벨업 체크
    CheckLevelUp();

    // 웨이브 종료 체크 -> 다음 웨이브
    if (EnemiesAlive <= 0)
    {
        FTimerHandle WaveTimerHandle;
        GetWorld()->GetTimerManager().SetTimer(WaveTimerHandle, this, &ASSGameMode::StartWave, TimeBetweenWaves, false);
    }
}

void ASSGameMode::CheckLevelUp()
{
    // 점진적 난이도 공식 적용
    int32 RequiredScore = GetRequiredScoreForLevel(CurrentLevel + 1);

    if (TotalScore >= RequiredScore)
    {
        CurrentLevel++;
        
        // 레벨업 했으니 UI 한번 더 갱신 (레벨 숫자 바뀜)
        OnScoreChanged.Broadcast(TotalScore, CurrentLevel);
        
        // [중요] 블루프린트에게 "레벨업 화면 띄워!" 명령 (Event On Level Up)
        OnLevelUp();
        
        UE_LOG(LogTemp, Warning, TEXT("LEVEL UP! New Level: %d"), CurrentLevel);
    }
}

int32 ASSGameMode::GetRequiredScoreForLevel(int32 LevelToCheck)
{
    // 경험치 공식: 100, 250(100+150), 450(250+200)...
    int32 TotalReq = 0;
    int32 CurrentReq = BaseXPToLevelUp; // 초기값 100

    // 1레벨부터 목표 레벨 직전까지 누적
    for (int32 i = 1; i < LevelToCheck; i++)
    {
        TotalReq += CurrentReq;
        CurrentReq += 50; // 난이도 상승폭 (매 렙업마다 필요 경험치 통이 50씩 커짐)
    }

    return TotalReq;
}

void ASSGameMode::GameOver()
{
    OnGameOver();

    // 여기서 점수 집계나 저장 로직?
}