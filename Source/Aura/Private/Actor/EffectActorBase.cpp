// 版权归暮志未晚所有。


#include "Actor/EffectActorBase.h"

// Sets default values
AEffectActorBase::AEffectActorBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AEffectActorBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AEffectActorBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

