// 版权归暮志未晚所有。

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/ProjectileSpell.h"
#include "RPGFireBolt.generated.h"

/**
 * 
 */
UCLASS()
class RPG_API URPGFireBolt : public UProjectileSpell
{
	GENERATED_BODY()

public:
	// FString GetDescriptionAtLevel(int32 INT32, const char* Str);
	virtual FString GetDescription(int32 Level) override; //获取投射技能描述
	virtual FString GetNextLevelDescription(int32 Level) override; //获取投射技能下一等级描述

	FString GetDescriptionAtLevel(int32 Level, const FString& Title); //获取对应等级的技能描述
};
