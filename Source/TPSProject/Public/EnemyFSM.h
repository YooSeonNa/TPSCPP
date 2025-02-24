// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "EnemyFSM.generated.h"

// 사용할 상태 정의
UENUM(BlueprintType)
enum class EEnemyState : uint8
{
	Idle,
	Move,
	Attack,
	Damage,
	Die,
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TPSPROJECT_API UEnemyFSM : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UEnemyFSM();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	// 상태 변수
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = FSM)
	EEnemyState mState = EEnemyState::Idle;

	// 대기 상태
	void IdleState();
	
	// 이동 상태
	void MoveState();

	// 공격 상태
	void AttackState();

	// 피격 상태
	void DamageState();

	// 죽음 상태
	void DieState();


	// 대기 시간
	UPROPERTY(EditDefaultsOnly, Category = FSM)
	float IdleDelayTime = 2.0f;

	// 경과 시간 
	float currentTime = 0.0f;
	
	// 타켓
	UPROPERTY(VisibleAnywhere, Category = FSM)
	class ATPSPlayer* target;

	// 소유 액터
	UPROPERTY()
	class AEnemy* me;


	// 공격 범위
	UPROPERTY(EditAnywhere, Category = FSM)
	float attackRange = 150.0f;

	// 공격 대기 시간
	UPROPERTY(EditAnywhere, Category = FSM)
	float attackDelayTime = 2.0f;


	// 피격 알림 이벤트 함수
	void OnDamageProcess(int32 damage);

	int32 MaxHP = 3;

	// 체력
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = FSM)
	int32 hp = MaxHP;

	// 피격 대기 시간
	UPROPERTY(EditAnywhere, Category = FSM)
	float damageDelayTime = 2.0f;

	// 아래로 사라지는 속도
	UPROPERTY( EditAnywhere, Category = FSM )
	float dieSpeed = 50.0f;


	// 사용 중인 애니메이션 블루프린트
	UPROPERTY()
	class UEnemyAnim* Anim;

	// Enemy를 소유하고 있는 AIController
	UPROPERTY()
	class AAIController* ai;


	// 길찾기 수행시 랜덤 위치
	FVector randomPos;
	// 랜덤위치 가져오기
	bool GetRandomPositionInNavMesh(FVector centerLocation, float radius, FVector& dest);



	void OnAttackEnd();

	// 애니메이션 재생이 끝나면 아래로 내려가게 하고 싶다.
	bool bDieDone = false;

	// 원래는 이렇게 쓰지만
	void OnDieEnd2();
	
	// INLINE 함수로 이렇게도 쓸 수 있다
	FORCEINLINE void OnDieEnd() {	bDieDone = true;	}


	UPROPERTY()
	class UEnemyHPWidget* HPWidget;

};
