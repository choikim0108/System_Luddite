// Source/SuperSimple/UI/SSHUDWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SSHUDWidget.generated.h"

UCLASS()
class SUPERSIMPLE_API USSHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
    
	// 점수 및 레벨 갱신
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Update")
	void UpdateScore(int32 NewScore, int32 NewLevel);

	// 웨이브 정보 갱신
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Update")
	void UpdateWave(int32 WaveNumber);

	// 체력 갱신
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Update")
	void UpdateHealth(int32 CurrentHP, int32 MaxHP);
	
};