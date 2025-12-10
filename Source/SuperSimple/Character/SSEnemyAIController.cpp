// Source/SuperSimple/Character/SSEnemyAIController.cpp
#include "SSEnemyAIController.h"
#include "Kismet/GameplayStatics.h" // ★ 플레이어를 찾기 위해 필수
#include "AITypes.h"

void ASSEnemyAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (TargetActor == nullptr)
	{
		TargetActor = UGameplayStatics::GetPlayerPawn(this, 0);
	}

	if (TargetActor && GetPawn())
	{
		float Distance = GetPawn()->GetDistanceTo(TargetActor);

		if (Distance > 800.0f)
		{
			FAIRequestID RequestID = MoveToActor(TargetActor, 50.0f);
            
			if (RequestID.IsValid())
			{
				GEngine->AddOnScreenDebugMessage(1, 0.0f, FColor::Green, FString::Printf(TEXT("Moving... Dist: %f"), Distance));
			}
			else
			{
				GEngine->AddOnScreenDebugMessage(1, 0.0f, FColor::Red, TEXT("Move Failed! (NavMesh Check Needed)"));
			}
		}
		else
		{
			StopMovement();
			GEngine->AddOnScreenDebugMessage(1, 0.0f, FColor::Yellow, TEXT("Stopped (In Range)"));
		}
	}
}


void ASSEnemyAIController::SetTargetActor(AActor* NewTarget)
{
	TargetActor = NewTarget;
}