// Fill out your copyright notice in the Description page of Project Settings.


#include "TPSPlayer.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "../../../../Plugins/EnhancedInput/Source/EnhancedInput/Public/EnhancedInputComponent.h"
#include "../../../../Plugins/EnhancedInput/Source/EnhancedInput/Public/EnhancedInputSubsystems.h"
#include "Bullet.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "EnemyFSM.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TPSPlayerAnimInstance.h"
#include "PlayerMoveComponent.h"
#include "PlayerFireComponent.h"

// Sets default values
ATPSPlayer::ATPSPlayer()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Mesh에 퀸을 로드해서 넣고싶다.
	ConstructorHelpers::FObjectFinder<USkeletalMesh> TempMesh(TEXT("/Script/Engine.SkeletalMesh'/Game/Characters/Mannequins/Meshes/SKM_Quinn.SKM_Quinn'"));

	// 만약 파일읽기가 성공했다면
	if( TempMesh.Succeeded() )
	{
		// 로드한 메시를 넣어주고 싶다.
		GetMesh()->SetSkeletalMesh(TempMesh.Object);

		// 위치값과 회전값을 반영해주고 싶다.
		GetMesh()->SetRelativeLocationAndRotation(FVector( 0.0f, 0.0f, -90.0f ), FRotator( 0.0f, -90.0f, 0.0f ) );
	}

	// TPS카메라를 붙이고 싶다.
	// SpringArm 컴포넌트 붙이기
	springArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	springArmComp->SetupAttachment( RootComponent );
	springArmComp->SetRelativeLocation( FVector( 0.0f, 60.0f, 80.0f ) );
	springArmComp->TargetArmLength = 300.0f;
	springArmComp->bUsePawnControlRotation = true;

	// Camera 컴포넌트 붙이기
	tpsCamComp = CreateDefaultSubobject<UCameraComponent>(TEXT("TpsCamComp"));
	tpsCamComp->SetupAttachment( springArmComp );
	tpsCamComp->bUsePawnControlRotation = false;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;


	// 총 스켈레탈 메시 컴포넌트 등록
	GunMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>( TEXT("GunMeshComp") );

	// 부모 컴포넌트를 Mesh 컴포넌트로 설정
	GunMeshComp->SetupAttachment( GetMesh() );

	// 스켈레탈 메시 데이터 로드
	ConstructorHelpers::FObjectFinder<USkeletalMesh> TempGunMesh( TEXT("/Script/Engine.SkeletalMesh'/Game/FPWeapon/Mesh/SK_FPGun.SK_FPGun'") );

	// 데이터 로드가 성공했다면
	if( TempGunMesh.Succeeded() )
	{
		// 스켈레탈 메시 데이터 할당
		GunMeshComp->SetSkeletalMesh( TempGunMesh.Object );

		// 총 위치 설정
		GunMeshComp->SetRelativeLocation( FVector( 0.0f, 60.0f, 120.0f ) );
	}

	// Mesh의 AnimInstace를 파일로 로드해서 적용하고 싶다.
	ConstructorHelpers::FClassFinder<UTPSPlayerAnimInstance> TempAnimInst(TEXT("/Script/Engine.AnimBlueprint'/Game/NYS/Blueprints/Anim/ABP_TPSPlayer.ABP_TPSPlayer_C'"));

	// 만약 파일읽기를 성공했다면
	if( TempAnimInst.Succeeded() )
	{
		GetMesh()->SetAnimInstanceClass(TempAnimInst.Class);
	}

	// 스나이퍼건 컴포넌트 등록
	SniperGunMesh = CreateDefaultSubobject<UStaticMeshComponent>( TEXT("SniperGunMesh") );

	// 부모 컴포넌트를 Mesh 컴포넌트로 설정
	SniperGunMesh->SetupAttachment( GetMesh(), TEXT("hand_rSocket") );

	// 스태틱 메시 데이터 로드
	ConstructorHelpers::FObjectFinder<UStaticMesh> TempSniperMesh( TEXT("/Script/Engine.StaticMesh'/Game/SniperGun/sniper11.sniper11'") );

	// 데이터 로드가 성공했다면
	if( TempSniperMesh.Succeeded() )
	{
		// 스태틱 메시 데이터 할당
		SniperGunMesh->SetStaticMesh(TempSniperMesh.Object);

		// 총 위치 설정
		SniperGunMesh->SetRelativeLocationAndRotation( FVector( -37.0f, -5.6f, 3.0f ), FRotator( 0.0f, 110.0f, 0.0f ) );

		// 총 크기 설정
		SniperGunMesh->SetRelativeScale3D( FVector( 0.15f ) );
	}	

	// 쭈그리기를 활성화 하고 싶다.
	GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;

	MoveComp = CreateDefaultSubobject<UPlayerMoveComponent>( TEXT("MoveComp") );
	FireComp = CreateDefaultSubobject<UPlayerFireComponent>( TEXT("FireComp") );
}

// Called when the game starts or when spawned
void ATPSPlayer::BeginPlay()
{
	Super::BeginPlay();

	//MoveComp->me = this;
	Anim = Cast<UTPSPlayerAnimInstance>(GetMesh()->GetAnimInstance());

	auto pc = Cast<APlayerController>(Controller);
	if( pc )
	{
		auto subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(pc->GetLocalPlayer());

		if( subsystem )
		{
			subsystem->AddMappingContext(IMC_TPS, 0);
		}
	}	

	//JumpMaxCount = 3;
}

// Called every frame
void ATPSPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ATPSPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	auto PlayerInput = Cast<UEnhancedInputComponent>( PlayerInputComponent );
	if( PlayerInput )
	{
		// 컴포넌트에 입력 바인딩 처리를 하도록 호출
		//MoveComp->SetupInputBinding(PlayerInput);
		//FireComp->SetupInputBinding( PlayerInput );				

		// 일반 델리게이트
		//TestDelegate.ExecuteIfBound();

		// 멀티캐스트 델리게이트를 호출해준다.
		InputBindingDelegate.Broadcast( PlayerInput );
	}
}

