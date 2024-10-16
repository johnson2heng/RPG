// 版权归暮志未晚所有。

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "RPGGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class RPG_API URPGGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:

	virtual void InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	FGameplayTag StartupInputTag;

	virtual FString GetDescription(int32 Level); //获取当前等级的技能描述
	virtual FString GetNextLevelDescription(int32 Level); //获取技能下一等级的技能描述
	static  FString GetLockedDescription(int32 Level); //获取锁定技能描述

protected:

	float GetManaCost(float InLevel = 1.f) const; //获取技能蓝量消耗
	float GetCooldown(float InLevel = 1.f) const; //获取技能冷却时间
};
