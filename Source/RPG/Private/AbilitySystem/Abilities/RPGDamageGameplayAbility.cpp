// 版权归暮志未晚所有。


#include "AbilitySystem/Abilities/RPGDamageGameplayAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

void URPGDamageGameplayAbility::CauseDamage(AActor* TargetActor)
{
	//创建GE
	FGameplayEffectSpecHandle DamageSpecHandle = MakeOutgoingGameplayEffectSpec(DamageEffectClass, 1.f);
	//通过SetByCaller设置属性伤害
	for(auto Pair : DamageTypes)
	{
		const float ScaleDamage = Pair.Value.GetValueAtLevel(GetAbilityLevel());
		UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(DamageSpecHandle, Pair.Key, ScaleDamage);
	}
	//将GE应用给目标
	GetAbilitySystemComponentFromActorInfo()->ApplyGameplayEffectSpecToTarget(
		*DamageSpecHandle.Data.Get(),
		UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor));
}

FTaggedMontage URPGDamageGameplayAbility::GetRandomTaggedMontageFromArray(const TArray<FTaggedMontage>& TaggedMontages)
{
	if(TaggedMontages.Num() > 0)
	{
		const int32 RandomNum = FMath::RandRange(0, TaggedMontages.Num() - 1);
		return TaggedMontages[RandomNum];
	}
	return FTaggedMontage();
}

float URPGDamageGameplayAbility::GetDamageByDamageType(const float InLevel, const FGameplayTag& DamageType)
{
	checkf(DamageTypes.Contains(DamageType), TEXT("技能 [%s] 没有包含 [%s] 类型的伤害"), *GetNameSafe(this), *DamageType.ToString());
	return DamageTypes[DamageType].GetValueAtLevel(InLevel); //根据等级获取技能伤害
}
