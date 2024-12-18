// 版权归暮志未晚所有。


#include "AbilitySystem/Abilities/RPGPassiveAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/RPGAbilitySystemComponent.h"

void URPGPassiveAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	//获取到ASC
	if(URPGAbilitySystemComponent* RPGASC = Cast<URPGAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetAvatarActorFromActorInfo())))
	{
		if(!RPGASC->DeactivatePassiveAbility.IsBoundToObject(this))
		{
			//绑定技能取消回调
			RPGASC->DeactivatePassiveAbility.AddUObject(this, &URPGPassiveAbility::ReceiveDeactivate);
		}
	}
}

void URPGPassiveAbility::ReceiveDeactivate(const FGameplayTag& AbilityTag)
{
	//判断技能标签容器里是否包含此标签
	if(AbilityTags.HasTagExact(AbilityTag))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
	}
}
