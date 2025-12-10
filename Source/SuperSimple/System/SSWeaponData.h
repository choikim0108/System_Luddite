// Source/SuperSimple/System/SSWeaponData.h
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SSWeaponData.generated.h"

UCLASS(BlueprintType)
class SUPERSIMPLE_API USSWeaponData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// --- 비주얼 ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	FText WeaponName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	UStaticMesh* GunMesh; // 총 모델링

	// --- 사운드 (피치 조절) ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound")
	USoundBase* FireSound; // 공용 사운드 파일
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound")
	USoundBase* EmptyMagSound; // 공용 사운드 파일
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound")
	USoundBase* ReloadSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound")
	float PlayerPitch = 1.0f; // 플레이어가 쏠 때 톤 (예: 1.0)

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound")
	float EnemyPitch = 0.8f;  // 적이 쏠 때 톤 (예: 0.8 = 낮고 무거운 소리)

	// --- 전투 성능 (공용) ---
	// 플레이어 연사 속도 (0.1 = 초당 10발, 빠름)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float FireRate_Player = 0.2f;

	// 적 연사 속도 (1.0 = 초당 1발, 느림)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float FireRate_Enemy = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	int32 BulletsPerShot = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float SpreadAngle = 0.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float SpreadAngle_Enemy = 5.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float ProjectileSpeed = 3000.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float Recoil = -0.5f; // 플레이어용 반동 (음수 = 위로 튐)
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float ReloadTime = 2.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	int32 MaxAmmo = 30; //(무제한으로 하려면 -1)

	// --- 적 스폰 설정 (이 무기를 든 적의 스탯) ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "EnemyConfig")
	int32 EnemyMaxHealth = 1; // 샷건맨은 3, 소총수는 1 등

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "EnemyConfig")
	float EnemyWalkSpeed = 600.0f; // SMG는 800, 샷건은 400 등

	// 스폰 조건 (플레이어 레벨이 이 값 이상일 때만 등장)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpawnRules")
	int32 MinLevelToSpawn = 1;
    
	// 등장 확률 가중치 (높을수록 자주 나옴)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpawnRules")
	float SpawnWeight = 1.0f; 
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	UParticleSystem* MuzzleFlashEffect; // 또는 UNiagaraSystem
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	UTexture2D* WeaponIcon;
};