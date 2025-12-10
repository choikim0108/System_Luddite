// Source/SuperSimple/Core/SSGameMode.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SSTypes.h"
#include "SSGameMode.generated.h"

// UI 업데이트를 위한 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnScoreChangedDelegate, int32, CurrentScore, int32, CurrentLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWaveChangedDelegate, int32, CurrentWave);

UCLASS()
class SUPERSIMPLE_API ASSGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    ASSGameMode();

    virtual void StartPlay() override;

    // --- 게임 로직 ---
    UFUNCTION(BlueprintCallable, Category = "GameLogic")
    void OnEnemyKilled(int32 XPAmount, AActor* Killer);

    UFUNCTION(BlueprintCallable, Category = "GameLogic")
    void GameOver();
    
    UPROPERTY(EditDefaultsOnly, Category = "Spawning")
    int32 MaxEnemiesPerWave = 20;

    // [추가] C++에서 레벨업이 확정되면 호출할 이벤트 (BP에서 구현)
    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnLevelUp();
    
    UFUNCTION(BlueprintImplementableEvent, Category = "GameLogic")
    void OnGameOver();

protected:
    void StartWave();
    void SpawnEnemy();
    void CheckLevelUp();

    // 레벨별 필요 경험치 계산 함수
    int32 GetRequiredScoreForLevel(int32 LevelToCheck);

public:
    // --- 설정 변수 ---
    UPROPERTY(EditDefaultsOnly, Category = "Spawning")
    TSubclassOf<class ACharacter> EnemyClass;

    UPROPERTY(EditDefaultsOnly, Category = "Spawning")
    int32 BaseEnemyCount = 3;

    UPROPERTY(EditDefaultsOnly, Category = "Spawning")
    float TimeBetweenWaves = 3.0f;

    // --- 상태 변수 ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    int32 CurrentWave = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    int32 EnemiesAlive = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    int32 TotalScore = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    int32 CurrentLevel = 1;

    // 기본 필요 경험치 (1레벨업 기준)
    UPROPERTY(EditDefaultsOnly, Category = "Progression")
    int32 BaseXPToLevelUp = 100;

    // 힐을 위한 킬 카운트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    int32 KillStreakForHeal = 0;

    // --- 데이터 에셋 ---
    // 플레이어 증강 목록
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Augment")
    TArray<class USSAugmentData*> AllAugments;

    // [추가] 적에게 지급할 무기 목록
    UPROPERTY(EditDefaultsOnly, Category = "Spawning")
    TArray<class USSWeaponData*> AllEnemyWeapons;

    // --- 델리게이트 ---
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnScoreChangedDelegate OnScoreChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnWaveChangedDelegate OnWaveChanged;
};