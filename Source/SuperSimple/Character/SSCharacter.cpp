// Source/SuperSimple/Character/SSCharacter.cpp
#include "SSCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Core/SSGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SuperSimple/Combat/SSProjectile.h"
#include "Components/AudioComponent.h"

ASSCharacter::ASSCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

    FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
    FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
    FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f));
    FirstPersonCameraComponent->bUsePawnControlRotation = true;

    Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
    Mesh1P->SetOnlyOwnerSee(true);
    Mesh1P->SetupAttachment(FirstPersonCameraComponent);
    Mesh1P->bCastDynamicShadow = false;
    Mesh1P->SetCastShadow(false);
    Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));

    MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
    MuzzleLocation->SetupAttachment(Mesh1P);
    MuzzleLocation->SetRelativeLocation(FVector(60.0f, 10.0f, 150.0f));

    CurrentHealth = MaxHealth;
}

void ASSCharacter::BeginPlay()
{
    Super::BeginPlay();
    CurrentHealth = MaxHealth;

    if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
        {
            Subsystem->AddMappingContext(DefaultMappingContext, 0);
        }
    }
    
    if (CurrentWeapon)
    {
        SwapWeapon(CurrentWeapon);
    }
}

void ASSCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (!bIsDead)
    {
        UpdateTimeDilation(DeltaTime);
    }
}

// --- 데미지 처리 ---
void ASSCharacter::TakeDamage_Implementation(float DamageAmount, AActor* DamageCauser)
{
    if (bIsDead) return;

    CurrentHealth--;
    OnHitVisuals();

    if (CurrentHealth <= 0)
    {
        bIsDead = true;

        // 게임 오버 시 시간 정상화
        UGameplayStatics::SetGlobalTimeDilation(this, 1.0f);
        this->CustomTimeDilation = 1.0f;

        if (APlayerController* PC = Cast<APlayerController>(Controller))
        {
            DisableInput(PC);
        }

        if (ASSGameMode* GM = Cast<ASSGameMode>(UGameplayStatics::GetGameMode(this)))
        {
            GM->GameOver(); // C++ GameOver 호출 -> 거기서 BP OnGameOver 호출 예정
        }
    }
}

void ASSCharacter::Heal(int32 Amount)
{
    if (bIsDead) return;
    if (CurrentHealth < MaxHealth)
    {
        CurrentHealth = FMath::Clamp(CurrentHealth + Amount, 0, MaxHealth);
        
        OnHealVisuals();
        
        UE_LOG(LogTemp, Log, TEXT("Player Healed! Current HP: %d"), CurrentHealth);
    }
}

void ASSCharacter::RestartLevel()
{
    UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()), false);
}

// --- 시간 조절 ---
void ASSCharacter::UpdateTimeDilation(float DeltaTime)
{
    FVector Velocity = GetVelocity();
    Velocity.Z = 0.f;
    float CurrentSpeed = Velocity.Size();

    float MaxSpeed = GetCharacterMovement()->MaxWalkSpeed;
    float TargetTimeScale = UKismetMathLibrary::MapRangeClamped(CurrentSpeed, 0.0f, MaxSpeed, MinTimeDilation, 1.0f);

    float CurrentTimeScale = UGameplayStatics::GetGlobalTimeDilation(this);
    float NewTimeScale = FMath::FInterpTo(CurrentTimeScale, TargetTimeScale, DeltaTime, 10.0f);

    UGameplayStatics::SetGlobalTimeDilation(this, NewTimeScale);

    if (NewTimeScale > KINDA_SMALL_NUMBER)
    {
        this->CustomTimeDilation = 1.0f / NewTimeScale;
    }
}

// --- 입력 ---
void ASSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
        EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
        EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASSCharacter::Move);
        EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASSCharacter::Look);
        EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &ASSCharacter::Fire);
    }
}

