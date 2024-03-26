// 版权归暮志未晚所有。

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

/**
 * GameplayTags 标签 单例模式
 * 内部包含原生的项目中使用的标签
 */

struct FMyGameplayTags
{
public:
	static const FMyGameplayTags& Get() { return GameplayTags; }
	static void InitializeNativeGameplayTags();

	FGameplayTag Attributes_Secondary_Armor;
protected:

private:
	static FMyGameplayTags GameplayTags;
 
};
