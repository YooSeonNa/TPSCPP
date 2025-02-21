// Fill out your copyright notice in the Description page of Project Settings.


#include "TPSPlayerAnimInstance.h"
#include "TPSPlayer.h"
#include "GameFramework/CharacterMovementComponent.h"

void UTPSPlayerAnimInstance::NativeUpdateAnimation( float DeltaSeconds )
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	ATPSPlayer* player = Cast<ATPSPlayer>(TryGetPawnOwner());
	if( player == nullptr )
	{
		return;
	}

	FVector velocity = player->GetVelocity();
	FVector forwardVector = player->GetActorForwardVector();

	// 앞 / 뒤 이동속도
	Speed = FVector::DotProduct(forwardVector, velocity);

	// 좌 / 우 이동속도
	FVector rightVector = player->GetActorRightVector();
	Direction = FVector::DotProduct(rightVector, velocity);

	// 플레이어가 현재 공중에 있는지 저장	
	auto movement = player->GetCharacterMovement();
	if( movement != nullptr )
	{
		isInAir = movement->IsFalling();
	}	

	//isInAir = player->GetCharacterMovement()->IsFalling();

	// 주인공의 AimRotation값을 가져와서 Pitch값을 채우고 싶다.
	Pitch = player->GetBaseAimRotation().Pitch;
}

void UTPSPlayerAnimInstance::PlayAttackAnim()
{
	if( AttackAnimMontage == nullptr )	return;
	Montage_Play(AttackAnimMontage);
}
