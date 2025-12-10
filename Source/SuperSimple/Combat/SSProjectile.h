// Source/SuperSimple/Combat/SSProjectile.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SSProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UParticleSystem;

UCLASS()
class SUPERSIMPLE_API ASSProjectile : public AActor
{
	GENERATED_BODY()

public:
	ASSProjectile();
	
	// 외부 설정 함수
	void SetProjectileSpeed(float NewSpeed);
	void SetupSpecialEffects(int32 Ricochet, int32 Pierce);
	
	UPROPERTY(VisibleDefaultsOnly, Category = Projectile)
	USphereComponent* CollisionComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement)
	UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float Damage = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float ImpulseStrength = 2000.0f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	UParticleSystem* ImpactEffect;

protected:
	virtual void BeginPlay() override;

	// 충돌 처리 (관통 로직용)
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	// 튕김 처리 (도탄 횟수 차감용)
	UFUNCTION()
	void OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity);
	
	// 남은 횟수 저장 변수
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	int32 CurrentRicochetCount = 0;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	int32 CurrentPierceCount = 0;
	
	// 이미 맞춘 적들을 기억하는 리스트
	UPROPERTY()
	TArray<AActor*> HitActors;
	
	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
};