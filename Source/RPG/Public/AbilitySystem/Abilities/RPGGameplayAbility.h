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
	
	/**
	 * 获取当前等级技能描述
	 * @param Level 技能等级
	 * @return 技能描述富文本
	 */
	virtual FString GetDescription(int32 Level);

	/**
	 * 获取下一等级技能描述
	 * @param Level 下一等级
	 * @return 技能描述附文本
	 */
	virtual FString GetNextLevelDescription(int32 Level);

	/**
	 * 当前技能的技能描述，将共用部分抽离出来为单个函数
	 * @param Level 显示的技能等级
	 * @param Title 技能标题
	 * @return 技能描述文本
	 */
	virtual FString GetDescriptionAtLevel(int32 Level, const FString& Title);
	static  FString GetLockedDescription(int32 Level); //获取锁定技能描述

protected:

	float GetManaCost(float InLevel = 1.f) const; //获取技能蓝量消耗
	float GetCooldown(float InLevel = 1.f) const; //获取技能冷却时间
};
