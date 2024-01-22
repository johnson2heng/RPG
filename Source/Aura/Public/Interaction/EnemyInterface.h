// 版权归暮志未晚所有。

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EnemyInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UEnemyInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 敌人的接口，所有敌人都包含此接口
 */
class AURA_API IEnemyInterface
{
	GENERATED_BODY()

	// 向此类添加接口函数（虚函数），继承此接口的类都将需要继承并实现这些函数。
public:
	virtual void HighlightActor() = 0; //高亮
	virtual void UnHighlightActor() = 0; //取消高亮
};
