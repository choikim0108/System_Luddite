// Source/SuperSimple/Core/SSTypes.h
#pragma once

#include "CoreMinimal.h"
#include "SSTypes.generated.h"

// 게임의 현재 상태를 정의
UENUM(BlueprintType)
enum class ESSGameState : uint8
{
	Playing     UMETA(DisplayName = "Playing"),
	Paused      UMETA(DisplayName = "Paused/Menu"),
	GameOver    UMETA(DisplayName = "Game Over"),
	LevelUp     UMETA(DisplayName = "Level Up Selection")
};

// 투사체의 주인을 구분 (팀 구분)
UENUM(BlueprintType)
enum class EProjectileOwner : uint8
{
	Player      UMETA(DisplayName = "Player"),
	Enemy       UMETA(DisplayName = "Enemy")
};