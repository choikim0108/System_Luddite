// Source/SuperSimple/Character/SSEnemy.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SuperSimple/System/SSWeaponData.h" // [중요] 무기 데이터 헤더
#include "SSEnemy.generated.h"

// 적 상태 열거형
UENUM(BlueprintType)
enum class EEnemyState : uint8
{
	Chasing,
	Aiming,
	Firing,
	Dead
};

UCLASS()
class SUPERSIMPLE_API ASSEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	ASSEnemy();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// --- 전투 및 무기 시스템 ---
    
	// 발사할 총알 클래스 (기본값, 무기 데이터 없을 때 사용)
	UPROPERTY(EditDefaultsOnly, Category = Combat)
	TSubclassOf<class AActor> ProjectileClass;

	// 기본 연사 속도
	UPROPERTY(EditAnywhere, Category = Combat)
	float FireRate = 2.0f;

	// [추가] 무기 장착 함수 (스폰 시 GameMode가 호출)
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void EquipWeapon(class USSWeaponData* NewWeaponData);

	// [추가] 현재 장착된 무기 데이터
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	class USSWeaponData* CurrentWeapon;

	// [추가] 총 모델링 컴포넌트 (BP의 Gun 컴포넌트를 여기에 연결해야 함)
	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	UStaticMeshComponent* GunMeshComp;

	// --- 상태 및 체력 ---

	// [추가] C++ 체력 변수 (무기 데이터에 따라 변함)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 CurrentHP = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 MaxHP = 1;

	// 사망 처리 함수
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void SetEnemyDead();

protected:
	void FireAtPlayer();

private:
	EEnemyState CurrentState;
	float FireTimer;
	AActor* PlayerRef;
};