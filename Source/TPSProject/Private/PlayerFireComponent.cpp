// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerFireComponent.h"
#include "Blueprint/UserWidget.h"
#include "../../../../Plugins/EnhancedInput/Source/EnhancedInput/Public/EnhancedInputComponent.h"
#include "TPSPlayerAnimInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "EnemyFSM.h"
#include "Bullet.h"

// Sets default values for this component's properties
UPlayerFireComponent::UPlayerFireComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// 총알 사운드 가져오기
	ConstructorHelpers::FObjectFinder<USoundBase> TempSound( TEXT( "/Script/Engine.SoundWave'/Game/SniperGun/Rifle.Rifle'" ) );
	if( TempSound.Succeeded() )
	{
		BulletSound = TempSound.Object;
	}
}


// Called when the game starts
void UPlayerFireComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	//me = Cast<ATPSPlayer>( GetOwner() );
	if( me == nullptr )	return;

	tpsCamComp = me->tpsCamComp;
	GunMeshComp = me->GunMeshComp;
	SniperGunMesh = me->SniperGunMesh;

	// 스나이퍼 UI 위젯 인스턴스 생성
	SniperUI = CreateWidget( GetWorld() , SniperUIFactory );

	// 일반 조준 UI 크로스헤어 인스턴스 생성
	_CrossHairUI = CreateWidget( GetWorld() , CrossHairUIFactory );
	
	// 일반 조준 UI 등록
	_CrossHairUI->AddToViewport();

	// 기본으로 스나이퍼건을 사용하도록 설정
	ChangeToSniperGun( FInputActionValue() );
	
}


// Called every frame
void UPlayerFireComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UPlayerFireComponent::SetupInputBinding( class UEnhancedInputComponent* input )
{
	Super::SetupInputBinding(input);

	// 총알 발사 이벤트 처리 함수 바인딩
	input->BindAction( IA_Fire , ETriggerEvent::Started , this , &UPlayerFireComponent::InputFire );

	// 총 교체 이벤트 처리함수 바인딩
	input->BindAction( IA_GrenadeGun , ETriggerEvent::Started , this , &UPlayerFireComponent::ChangeToGrenadeGun );

	input->BindAction( IA_SniperGun , ETriggerEvent::Started , this , &UPlayerFireComponent::ChangeToSniperGun );

	// 스나이퍼 조준모드 이벤트 처리 함수 바인딩
	input->BindAction( IA_Sniper , ETriggerEvent::Started , this , &UPlayerFireComponent::SniperAim );
	input->BindAction( IA_Sniper , ETriggerEvent::Completed , this , &UPlayerFireComponent::SniperAim );
}

void UPlayerFireComponent::InputFire( const FInputActionValue& inputValue )
{
	// 카메라 셰이크 재생
	auto controller = GetWorld()->GetFirstPlayerController();
	controller->PlayerCameraManager->StartCameraShake( CameraShake );

	// 공격 애니메이션 재생
	auto anim = Cast<UTPSPlayerAnimInstance>( me->GetMesh()->GetAnimInstance() );
	anim->PlayAttackAnim();

	// 유탄총 사용 시
	if( bUsingGrenadeGun == true )
	{
		// 총알 발사 처리
		FTransform firePosition = GunMeshComp->GetSocketTransform( TEXT( "FirePosition" ) );

		GetWorld()->SpawnActor<ABullet>( BulletFactory , firePosition );
	}
	// 스나이퍼건 사용 시
	else
	{
		// 발사 사운드
		UGameplayStatics::PlaySound2D( GetWorld() , BulletSound );

		// LineTrace 의 시작위치
		FVector startPos = tpsCamComp->GetComponentLocation();
		
		// LineTrace 의 종료위치
		FVector endPos = tpsCamComp->GetComponentLocation() + tpsCamComp->GetForwardVector() * 5000.0f;

		// LineTrace 의 충돌 정보를 담을 변수
		FHitResult hitInfo;

		// 충돌 옵션 설정 변수
		FCollisionQueryParams params;

		// 자기 자신(플레이어)는 충돌에서 제외
		params.AddIgnoredActor( me );

		// Channel필터를 이용한 LineTrace 충돌 검출
		bool bHit = GetWorld()->LineTraceSingleByChannel( hitInfo , startPos , endPos , ECollisionChannel::ECC_Visibility , params );

		// LineTrace가 충돌했을때
		if( bHit == true )
		{
			// 충돌 처리 -> 충돌효과 표현

			// 총알 파편 효과 트랜스폼
			FTransform BulletTrans;

			// 부딪힌 위치 할당
			BulletTrans.SetLocation( hitInfo.ImpactPoint );

			// 총알 파편 효과 인스턴스 생성
			UGameplayStatics::SpawnEmitterAtLocation( GetWorld() , BulletEffectFactory , BulletTrans );

			auto hitComp = hitInfo.GetComponent();

			// 만약에 컴포넌트에 물리가 적용되어 있다면
			if( hitComp && hitComp->IsSimulatingPhysics() )
			{
				// 조준한 방향이 필요
				FVector dir = ( endPos - startPos ).GetSafeNormal();

				// 날려 버릴 힘 ( F = ma )
				FVector force = dir * hitComp->GetMass() * 500000;

				// 그 방향으로 날려 버리고 싶다.
				hitComp->AddForceAtLocation( force , hitInfo.ImpactPoint );
			}

			// 부딪힌 대상이 적인지 체크
			auto enemy = hitInfo.GetActor()->GetDefaultSubobjectByName( TEXT( "FSM" ) );
			if( enemy )
			{
				auto enemyFSM = Cast<UEnemyFSM>( enemy );
				enemyFSM->OnDamageProcess( 1 );
			}
		}
	}
}


void UPlayerFireComponent::ChangeToGrenadeGun( const FInputActionValue& inputValue )
{
	// 유탄총 사용 중으로 체크
	bUsingGrenadeGun = true;
	SniperGunMesh->SetVisibility( false );
	GunMeshComp->SetVisibility( true );
}

void UPlayerFireComponent::ChangeToSniperGun( const FInputActionValue& inputValue )
{
	bUsingGrenadeGun = false;
	SniperGunMesh->SetVisibility( true );
	GunMeshComp->SetVisibility( false );
}

void UPlayerFireComponent::SniperAim( const FInputActionValue& inputValue )
{
	// 스나이퍼건 모드가 아니라면 처리하지 않는다.
	if( bUsingGrenadeGun )
	{
		return;
	}

	// Pressed 입력 처리
	if( bSniperAim == false )
	{
		// 스나이퍼 조준 모드 활성화
		bSniperAim = true;
		// 스나이퍼 조준 UI 등록
		SniperUI->AddToViewport();
		// 카메라의 시야각 FOV(Field Of View) 설정
		tpsCamComp->SetFieldOfView( 45.0f );
		// 일반 조준 UI 제거
		_CrossHairUI->RemoveFromParent();
	}
	// Released 입력 처리
	else
	{
		// 스나이퍼 조준 모드 비활성화
		bSniperAim = false;
		// 스나이퍼 조준 UI 화면에서 제거
		SniperUI->RemoveFromParent();
		// 카메라 시야각을 원래대로 복원
		tpsCamComp->SetFieldOfView( 90.0f );
		// 일반 조준 UI 등록
		_CrossHairUI->AddToViewport();
	}
}


