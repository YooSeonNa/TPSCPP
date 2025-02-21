// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyFSM.h"
#include "Kismet/GameplayStatics.h"
#include "TPSPlayer.h"
#include "Enemy.h"
#include <AIController.h>
#include <NavigationSystem.h>
#include "Navigation/PathFollowingComponent.h"
#include "EnemyAnim.h"
#include "TPSProject.h"
#include "Components/CapsuleComponent.h"

// Sets default values for this component's properties
UEnemyFSM::UEnemyFSM()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UEnemyFSM::BeginPlay()
{
	Super::BeginPlay();

	// ...
	// 월드에서 ATPSPlayer 찾아오기
	auto actor = UGameplayStatics::GetActorOfClass(GetWorld(), ATPSPlayer::StaticClass());
	
	// ATPSPlayer 타입으로 캐스팅
	target = Cast<ATPSPlayer>(actor);
	
	// 소유 객체 가져오기
	me = Cast<AEnemy>(GetOwner());	

	// UEnemyAnim 할당
	Anim = Cast<UEnemyAnim>(me->GetMesh()->GetAnimInstance());

	// AIController 할당하기
	ai = Cast<AAIController>( me->GetController() );
}


// Called every frame
void UEnemyFSM::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 실행창에 상태 메세지 출력하기
	FString logMsg = UEnum::GetValueAsString(mState);
	GEngine->AddOnScreenDebugMessage(0, 1, FColor::Red, logMsg);

	// ...
	switch(mState)
	{
		case EEnemyState::Idle:		{ IdleState();		}	break;
		case EEnemyState::Move:		{ MoveState();		}	break;
		case EEnemyState::Attack:	{ AttackState();	}	break;
		case EEnemyState::Damage:	{ DamageState();	}	break;
		case EEnemyState::Die:		{ DieState();		}	break;
	}
}

void UEnemyFSM::IdleState()
{
	// 시간이 흐르다가
	currentTime += GetWorld()->DeltaTimeSeconds;

	// 만약 경과 시간이 대기시간을 초과했다면
	if( currentTime > IdleDelayTime )
	{
		// 이동 상태로 전환하고 싶다.
		mState = EEnemyState::Move;

		// 경과 시간 초기화
		currentTime = 0.0f;
		
		// 애니메이션 상태 동기화
		Anim->AnimState = mState;

		// 새로운 랜덤 위치 가져오기
		GetRandomPositionInNavMesh( me->GetActorLocation() , 500.0f , randomPos );
	}
}

void UEnemyFSM::MoveState()
{
	// 타겟 목적지가 필요하다.
	FVector destination = target->GetActorLocation();

	// 방향이 필요하다
	FVector dir = destination - me->GetActorLocation();

	auto ns = UNavigationSystemV1::GetNavigationSystem(GetWorld());

	// 목적지 길찾기 경로 데이터 검색
	FPathFindingQuery query;
	FAIMoveRequest req;

	// 목적지에서 인지할 수 있는 범위
	req.SetAcceptanceRadius( 3 );
	req.SetGoalLocation( destination );

	// 길 찾기를 위한 쿼리 생성
	ai->BuildPathfindingQuery( req, query );
	
	// 길 찾기 결과를 가져오기
	FPathFindingResult r = ns->FindPathSync(query);

	// 목적지까지 길 찾기 성공 여부 확인
	if( r.Result == ENavigationQueryResult::Success )
	{
		// 타겟으로 이동
		ai->MoveToLocation(destination);
	}
	else
	{
		// 랜덤 위치로 이동
		auto result = ai->MoveToLocation(randomPos);

		// 목적지에 도착하면
		if( result == EPathFollowingRequestResult::AlreadyAtGoal )
		{
			// 새로운 랜덤 위치 가져오기
			GetRandomPositionInNavMesh( me->GetActorLocation(), 500.0f, randomPos );
		}
	}
	
	// dir 방향으로 이동하고 싶다.
	//me->AddMovementInput(dir.GetSafeNormal());
	//ai->MoveToLocation(destination);

	// 타겟과 거리를 체크해서 attackRange 안으로 들어오면 공격상태로 전환 하고 싶다.
	// 거리체크
	if(dir.Size() < attackRange)
	{
		// 길 찾기 기능 정지
		ai->StopMovement();

		// 공격 상태로 전환하고 싶다.
		mState = EEnemyState::Attack;

		// 애니메이션 상태 동기화
		Anim->AnimState = mState;

		// 공격 애니메이션 재생 활성화
		Anim->bAttackPlay = true;

		// 공격 상태 전환시 대기 시간 없이 바로 플레이가 되도록
		currentTime = attackDelayTime;
	}
}

