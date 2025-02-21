// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "../../../../Plugins/EnhancedInput/Source/EnhancedInput/Public/InputActionValue.h"
#include "TPSPlayer.generated.h"

DECLARE_DELEGATE(FMyDelegate)
//DECLARE_DELEGATE_OneParam(FDelegate_One, int32)
//DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDelegate_DynamicMulti)

DECLARE_MULTICAST_DELEGATE_OneParam(FInputBindingDelegate, class UEnhancedInputComponent*)

UCLASS()
class TPSPROJECT_API ATPSPlayer : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ATPSPlayer();
	//FMyDelegate TestDelegate;
	FInputBindingDelegate InputBindingDelegate;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


public:
	UPROPERTY( EditDefaultsOnly , Category = Camera)
	class USpringArmComponent* springArmComp;

	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	class UCameraComponent* tpsCamComp;

	// 총 스켈레탈 메시
	UPROPERTY( VisibleAnywhere, Category = GunMesh )
	class USkeletalMeshComponent* GunMeshComp;




	// 스나이퍼건 스태틱메시 추가
	UPROPERTY(VisibleAnywhere, Category = GunMesh)
	class UStaticMeshComponent*	SniperGunMesh;

	// Input
public:
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputMappingContext*	IMC_TPS;
		
	UPROPERTY()
	class UTPSPlayerAnimInstance* Anim;


	UPROPERTY(EditDefaultsOnly)
	class UPlayerMoveComponent* MoveComp;
	//UPROPERTY(EditAnywhere)
	//class UPlayerFireComponent* FireComp;
};
