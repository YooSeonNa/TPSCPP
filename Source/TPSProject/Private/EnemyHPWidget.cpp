// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyHPWidget.h"

#include "Components/ProgressBar.h"

void UEnemyHPWidget::NativeTick( const FGeometry& MyGeometry , float InDeltaTime )
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}

void UEnemyHPWidget::SetHPBar( float HP , float MaxHP )
{
	if( HPBar )
	{
		if( HP > MaxHP )
		{
			HP = MaxHP;
		}
		else if( HP < 0.0f )
		{
			HP = 0.0f;
		}
		
		HPBar->SetPercent( HP / MaxHP );
	}
}