void UEnemyFSM::AttackState()
{
	// 일정 시간에 한번씩 공격하고 싶다.
	// 시간이 흐르다가
	currentTime += GetWorld()->DeltaTimeSeconds;

	// 공격시간이 되면
	if( currentTime > attackDelayTime )
	{
		// 공격을 한다.
		PRINT_LOG( TEXT("Attack~!~!~!") );

		// 경과 시간 초기화
		currentTime = 0.0f;
		Anim->bAttackPlay = true;
	}
	else
	{
		// 타겟이 공격범위를 벗어나면 이동상태로 전환하고 싶다.
		// 타겟과의 거리가 필요하다.
		float distance = FVector::Distance(target->GetActorLocation(), me->GetActorLocation());

		// 타겟과의 거리가 공격 범위를 벗어나면
		if( distance > attackRange )
		{
			// 상태를 이동상태로 전환하고 싶다.
			mState = EEnemyState::Move;
		
			// 애니메이션 상태 동기화
			Anim->AnimState = mState;

			// 새로운 랜덤 위치 가져오기
			GetRandomPositionInNavMesh( me->GetActorLocation() , 500.0f , randomPos );
		}
	}
}

void UEnemyFSM::DamageState()
{
	// 시간이 흐르다가
	currentTime += GetWorld()->DeltaTimeSeconds;

	// 경과 시간이 대기 시간을 초과하면
	if( currentTime > damageDelayTime )
	{
		// 대기 상태로 전환 하고 싶다.
		mState = EEnemyState::Idle;

		// 경과 시간 초기화
		currentTime = 0.0f;

		Anim->AnimState = mState;
	}
}

void UEnemyFSM::DieState()
{
	// 애니메이션 재생이 끝나면 아래로 내려가게 하고 싶다.
	if( bDieDone == false )
	{
		return;
	}

	// 계속 아래로 내려가고 싶다.
	// P = P0 + vt
	FVector P0 = me->GetActorLocation();
	FVector vt = FVector::DownVector * dieSpeed * GetWorld()->DeltaTimeSeconds;
	FVector P = P0 + vt;
	me->SetActorLocation( P );

	// 만약 2미터 이상 내려왔다면
	if( P.Z < -200.0f )
	{
		// 제거시킨다.
		me->Destroy();
	}
}

void UEnemyFSM::OnDamageProcess( int32 damage )
{
	// 체력 감소
	hp -= damage;

	// 체력이 남아있는지 체크
	if( hp > 0 )
	{
		// 상태를 피격으로 전환
		mState = EEnemyState::Damage;

		int randValue = FMath::RandRange(0, 1);
		FString sectionName = FString::Printf(TEXT("Damage%d"), randValue);
		me->PlayAnimMontage(Anim->EnemyMontage, 1.0f, FName(*sectionName));
	}
	else
	{
		// 상태를 죽음으로 전환
		mState = EEnemyState::Die;
		// 캡슐 충돌체 비활성화
		me->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		// 죽음 애니메이션 재생
		me->PlayAnimMontage(Anim->EnemyMontage, 1.0f, TEXT("Die"));
	}

	// 길 찾기 기능 정지
	ai->StopMovement();
	Anim->AnimState = mState;
}


void UEnemyFSM::OnAttackEnd()
{
	Anim->bAttackPlay = false;
}


void UEnemyFSM::OnDieEnd2()
{
	bDieDone = true;
}


bool UEnemyFSM::GetRandomPositionInNavMesh( FVector centerLocation , float radius , FVector& dest )
{
	auto ns = UNavigationSystemV1::GetNavigationSystem(GetWorld());
	
	FNavLocation loc;
	bool result = ns->GetRandomReachablePointInRadius(centerLocation, radius, loc );

	dest = loc.Location;
	return result;
}


