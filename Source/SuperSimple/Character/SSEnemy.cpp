// Source/SuperSimple/Character/SSEnemy.cpp
#include "SSEnemy.h"
#include "SSEnemyAIController.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h" // GunMeshComp용
#include "SuperSimple/Combat/SSProjectile.h"

ASSEnemy::ASSEnemy()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // AI 컨트롤러 자동 할당
    AIControllerClass = ASSEnemyAIController::StaticClass();
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void ASSEnemy::BeginPlay()
{
    Super::BeginPlay();

    CurrentState = EEnemyState::Chasing;
    PlayerRef = UGameplayStatics::GetPlayerPawn(this, 0);

    // AI 컨트롤러에게 타겟 전달
    if (ASSEnemyAIController* AICon = Cast<ASSEnemyAIController>(GetController()))
    {
        AICon->SetTargetActor(PlayerRef);
    }
}

void ASSEnemy::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // 죽었으면 로직 중단
    if (CurrentState == EEnemyState::Dead) return;

    if (!PlayerRef) return;

    float DistanceToPlayer = GetDistanceTo(PlayerRef);

    switch (CurrentState)
    {
    case EEnemyState::Chasing:
        // 사거리(800) 안에 들어오면 조준 시작
        if (DistanceToPlayer <= 800.0f)
        {
            CurrentState = EEnemyState::Aiming;
        }
        break;

    case EEnemyState::Aiming:
        {
            // 플레이어의 머리(head) 위치 조준
            FVector TargetLocation = PlayerRef->GetActorLocation(); // 기본값
            
            if (ACharacter* PlayerChar = Cast<ACharacter>(PlayerRef))
            {
                if (PlayerChar->GetMesh()->DoesSocketExist("head"))
                {
                    TargetLocation = PlayerChar->GetMesh()->GetSocketLocation("head");
                }
            }

            // 회전 로직
            FRotator LookAtRot = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), TargetLocation);
            FRotator NewRot = FMath::RInterpTo(GetActorRotation(), LookAtRot, DeltaTime, 5.0f);
            SetActorRotation(NewRot);

            // 조준 완료 (오차 5도 이내)
            if (FMath::Abs(NewRot.Yaw - LookAtRot.Yaw) < 5.0f)
            {
                CurrentState = EEnemyState::Firing;
                FireTimer = FireRate;
            }

            // 멀어지면 다시 추격
            if (DistanceToPlayer > 1000.0f)
            {
                CurrentState = EEnemyState::Chasing;
            }
        }
        break;

    case EEnemyState::Firing:
        FireTimer -= DeltaTime;
        if (FireTimer <= 0.f)
        {
            FireAtPlayer();
            FireTimer = FireRate;
        }
        
        // 사격 중에도 계속 머리 조준
        {
            FVector TargetLocation = PlayerRef->GetActorLocation();
            if (ACharacter* PlayerChar = Cast<ACharacter>(PlayerRef))
            {
                if (PlayerChar->GetMesh()->DoesSocketExist("head"))
                {
                    TargetLocation = PlayerChar->GetMesh()->GetSocketLocation("head");
                }
            }
            FRotator LookAtRot = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), TargetLocation);
            SetActorRotation(FMath::RInterpTo(GetActorRotation(), LookAtRot, DeltaTime, 10.0f));
        }

        // 멀어지면 사격 중지
        if (DistanceToPlayer > 1000.0f)
        {
            CurrentState = EEnemyState::Chasing;
        }
        break;
    }
}

// 무기 장착 및 스탯 적용 함수
void ASSEnemy::EquipWeapon(USSWeaponData* NewWeaponData)
{
    if (!NewWeaponData) return;

    CurrentWeapon = NewWeaponData;

    // 1. 총 모델링 변경 (GunMeshComp가 BP에서 연결되어 있어야 함)
    if (GunMeshComp && CurrentWeapon->GunMesh)
    {
        GunMeshComp->SetStaticMesh(CurrentWeapon->GunMesh);
    }

    // 2. 적 스탯 적용
    GetCharacterMovement()->MaxWalkSpeed = CurrentWeapon->EnemyWalkSpeed;
    
    // 체력 설정 (요청하신 부분 주석 해제)
    MaxHP = CurrentWeapon->EnemyMaxHealth;
    CurrentHP = MaxHP;

    // 3. 발사 속도 적용
    FireRate = CurrentWeapon->FireRate_Enemy;
    
    if (GunMeshComp)
    {
        GunMeshComp->SetCollisionProfileName(TEXT("NoCollision"));
    }
}

void ASSEnemy::FireAtPlayer()
{
    // 총알 클래스가 없거나 무기 데이터가 없으면 기본 발사
    if (!ProjectileClass) return;

    FVector SpawnLoc = GetActorLocation() + (GetActorForwardVector() * 100.0f) + FVector(0,0,50);
    FRotator BaseRot = GetActorRotation();

    // 1. 사운드 재생
    if (CurrentWeapon && CurrentWeapon->FireSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, CurrentWeapon->FireSound, GetActorLocation(), 1.0f, CurrentWeapon->EnemyPitch);
    }

    // 2. 발사 설정값 가져오기 (데이터가 없으면 기본값)
    int32 NumBullets = (CurrentWeapon) ? CurrentWeapon->BulletsPerShot : 1;
    float Spread = (CurrentWeapon) ? CurrentWeapon->SpreadAngle_Enemy : 5.0f;
    float Speed = (CurrentWeapon) ? CurrentWeapon->ProjectileSpeed : 3000.0f;

    // 3. 반복문으로 산탄 발사
    for (int32 i = 0; i < NumBullets; i++)
    {
        FRotator FinalRot = BaseRot;

        // 탄 퍼짐 적용
        if (Spread > 0.0f)
        {
            float RandPitch = FMath::RandRange(-Spread, Spread);
            float RandYaw = FMath::RandRange(-Spread, Spread);
            FinalRot.Pitch += RandPitch;
            FinalRot.Yaw += RandYaw;
        }

        FActorSpawnParameters Params;
        Params.Owner = this;
        Params.Instigator = this;

        // 총알 스폰
        AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(ProjectileClass, SpawnLoc, FinalRot, Params);
        
        // 스폰된 총알의 속도 설정
        if (ASSProjectile* Bullet = Cast<ASSProjectile>(SpawnedActor))
        {
            Bullet->SetProjectileSpeed(Speed);
        }
    }
}

void ASSEnemy::SetEnemyDead()
{
    CurrentState = EEnemyState::Dead;
    
    // 1. AI 및 캡슐 끄기
    if (GetController())
    {
        GetController()->StopMovement();
        GetController()->UnPossess();
    }
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // 2. 총 충돌 완벽 차단
    if (GunMeshComp)
    {
        GunMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        GunMeshComp->SetCollisionProfileName(TEXT("NoCollision"));
    }

    // 3. 틱(Tick) 끄기
    SetActorTickEnabled(false);
}