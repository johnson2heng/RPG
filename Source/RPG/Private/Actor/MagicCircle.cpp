// 版权归暮志未晚所有。


#include "Actor/MagicCircle.h"

#include "Components/DecalComponent.h"

AMagicCircle::AMagicCircle()
{
	PrimaryActorTick.bCanEverTick = true;

	MagicCircleDecal = CreateDefaultSubobject<UDecalComponent>("MagicCircleDecal");
	SetRootComponent(MagicCircleDecal);
}

void AMagicCircle::BeginPlay()
{
	Super::BeginPlay();
	
}

void AMagicCircle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