void ASSCharacter::Move(const FInputActionValue& Value)
{
    FVector2D MovementVector = Value.Get<FVector2D>();
    if (Controller != nullptr)
    {
        AddMovementInput(GetActorForwardVector(), MovementVector.Y);
        AddMovementInput(GetActorRightVector(), MovementVector.X);
    }
}

void ASSCharacter::Look(const FInputActionValue& Value)
{
    FVector2D LookAxisVector = Value.Get<FVector2D>();
    if (Controller != nullptr)
    {
        AddControllerYawInput(LookAxisVector.X);
        AddControllerPitchInput(LookAxisVector.Y);
    }
}

void ASSCharacter::Fire()
{
    // 1. 기본 체크 (무기 없음, 장전 중이면 발사 불가)
    if (!CurrentWeapon || !ProjectileClass || bIsReloading) return;

    // 2. 연사 속도 체크 (쿨타임 안 찼으면 무시)
    double CurrentTime = GetWorld()->GetTimeSeconds();
    
    // 원래 딜레이(0.2초) / 1.5배 = 0.133초 (빨라짐)
    float EffectiveFireDelay = CurrentWeapon->FireRate_Player / FireRateMultiplier;
    if (CurrentTime - LastFireTime < EffectiveFireDelay) return;

    // 3. 탄약 없음 체크 (0발 이하면 발사 불가)
    if (CurrentAmmo <= 0)
    {
        // 빈 탄창 소리 재생
        if (CurrentWeapon->EmptyMagSound)
        {
            FVector MuzzleLoc = MuzzleLocation->GetComponentLocation();
            UGameplayStatics::PlaySoundAtLocation(this, CurrentWeapon->EmptyMagSound, MuzzleLoc, 1.0f, CurrentWeapon->PlayerPitch);
        }
        return; // ★ 여기서 함수를 끝내야 마이너스로 안 내려갑니다!
    }

    // --- 발사 로직 시작 ---

    // 4. 탄약 차감 (★ 반복문 밖에서 1만 차감)
    CurrentAmmo--;
    
    // UI 갱신
    OnUpdateAmmoUI(CurrentAmmo, CurrentWeapon->MaxAmmo);

    // 5. 발사 사운드 재생
    FVector SpawnLocation = MuzzleLocation->GetComponentLocation();
    FRotator BaseRotation = FirstPersonCameraComponent->GetComponentRotation();

    if (CurrentWeapon->FireSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, CurrentWeapon->FireSound, SpawnLocation, 1.0f, CurrentWeapon->PlayerPitch);
    }
    
    // 6. 총구 화염 (Muzzle Flash)
    if (CurrentWeapon->MuzzleFlashEffect) // (변수 추가하셨다면)
    {
        UGameplayStatics::SpawnEmitterAttached(CurrentWeapon->MuzzleFlashEffect, MuzzleLocation, NAME_None, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTarget);
    }

    // 7. 반동 적용
    AddControllerPitchInput(CurrentWeapon->Recoil); // (변수 추가하셨다면)

    // 8. 실제 총알 생성 (여기가 반복문!)
    for (int32 i = 0; i < CurrentWeapon->BulletsPerShot; i++)
    {
        FRotator FinalRotation = BaseRotation;

        // 탄 퍼짐 적용
        if (CurrentWeapon->SpreadAngle > 0.0f)
        {
            float RandomPitch = FMath::RandRange(-CurrentWeapon->SpreadAngle, CurrentWeapon->SpreadAngle);
            float RandomYaw = FMath::RandRange(-CurrentWeapon->SpreadAngle, CurrentWeapon->SpreadAngle);
            FinalRotation.Pitch += RandomPitch;
            FinalRotation.Yaw += RandomYaw;
        }

        FActorSpawnParameters ActorSpawnParams;
        ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
        ActorSpawnParams.Owner = this;
        ActorSpawnParams.Instigator = this;

        AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(ProjectileClass, SpawnLocation, FinalRotation, ActorSpawnParams);

        if (ASSProjectile* Bullet = Cast<ASSProjectile>(SpawnedActor))
        {
            // [수정 2] 탄속 증가 적용
            float FinalSpeed = CurrentWeapon->ProjectileSpeed * ProjectileSpeedMultiplier;
            Bullet->SetProjectileSpeed(FinalSpeed);
            
            // 특수 능력 전달 (기존 유지)
            Bullet->SetupSpecialEffects(BonusRicochetCount, BonusPierceCount);
        }
    }
}

