// 版权归暮志未晚所有。


#include "AbilitySystem/Abilities/RPGGameplayAbility.h"

#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "AbilitySystem/RPGAbilitySystemLibrary.h"
#include "AbilitySystem/RPGAttributeSet.h"

void URPGGameplayAbility::InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputPressed(Handle, ActorInfo, ActivationInfo);
	//通过句柄获取技能实例
	FGameplayAbilitySpec* AbilitySpec = ActorInfo->AbilitySystemComponent->FindAbilitySpecFromHandle(Handle);
	//技能实例在激活状态，触发输入事件
	if(AbilitySpec->IsActive())
	{
		//将按下事件复制到服务器和所有相关的客户端
		ActorInfo->AbilitySystemComponent->InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, Handle, ActivationInfo.GetActivationPredictionKey());
	}
}

void URPGGameplayAbility::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputReleased(Handle, ActorInfo, ActivationInfo);

	//将抬起事件复制到服务器和所有相关的客户端
	ActorInfo->AbilitySystemComponent->InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, Handle, ActivationInfo.GetActivationPredictionKey());
}

FString URPGGameplayAbility::GetDescription(int32 Level)
{
	return FString::Printf(TEXT("%s, <Level>%d</>"), L"默认技能名称 - 请覆写技能的GetDescription来实现技能的描述", Level);
}

FString URPGGameplayAbility::GetNextLevelDescription(int32 Level)
{
	return FString::Printf(TEXT("下一等级：<Level>%d</> 请覆写技能的GetNextLevelDescription来实现技能的描述。"), Level);
}

FString URPGGameplayAbility::GetDescriptionAtLevel(int32 Level, const FString& Title)
{
	return FString::Printf(TEXT("通用的技能描述：<Level>%d</> 在GetDescription和GetNextLevelDescription抽离出的公用的部分实现的函数。"), Level);
}

FString URPGGameplayAbility::GetLockedDescription(int32 Level)
{
	return FString::Printf(TEXT("技能将在角色等级达到<Level>%d</>时解锁"), Level);
}

float URPGGameplayAbility::GetManaCost(const float InLevel) const
{
	float ManaCost = 0.f;
	//获取到冷却GE
	if(const UGameplayEffect* CostEffect = GetCostGameplayEffect())
	{
		//遍历GE修改的内容
		for(FGameplayModifierInfo Mod : CostEffect->Modifiers)
		{
			//判断修改的属性是否为角色蓝量属性
			if(Mod.Attribute == URPGAttributeSet::GetManaAttribute())
			{
				//通过修饰符获取到使用的FScalableFloat并计算传入等级的蓝量消耗，FScalableFloat是受保护性的属性，无法直接获取，只能通过函数
				Mod.ModifierMagnitude.GetStaticMagnitudeIfPossible(InLevel, ManaCost);
				break; //获取到了就结束遍历
			}
		}
	}
	return ManaCost;
}

float URPGGameplayAbility::GetCooldown(const float InLevel) const
{
	float Cooldown = 0.f;
	//获取到技能冷却GE
	if(const UGameplayEffect* CooldownEffect = GetCooldownGameplayEffect())
	{
		//获取到当前冷却时间
		CooldownEffect->DurationMagnitude.GetStaticMagnitudeIfPossible(InLevel, Cooldown);
	}
	return Cooldown;
}
