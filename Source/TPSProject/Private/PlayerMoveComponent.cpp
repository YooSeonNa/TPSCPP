// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerMoveComponent.h"
#include "EnhancedInputComponent.h"
#include "TPSPlayerAnimInstance.h"

// Sets default values for this component's properties
UPlayerMoveComponent::UPlayerMoveComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UPlayerMoveComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	//me = Cast<ATPSPlayer>(GetOwner());
	if( me == nullptr )	return;

	MoveComp = me->GetCharacterMovement();

	// 초기속도를 걷기로 설정
	//me->GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	MoveComp->MaxWalkSpeed = WalkSpeed;
	
}


// Called every frame
void UPlayerMoveComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);


	// P(결과위치) = P0(현재위치) + v(속도) x t(시간)	
	Direction = FTransform( me->GetControlRotation() ).TransformVector( Direction );
	 
	/*
	FVector P0 = GetActorLocation();
	FVector vt = Direction * WalkSpeed * DeltaTime;
	FVector P = P0 + vt;
	SetActorLocation( P );
	*/
	me->AddMovementInput( Direction );

	Direction = FVector::ZeroVector;
}

void UPlayerMoveComponent::SetupInputBinding( class UEnhancedInputComponent* input )
{
	Super::SetupInputBinding(input);

	input->BindAction( IA_Turn , ETriggerEvent::Triggered , this , &UPlayerMoveComponent::Turn );
	input->BindAction( IA_LookUp , ETriggerEvent::Triggered , this , &UPlayerMoveComponent::LookUp );

	input->BindAction( IA_Move , ETriggerEvent::Triggered , this , &UPlayerMoveComponent::Move );

	// 달리기 입력 이벤트 처리 함수 바인딩
	input->BindAction( IA_Run , ETriggerEvent::Started , this , &UPlayerMoveComponent::InputRun );
	input->BindAction( IA_Run , ETriggerEvent::Completed , this , &UPlayerMoveComponent::InputRun );

	input->BindAction( IA_Jump , ETriggerEvent::Started , this , &UPlayerMoveComponent::InputJump );

	// 쪼그려 앉기
	input->BindAction( IA_CrouchCtrl , ETriggerEvent::Started , this , &UPlayerMoveComponent::InputCrouchCtrl );
	input->BindAction( IA_CrouchCtrl , ETriggerEvent::Completed , this , &UPlayerMoveComponent::InputUnCrouchUnCtrl );
	input->BindAction( IA_CrouchC , ETriggerEvent::Started , this , &UPlayerMoveComponent::InputCrouchToggle );
}



void UPlayerMoveComponent::Turn( const FInputActionValue& inputValue )
{
	float value = inputValue.Get<float>();
	me->AddControllerYawInput( value );
}

void UPlayerMoveComponent::LookUp( const FInputActionValue& inputValue )
{
	float value = inputValue.Get<float>();
	me->AddControllerPitchInput( value );
}


void UPlayerMoveComponent::Move( const FInputActionValue& inputValue )
{
	FVector2D value = inputValue.Get<FVector2D>();

	// 상하 입력 처리
	Direction.X = value.X;

	// 좌우 입력 처리
	Direction.Y = value.Y;
}


void UPlayerMoveComponent::InputRun()
{
	//auto movement = GetCharacterMovement();
	//if( movement == nullptr )	return;
	if( MoveComp == nullptr )	return;

	// 현재 달리기 모드면
	if( MoveComp->MaxWalkSpeed > WalkSpeed )
	{
		// 걷기 속도로 전환
		MoveComp->MaxWalkSpeed = WalkSpeed;
	}
	else
	{
		MoveComp->MaxWalkSpeed = RunSpeed;
	}
}

void UPlayerMoveComponent::InputJump( const FInputActionValue& inputValue )
{
	me->Jump();
}


void UPlayerMoveComponent::InputCrouchCtrl()
{
	me->Anim->IsCrouched = true;
	me->Crouch();
}

void UPlayerMoveComponent::InputUnCrouchUnCtrl()
{
	me->Anim->IsCrouched = false;
	me->UnCrouch();
}

void UPlayerMoveComponent::InputCrouchToggle()
{
	bCrouched = !bCrouched;
	me->Anim->IsCrouched = bCrouched;
	if( bCrouched == true )
	{
		me->Crouch();
	}
	else
	{
		me->UnCrouch();
	}
}

