// Source/SuperSimple/Character/SSEnemyAIController.h
#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "SSEnemyAIController.generated.h"

UCLASS()
class SUPERSIMPLE_API ASSEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaTime) override;
	void SetTargetActor(AActor* NewTarget);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) // 디버깅을 위해 보이게 설정
	AActor* TargetActor;
};