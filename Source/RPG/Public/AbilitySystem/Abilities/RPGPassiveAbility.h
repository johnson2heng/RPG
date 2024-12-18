// 版权归暮志未晚所有。

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/RPGGameplayAbility.h"
#include "RPGPassiveAbility.generated.h"

/**
 * 
 */
UCLASS()
class RPG_API URPGPassiveAbility : public URPGGameplayAbility
{
	GENERATED_BODY()

public:
	/**
	 * 覆写激活技能函数
	 * @param Handle 技能实力的句柄
	 * @param ActorInfo 技能拥有者
	 * @param ActivationInfo 激活信息
	 * @param TriggerEventData 游戏事件信息
	 */
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	/**
	 * 接收到技能结束回调函数
	 * @param AbilityTag 结束的技能标识标签
	 */
	void ReceiveDeactivate(const FGameplayTag& AbilityTag);
};