void ASSCharacter::Reload()
{
    // 이미 장전 중이거나, 탄약이 꽉 찼거나, 무기가 없으면 무시
    if (bIsReloading || !CurrentWeapon || CurrentAmmo >= CurrentWeapon->MaxAmmo) return;

    bIsReloading = true;
    
    // 원래 2.0초 / 2.0배 = 1.0초 (빨라짐)
    float EffectiveReloadTime = CurrentWeapon->ReloadTime / ReloadSpeedMultiplier;

    // 사운드 재생
    if (CurrentWeapon->ReloadSound)
    {
        // 1. 오디오 컴포넌트 생성 (또는 재사용)
        if (!ReloadAudioComp)
        {
            ReloadAudioComp = UGameplayStatics::SpawnSoundAttached(CurrentWeapon->ReloadSound, GetRootComponent());
        }
        else
        {
            ReloadAudioComp->SetSound(CurrentWeapon->ReloadSound);
        }

        // 2. 피치(속도) 계산: (소리 길이 / 장전 시간)
        // 예: 소리 1초 / 장전 2초 = 0.5배속 (천천히)
        float SoundDuration = CurrentWeapon->ReloadSound->Duration;
        float TargetPitch = 1.0f;
        
        if (SoundDuration > 0.0f && EffectiveReloadTime > 0.0f)
        {
            TargetPitch = SoundDuration / EffectiveReloadTime;
        }
        
        if (ReloadAudioComp)
        {
            ReloadAudioComp->SetPitchMultiplier(TargetPitch);
            ReloadAudioComp->Play();
        }
    }

    // UI에 "장전 바 보여줘" 신호 보냄
    OnReloadStart(EffectiveReloadTime);

    // 타이머 설정 (ReloadTime 뒤에 FinishReload 호출)
    FTimerHandle ReloadTimerHandle;
    GetWorldTimerManager().SetTimer(ReloadTimerHandle, this, &ASSCharacter::FinishReload, EffectiveReloadTime, false);
    
    UE_LOG(LogTemp, Log, TEXT("Reloading... Time: %f"), CurrentWeapon->ReloadTime);
}

void ASSCharacter::FinishReload()
{
    bIsReloading = false;
    
    if (ReloadAudioComp && ReloadAudioComp->IsPlaying())
    {
        ReloadAudioComp->Stop();
    }
    
    // 탄약 꽉 채우기
    if (CurrentWeapon)
    {
        CurrentAmmo = CurrentWeapon->MaxAmmo;
        
        // UI 갱신 (탄약 숫자)
        OnUpdateAmmoUI(CurrentAmmo, CurrentWeapon->MaxAmmo);
    }
}

// --- 무기 교체 ---
void ASSCharacter::SwapWeapon(USSWeaponData* NewWeapon)
{
    if (NewWeapon)
    {
        CurrentWeapon = NewWeapon;
        
        // 탄약 채우기
        CurrentAmmo = CurrentWeapon->MaxAmmo;

        // 메쉬 변경 신호
        if (NewWeapon->GunMesh)
        {
            OnUpdateWeaponMesh(NewWeapon->GunMesh);
        }
        
        // [추가] UI 갱신 신호 (아이콘 & 탄약)
        if (NewWeapon->WeaponIcon)
        {
            OnUpdateWeaponIcon(NewWeapon->WeaponIcon);
        }
        OnUpdateAmmoUI(CurrentAmmo, CurrentWeapon->MaxAmmo);

        UE_LOG(LogTemp, Log, TEXT("Weapon Swapped: %s (Ammo: %d)"), *NewWeapon->WeaponName.ToString(), CurrentAmmo);
    }
}