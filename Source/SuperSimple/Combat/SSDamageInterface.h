// Source/SuperSimple/Combat/SSDamageInterface.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SSDamageInterface.generated.h"

// 언리얼 리플렉션 시스템용 빈 클래스
UINTERFACE(MinimalAPI)
class USSDamageInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 데미지를 받을 수 있는 모든 액터가 상속받는 인터페이스
 */
class SUPERSIMPLE_API ISSDamageInterface
{
	GENERATED_BODY()

public:
	// 데미지 처리 함수 (블루프린트에서도 구현 가능하도록 설정)
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat")
	void TakeDamage(float DamageAmount, AActor* DamageCauser);
};