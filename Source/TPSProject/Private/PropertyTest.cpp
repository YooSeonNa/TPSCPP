// Fill out your copyright notice in the Description page of Project Settings.


#include "PropertyTest.h"

// Sets default values
APropertyTest::APropertyTest()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void APropertyTest::BeginPlay()
{
	Super::BeginPlay();
	
	CallBlueprintFunc_1();
	CallBlueprintFunc_2(4.0f);
}

// Called every frame
void APropertyTest::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

