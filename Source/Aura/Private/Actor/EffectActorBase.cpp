// 版权归暮志未晚所有。


#include "Actor/EffectActorBase.h"

// Sets default values
AEffectActorBase::AEffectActorBase()
{
 	// 设置当前对象是否每帧调用Tick()
	PrimaryActorTick.bCanEverTick = true;

}

// 游戏开始或生成对象时回调
void AEffectActorBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// 每帧的回调
void AEffectActorBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

