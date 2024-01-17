// 版权归暮志未晚所有。

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EffectActorBase.generated.h"

UCLASS()
class AURA_API AEffectActorBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// 构造函数，设置初始值
	AEffectActorBase();

protected:
	// 游戏开始或生成对象时回调
	virtual void BeginPlay() override;

public:	
	// 每帧的回调
	virtual void Tick(float DeltaTime) override;

};
