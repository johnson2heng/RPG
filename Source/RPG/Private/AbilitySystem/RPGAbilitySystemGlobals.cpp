// 版权归暮志未晚所有。


#include "AbilitySystem/RPGAbilitySystemGlobals.h"

#include "RPGAbilityTypes.h"

FGameplayEffectContext* URPGAbilitySystemGlobals::AllocGameplayEffectContext() const
{
	return new FRPGGameplayEffectContext();
}
