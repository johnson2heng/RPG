// 版权归暮志未晚所有。


#include "AbilitySystem/Abilities/RPGDamageGameplayAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "RPGAbilityTypes.h"
#include "AbilitySystem/RPGAbilitySystemLibrary.h"

void URPGDamageGameplayAbility::CauseDamage(AActor* TargetActor)
{
	//生成配置
	FDamageEffectParams Params = MakeDamageEffectParamsFromClassDefaults(TargetActor);

	//设置死亡冲击和击退
	// if(IsValid(TargetActor))
	// {
	// 	//获取到攻击对象和目标的朝向，并转换成角度
	// 	FRotator Rotation = (TargetActor->GetActorLocation() - GetAvatarActorFromActorInfo()->GetActorLocation()).Rotation();
	// 	Rotation.Pitch = 45.f; //设置击退角度垂直45度
	// 	const FVector ToTarget = Rotation.Vector();
	// 	Params.DeathImpulse = ToTarget * DeathImpulseMagnitude;
	// 	//判断攻击是否触发击退
	// 	if(FMath::RandRange(1, 100) < Params.KnockbackChance)
	// 	{
	// 		Params.KnockbackForce = ToTarget * KnockbackForceMagnitude;
	// 	}
	// }
		
	//通过配置项应用给目标ASC
	URPGAbilitySystemLibrary::ApplyDamageEffect(Params);
}


FDamageEffectParams URPGDamageGameplayAbility::MakeDamageEffectParamsFromClassDefaults(AActor* TargetActor, FVector InRadialDamageOrigin, bool bOverrideKnockbackDirection, FVector KnockbackDirectionOverride, bool bOverrideDeathImpulse, FVector DeathImpulseDirectionOverride)
{
	FDamageEffectParams Params;
	Params.WorldContextObject = GetAvatarActorFromActorInfo();
	Params.DamageGameplayEffectClass = DamageEffectClass;
	Params.SourceAbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	Params.TargetAbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	for(auto& Pair : DamageTypes)
	{
		const float ScaledDamage = Pair.Value.GetValueAtLevel(GetAbilityLevel()); //根据等级获取技能伤害
		Params.DamageTypes.Add(Pair.Key, ScaledDamage);
	}
	Params.AbilityLevel = GetAbilityLevel();
	
	//负面效果相关
	Params.DeBuffDamageType = DeBuffDamageType;
	Params.DeBuffChance = DeBuffChance;
	Params.DeBuffDamage = DeBuffDamage;
	Params.DeBuffDuration = DeBuffDuration;
	Params.DeBuffFrequency = DeBuffFrequency;
	Params.DeathImpulseMagnitude = DeathImpulseMagnitude;
	
	//击退相关
	Params.KnockbackForceMagnitude = KnockbackForceMagnitude;
	Params.KnockbackChance = KnockbackChance;
	if(IsValid(TargetActor))
	{
		//攻击击退处理
		FVector ToTarget;
		//如果设置了伤害中心，则使用中心的设置，否则采用攻击造成的
		if(bOverrideKnockbackDirection)
		{
			KnockbackDirectionOverride.Normalize();
			ToTarget = KnockbackDirectionOverride;
		}
		else
		{
			//获取到攻击对象和目标的朝向，并转换成角度
			FRotator Rotation = (TargetActor->GetActorLocation() - GetAvatarActorFromActorInfo()->GetActorLocation()).Rotation();
			Rotation.Pitch = 45.f; //设置击退角度垂直45度
			ToTarget = Rotation.Vector();
		}
		//判断攻击是否触发击退
		if(FMath::RandRange(1, 100) < Params.KnockbackChance)
		{
			Params.KnockbackForce = ToTarget * KnockbackForceMagnitude;
		}

		//死亡时击退处理
		if(bOverrideDeathImpulse)
		{
			DeathImpulseDirectionOverride.Normalize();
			Params.DeathImpulse = DeathImpulseDirectionOverride * DeathImpulseMagnitude;
		}
		else
		{
			Params.DeathImpulse = ToTarget * DeathImpulseMagnitude;
		}
	}
	
	//如果是范围伤害，将设置对应属性
	if(bIsRadialDamage)
	{
		Params.bIsRadialDamage = bIsRadialDamage;
		Params.RadialDamageOrigin = InRadialDamageOrigin.IsZero() ? RadialDamageOrigin : InRadialDamageOrigin;
		Params.RadialDamageInnerRadius = RadialDamageInnerRadius;
		Params.RadialDamageOuterRadius = RadialDamageOuterRadius;
	}
	return Params;
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
