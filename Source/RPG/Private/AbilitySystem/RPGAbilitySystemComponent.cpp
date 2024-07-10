// 版权归暮志未晚所有。


#include "AbilitySystem/RPGAbilitySystemComponent.h"

#include "RPGGameplayTags.h"
#include "AbilitySystem/Abilities/RPGGameplayAbility.h"
#include "RPG/RPGLogChannels.h"

void URPGAbilitySystemComponent::AbilityActorInfoSet()
{
	OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &URPGAbilitySystemComponent::ClientEffectApplied);
}

void URPGAbilitySystemComponent::AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities)
{
	for(const TSubclassOf<UGameplayAbility> AbilityClass : StartupAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		if(const URPGGameplayAbility* AbilityBase = Cast<URPGGameplayAbility>(AbilitySpec.Ability))
		{
			AbilitySpec.DynamicAbilityTags.AddTag(AbilityBase->StartupInputTag); //设置技能激活输入标签
			GiveAbility(AbilitySpec); //只应用不激活
			// GiveAbilityAndActivateOnce(AbilitySpec); //应用技能并激活一次
		}
	}

	bStartupAbilitiesGiven = true;
	AbilityGivenDelegate.Broadcast(this);
}

void URPGAbilitySystemComponent::AbilityInputTagHold(const FGameplayTag& InputTag)
{
	if(!InputTag.IsValid()) return;

	for(auto AbilitySpec : GetActivatableAbilities())
	{
		if(AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag))
		{
			AbilitySpecInputPressed(AbilitySpec);
			if(!AbilitySpec.IsActive())
			{
				TryActivateAbility(AbilitySpec.Handle);
			}
		}
	}
}

void URPGAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if(!InputTag.IsValid()) return;

	for(auto AbilitySpec : GetActivatableAbilities())
	{
		if(AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag))
		{
			AbilitySpecInputReleased(AbilitySpec);
		}
	}
}

void URPGAbilitySystemComponent::ForEachAbility(const FForEachAbility& Delegate)
{
	FScopedAbilityListLock ActiveScopeLock(*this); //使用域锁将此作用域this的内容锁定（无法修改），在遍历结束时解锁，保证线程安全
	for(const FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if(!Delegate.ExecuteIfBound(AbilitySpec)) //运行绑定在技能实例上的委托，如果失败返回false
		{
			UE_LOG(LogRPG, Error, TEXT("在函数[%hs]运行委托失败"), __FUNCTION__);
		}
	}
}

FGameplayTag URPGAbilitySystemComponent::GetAbilityTagFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	if(AbilitySpec.Ability)
	{
		for(FGameplayTag Tag : AbilitySpec.Ability.Get()->AbilityTags) //获取设置的所有的技能标签并遍历
		{
			if(Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Abilities")))) //判断当前标签是否包含"Abilities"名称
			{
				return Tag;
			}
		}
	}
	return FGameplayTag();
}

FGameplayTag URPGAbilitySystemComponent::GetInputTagFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	for(FGameplayTag Tag : AbilitySpec.DynamicAbilityTags) //从技能实例的动态标签容器中遍历所有标签
	{
		if(Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("InputTag")))) //查找标签中是否设置以输入标签开头的标签
		{
			return Tag;
		}
	}

	return FGameplayTag();
}

void URPGAbilitySystemComponent::OnRep_ActivateAbilities()
{
	Super::OnRep_ActivateAbilities();

	if(!bStartupAbilitiesGiven)
	{
		bStartupAbilitiesGiven = true;
		AbilityGivenDelegate.Broadcast(this);
	}
}

void URPGAbilitySystemComponent::ClientEffectApplied_Implementation(UAbilitySystemComponent* AbilitySystemComponent,
                                                                    const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveGameplayEffectHandle) const
{
	// GEngine->AddOnScreenDebugMessage(1, 8.f, FColor::Blue, FString("Effect Applied!"));
	FGameplayTagContainer TagContainer;
	EffectSpec.GetAllAssetTags(TagContainer);

	EffectAssetTags.Broadcast(TagContainer);
}
