// 版权归暮志未晚所有。

#pragma once

#include "CoreMinimal.h"
#include "Character/CharacterBase.h"
#include "Interaction/EnemyInterface.h"
#include "EnemyBase.generated.h"

/**
 * 敌人角色的基类
 */
UCLASS()
class AURA_API AEnemyBase : public ACharacterBase, public IEnemyInterface
{
	GENERATED_BODY()
	
public:
	AEnemyBase();

	/* 敌人接口 */
	virtual void HighlightActor() override; //高亮
	virtual void UnHighlightActor() override; //取消高亮
	/* 敌人接口 结束 */

protected:
	virtual void BeginPlay() override;
};
