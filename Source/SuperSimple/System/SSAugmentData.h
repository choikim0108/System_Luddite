// Source/SuperSimple/System/SSAugmentData.h
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SSAugmentData.generated.h"

// 증강의 종류 정의
UENUM(BlueprintType)
enum class EAugmentType : uint8
{
	SpeedUp         UMETA(DisplayName = "Movement Speed Up"),
	MaxHealthUp     UMETA(DisplayName = "Max Health Up"),
	DamageUp        UMETA(DisplayName = "Damage Up"),
	Weapon          UMETA(DisplayName = "New Weapon"), // 새로운 무기

	ReloadSpeed     UMETA(DisplayName = "Reload Speed"), // 재장전 속도
	FireRate        UMETA(DisplayName = "Fire Rate"),    // 연사 속도
	ProjectileSpeed UMETA(DisplayName = "Projectile Speed"), // 투사체 속도
	Ricochet        UMETA(DisplayName = "Ricochet Rounds"), // 도탄 (벽 튕김)
	Pierce          UMETA(DisplayName = "Piercing Rounds"), // 관통 (적 뚫음)
};

UCLASS(BlueprintType)
class SUPERSIMPLE_API USSAugmentData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// UI에 표시될 이름
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Augment")
	FText Title;

	// UI에 표시될 설명
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Augment")
	FText Description;

	// 아이콘 (선택 사항, 없으면 텍스트만 표시)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Augment")
	UTexture2D* Icon;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	FLinearColor RarityColor = FLinearColor::White;

	// 증강 효과 타입
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	EAugmentType AugmentType;

	// 적용할 수치 (예: 100.0이면 속도 +100, 1.0이면 데미지 +1)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float Value;
	
	// 만약 이 변수가 채워져 있다면, 스탯 업글 대신 무기를 교체함
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reward")
	class USSWeaponData* WeaponReward;
};