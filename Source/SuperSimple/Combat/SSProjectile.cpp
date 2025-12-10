// Source/SuperSimple/Combat/SSProjectile.cpp
#include "SSProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "SSDamageInterface.h"

ASSProjectile::ASSProjectile()
{
    // 1. 충돌체 설정
    CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
    CollisionComp->InitSphereRadius(5.0f);
    CollisionComp->BodyInstance.SetCollisionProfileName("Projectile");
    
    // 워킹(Walking) 중인 캐릭터의 캡슐과도 충돌하도록 설정
    CollisionComp->OnComponentHit.AddDynamic(this, &ASSProjectile::OnHit);
    
    CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
    CollisionComp->CanCharacterStepUpOn = ECB_No;
    
    RootComponent = CollisionComp;

    // 2. 이동 컴포넌트 설정
    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
    ProjectileMovement->UpdatedComponent = CollisionComp;
    ProjectileMovement->InitialSpeed = 3000.f;
    ProjectileMovement->MaxSpeed = 3000.f;
    ProjectileMovement->bRotationFollowsVelocity = true;
    
    // 기본 도탄 제거 및 중력 제거
    ProjectileMovement->bShouldBounce = false;
    ProjectileMovement->ProjectileGravityScale = 0.0f; // 직선으로 날아감

    // 3초 뒤 자동 삭제 (메모리 관리)
    InitialLifeSpan = 3.0f;
}

void ASSProjectile::BeginPlay()
{
    Super::BeginPlay();
    
    // [추가] 겹침 이벤트 연결
    if (CollisionComp)
    {
        CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &ASSProjectile::OnOverlap);
    }
}

void ASSProjectile::SetProjectileSpeed(float NewSpeed)
{
    if (ProjectileMovement)
    {
        ProjectileMovement->MaxSpeed = NewSpeed;
        ProjectileMovement->InitialSpeed = NewSpeed;
        ProjectileMovement->Velocity = ProjectileMovement->Velocity.GetSafeNormal() * NewSpeed;
    }
}

void ASSProjectile::SetupSpecialEffects(int32 Ricochet, int32 Pierce)
{
    // 1. 도탄 설정
    CurrentRicochetCount = Ricochet;
    if (CurrentRicochetCount > 0 && ProjectileMovement)
    {
        ProjectileMovement->bShouldBounce = true;
        ProjectileMovement->Bounciness = 0.6f;
        ProjectileMovement->Friction = 0.0f;
        
        // 튕김 이벤트 구독 (횟수 제한)
        ProjectileMovement->OnProjectileBounce.AddDynamic(this, &ASSProjectile::OnBounce);
    }

    // 2. 관통 설정
    CurrentPierceCount = Pierce;
}

// 튕길 때마다 호출
void ASSProjectile::OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity)
{
    // 횟수 차감
    CurrentRicochetCount--;

    UE_LOG(LogTemp, Log, TEXT("Bounced! Left: %d"), CurrentRicochetCount);

    // 더 이상 튕길 수 없으면 물리 반사 끄기
    if (CurrentRicochetCount <= 0 && ProjectileMovement)
    {
        ProjectileMovement->bShouldBounce = false;
        // 이벤트 구독 해제
        ProjectileMovement->OnProjectileBounce.RemoveDynamic(this, &ASSProjectile::OnBounce);
    }
}

// [신규] 적 관통/데미지 처리는 여기서 담당
void ASSProjectile::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
    // 1. 유효성 및 중복 피격 검사
    if ((OtherActor == nullptr) || (OtherActor == this) || (OtherActor == GetInstigator())) return;
    if (HitActors.Contains(OtherActor)) return; // 이미 맞춘 적 패스

    // 2. 적(Enemy)인지 확인 (태그 또는 인터페이스)
    bool bIsEnemy = OtherActor->ActorHasTag("Enemy") || OtherActor->GetClass()->ImplementsInterface(USSDamageInterface::StaticClass());

    if (bIsEnemy)
    {
        HitActors.Add(OtherActor); // 명단 등록

        // 데미지 주기
        if (OtherActor->GetClass()->ImplementsInterface(USSDamageInterface::StaticClass()))
        {
            ISSDamageInterface::Execute_TakeDamage(OtherActor, Damage, this);
        }

        // 이펙트 (피)
        if (ImpactEffect)
        {
            FVector EffectLoc; // 변수 먼저 선언

            if (bFromSweep)
            {
                // SweepResult.Location은 FVector_NetQuantize 타입이지만,
                // 대입 연산(=)을 할 때는 FVector로 자연스럽게 변환됩니다.
                EffectLoc = SweepResult.Location; 
            }
            else
            {
                EffectLoc = OtherActor->GetActorLocation();
            }
            
            UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactEffect, EffectLoc, GetActorRotation());
        }

        // --- 관통 로직 (이제 멈추지 않으니 속도 복구 불필요!) ---
        if (CurrentPierceCount > 0)
        {
            //CurrentPierceCount--; // 횟수만 까고 계속 날아감 > 이것도 bp로 이관
            UE_LOG(LogTemp, Log, TEXT("Pierced Enemy! (Overlap) Left: %d"), CurrentPierceCount);
        }
        else
        {
            // 관통 횟수 다 썼으면 소멸 > bp로 이관
            //Destroy();
        }
    }
}

// [수정] 벽/도탄 처리는 여기서 담당
void ASSProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    // 적은 이제 Overlap으로 처리되므로 OnHit이 발생하지 않음.
    // 즉, 여기 들어왔다는 건 "벽"이나 "못 뚫는 물체"에 맞았다는 뜻.

    if ((OtherActor != nullptr) && (OtherActor != this) && (OtherActor != GetInstigator()))
    {
        // 벽 데미지/이펙트 처리 (필요하면)
        if (ImpactEffect)
        {
            UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactEffect, Hit.Location, Hit.ImpactNormal.Rotation());
        }

        // 도탄 로직
        if (ProjectileMovement && ProjectileMovement->bShouldBounce)
        {
            // 벽에 맞았으니 튕김 (엔진이 알아서 함)
        }
        else
        {
            // 도탄 아니면 파괴
            Destroy();
        }
    }
}



