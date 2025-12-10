// ReSharper disable once UnrealHeaderToolError
// Source/SuperSimple/Character/SSCharacter.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "SuperSimple/Combat/SSDamageInterface.h"
#include "SuperSimple/System/SSWeaponData.h" // [중요] 무기 데이터 헤더 포함
#include "Camera/CameraShakeBase.h"
#include "SSCharacter.generated.h"


class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;

UCLASS()
class SUPERSIMPLE_API ASSCharacter : public ACharacter, public ISSDamageInterface
{
    GENERATED_BODY()

public:
    
    ASSCharacter();
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

    // --- 데미지 인터페이스 ---
    virtual void TakeDamage_Implementation(float DamageAmount, AActor* DamageCauser) override;

    // --- 체력 회복 ---
    UFUNCTION(BlueprintCallable, Category = "Health")
    void Heal(int32 Amount);

    // --- [추가] 무기 교체 함수 ---
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SwapWeapon(USSWeaponData* NewWeapon);

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
    void OnUpdateWeaponMesh(UStaticMesh* NewMesh);
    
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void Reload();
    
    UFUNCTION(BlueprintImplementableEvent, Category = "UI")
    void OnUpdateAmmoUI(int32 AmmoCount, int32 MaxAmmoCount);

    UFUNCTION(BlueprintImplementableEvent, Category = "UI")
    void OnUpdateWeaponIcon(UTexture2D* NewIcon);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "UI")
    void OnReloadStart(float Duration);
    
    // --- 컴포넌트 ---
    UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
    USkeletalMeshComponent* Mesh1P;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
    UCameraComponent* FirstPersonCameraComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
    USceneComponent* MuzzleLocation;

    // --- 입력 ---
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UInputMappingContext* DefaultMappingContext;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UInputAction* JumpAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UInputAction* MoveAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UInputAction* LookAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UInputAction* FireAction;

    // --- 전투 설정 ---
    UPROPERTY(EditDefaultsOnly, Category = Projectile)
    TSubclassOf<class AActor> ProjectileClass;

    // [추가] 현재 장착한 무기 데이터
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    USSWeaponData* CurrentWeapon;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    int32 CurrentAmmo;
    
    
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    int32 BonusRicochetCount = 0; // 도탄 횟수

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    int32 BonusPierceCount = 0;   // 관통 횟수
    
    // 연사 속도 배율 (기본 1.0, 높을수록 빨라짐)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float FireRateMultiplier = 1.0f;

    // 재장전 속도 배율 (기본 1.0, 높을수록 빨라짐)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float ReloadSpeedMultiplier = 1.0f;

    // 탄속 배율 (기본 1.0, 높을수록 빨라짐)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float ProjectileSpeedMultiplier = 1.0f;
    
    // [설정] 사격 시 사용할 카메라 셰이크 블루프린트 클래스
    UPROPERTY(EditDefaultsOnly, Category = "Combat|Effects")
    TSubclassOf<UCameraShakeBase> FireCameraShakeClass;

    // [설정] 피격 시 사용할 카메라 셰이크 블루프린트 클래스
    UPROPERTY(EditDefaultsOnly, Category = "Combat|Effects")
    TSubclassOf<UCameraShakeBase> HitCameraShakeClass;

    // 카메라 셰이크 재생 도우미 함수
    void PlayCameraShake(TSubclassOf<UCameraShakeBase> ShakeClass, float Scale = 1.0f);

    // --- 슈퍼핫 설정 ---
    UPROPERTY(EditAnywhere, Category = "SuperHot")
    float MinTimeDilation = 0.05f;

    // --- 체력 시스템 ---
    // [수정] ReadWrite로 변경하여 BP에서 수정 가능하게 함
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
    int32 MaxHealth = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
    int32 CurrentHealth;

    // 피격 시각 효과 이벤트
    UFUNCTION(BlueprintImplementableEvent, Category = "Health")
    void OnHitVisuals();

    // 회복 시각 효과 이벤트
    UFUNCTION(BlueprintImplementableEvent, Category = "Health")
    void OnHealVisuals();

protected:
    virtual void BeginPlay() override;
    
    // 마지막으로 발사한 시간 저장, 연사 속도 제한용
    double LastFireTime = 0.0;
    
    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    void Fire(); // [수정됨] 무기 데이터 기반 발사
    void FinishReload();
    
    void UpdateTimeDilation(float DeltaTime);
    void RestartLevel();

    bool bIsDead = false;
    bool bIsReloading = false;
    
    UPROPERTY()
    UAudioComponent* ReloadAudioComp;
    
    FTimerHandle FireTimerHandle;
    bool bIsFiring = false; // 마우스 누르고 있는지 확인용

    // 발사 시작/중지 (Input Binding용)
    UFUNCTION(BlueprintCallable)
    void StartFire();
    UFUNCTION(BlueprintCallable)
    void StopFire();
};