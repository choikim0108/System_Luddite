// Source/SuperSimple/UI/SSUpgradeWidget.cpp
#include "SSUpgradeWidget.h"

void USSUpgradeWidget::SetupOptions(const TArray<USSAugmentData*>& Options)
{
	CurrentOptions = Options;
    
	// BP로 시각적 갱신 위임 (텍스트, 아이콘 변경)
	UpdateOptionVisuals(CurrentOptions);
}

void USSUpgradeWidget::SelectOption(int32 OptionIndex)
{
	if (CurrentOptions.IsValidIndex(OptionIndex))
	{
		USSAugmentData* Choice = CurrentOptions[OptionIndex];
        
		// 선택된 데이터를 실어 보냄
		OnAugmentSelected.Broadcast(Choice);
        
		// 위젯 닫기 (Remove From Parent는 보통 호출하는 쪽에서 처리하지만 여기서 해도 무방)
		RemoveFromParent();
	}
}