// Source/SuperSimple/UI/SSUpgradeWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SuperSimple/System/SSAugmentData.h"
#include "SSUpgradeWidget.generated.h"

// 선택 완료 시 GameMode나 Player에게 알리기 위한 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAugmentSelected, USSAugmentData*, SelectedData);

UCLASS()
class SUPERSIMPLE_API USSUpgradeWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// GameMode가 이 함수를 호출해 데이터 3개를 넣어줍니다.
	UFUNCTION(BlueprintCallable, Category = "Setup")
	void SetupOptions(const TArray<USSAugmentData*>& Options);

	// 블루프린트의 버튼 클릭 이벤트에서 이 함수를 호출합니다.
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SelectOption(int32 OptionIndex);

	// 선택 이벤트 발송자
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnAugmentSelected OnAugmentSelected;

protected:
	// UI 갱신용 이벤트 (BP에서 버튼 텍스트 등을 바꿈)
	UFUNCTION(BlueprintImplementableEvent, Category = "Setup")
	void UpdateOptionVisuals(const TArray<USSAugmentData*>& Options);

private:
	// 현재 제안된 옵션들 저장
	UPROPERTY()
	TArray<USSAugmentData*> CurrentOptions;
};