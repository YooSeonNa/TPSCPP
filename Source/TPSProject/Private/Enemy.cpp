// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"
#include "EnemyFSM.h"
#include "Components/WidgetComponent.h"
#include "EnemyHPWidget.h"

// Sets default values
AEnemy::AEnemy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// 스켈레탈 메시 데이터 로드
	ConstructorHelpers::FObjectFinder<USkeletalMesh> TempMesh( TEXT( "/Script/Engine.SkeletalMesh'/Game/Enemy/Model/vampire_a_lusth.vampire_a_lusth'" ) );

	// 만약 파일읽기가 성공했다면
	if( TempMesh.Succeeded() )
	{
		// 로드한 메시를 넣어주고 싶다.
		GetMesh()->SetSkeletalMesh( TempMesh.Object );

		// 위치값과 회전값을 반영해주고 싶다.
		GetMesh()->SetRelativeLocationAndRotation( FVector( 0.0f , 0.0f , -90.0f ) , FRotator( 0.0f , -90.0f , 0.0f ) );

		// 메시 크기 수정
		GetMesh()->SetRelativeScale3D( FVector( 0.8f ) );
	}	

	// 애니메이션 블루프린트 할당하기
	ConstructorHelpers::FClassFinder<UAnimInstance> TempAnim( TEXT("/Script/Engine.AnimBlueprint'/Game/NYS/Blueprints/Anim/ABP_Enemy.ABP_Enemy_C'") );

	if( TempAnim.Succeeded() )
	{
		GetMesh()->SetAnimInstanceClass(TempAnim.Class);
	}


	// UWidgetComponent
	HPComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("HPComp"));
	HPComp->SetupAttachment(RootComponent);

	ConstructorHelpers::FClassFinder<UEnemyHPWidget> TempWidget(TEXT("/Script/UMGEditor.WidgetBlueprint'/Game/NYS/Blueprints/WBP_EnemyHP.WBP_EnemyHP_C'"));

	if( TempWidget.Succeeded() )
	{
		HPComp->SetWidgetClass( TempWidget.Class );
		HPComp->SetDrawSize( FVector2D(100.0f, 20.0f));
		HPComp->SetRelativeLocation(FVector( 0.0f, 0.0f, 120.0f ));
	}

	// EnemyFSM 컴포넌트 추가
	fsm = CreateDefaultSubobject<UEnemyFSM>( TEXT( "FSM" ) );

	// 월드에 배치되거나 스폰될 때 자동으로
	// AIController 로부터 Possess 될 수 있도록 설정
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector Target = GetWorld()->GetFirstPlayerController()->PlayerCameraManager->GetCameraLocation();

	FVector Dir = Target - HPComp->GetComponentLocation();

	FRotator Rot = Dir.ToOrientationRotator();

	HPComp->SetWorldRotation( Rot );
}

// Called to bind functionality to input
void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

