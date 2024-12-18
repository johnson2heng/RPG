// 版权归暮志未晚所有。


#include "AbilitySystem/RPGAbilitySystemComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "RPGGameplayTags.h"
#include "AbilitySystem/RPGAbilitySystemLibrary.h"
#include "AbilitySystem/Abilities/RPGGameplayAbility.h"
#include "AbilitySystem/Data/AbilityInfo.h"
#include "Game/LoadScreenSaveGame.h"
#include "Interaction/PlayerInterface.h"
#include "RPG/RPGLogChannels.h"

void URPGAbilitySystemComponent::AbilityActorInfoSet()
{
	OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &URPGAbilitySystemComponent::ClientEffectApplied);
}

void URPGAbilitySystemComponent::AddCharacterAbilitiesFormSaveData(ULoadScreenSaveGame* SaveGameData)
{
	for(const FSavedAbility& Data : SaveGameData->SavedAbilities)
	{
		const TSubclassOf<UGameplayAbility> LoadedAbilityClass = Data.GameplayAbility;

		FGameplayAbilitySpec LoadedAbilitySpec = FGameplayAbilitySpec(LoadedAbilityClass, Data.AbilityLevel);
		LoadedAbilitySpec.DynamicAbilityTags.AddTag(Data.AbilityInputTag); //设置技能激活输入标签
		LoadedAbilitySpec.DynamicAbilityTags.AddTag(Data.AbilityStatus); //设置技能状态标签

		//主动技能的处理
		if(Data.AbilityType == FRPGGameplayTags::Get().Abilities_Type_Offensive)
		{
			GiveAbility(LoadedAbilitySpec); //只应用不激活
		}
		//被动技能的处理
		else if(Data.AbilityType == FRPGGameplayTags::Get().Abilities_Type_Passive)
		{
			//确保技能已经装配
			if(Data.AbilityStatus.MatchesTagExact(FRPGGameplayTags::Get().Abilities_Status_Equipped))
			{
				GiveAbilityAndActivateOnce(LoadedAbilitySpec); //应用技能并激活
			}
			else
			{
				GiveAbility(LoadedAbilitySpec); //只应用不激活
			}
		}
	}

	bStartupAbilitiesGiven = true;
	AbilityGivenDelegate.Broadcast();
}

void URPGAbilitySystemComponent::AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities)
{
	for(const TSubclassOf<UGameplayAbility> AbilityClass : StartupAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		if(const URPGGameplayAbility* AbilityBase = Cast<URPGGameplayAbility>(AbilitySpec.Ability))
		{
			AbilitySpec.DynamicAbilityTags.AddTag(AbilityBase->StartupInputTag); //设置技能激活输入标签
			AbilitySpec.DynamicAbilityTags.AddTag(FRPGGameplayTags::Get().Abilities_Status_Equipped); //设置技能激活输入标签
			GiveAbility(AbilitySpec); //只应用不激活
			// GiveAbilityAndActivateOnce(AbilitySpec); //应用技能并激活一次
		}
	}

	bStartupAbilitiesGiven = true;
	AbilityGivenDelegate.Broadcast();
}

void URPGAbilitySystemComponent::AddCharacterPassiveAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupPassiveAbilities)
{
	for(const TSubclassOf<UGameplayAbility> AbilityClass : StartupPassiveAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		AbilitySpec.DynamicAbilityTags.AddTag(FRPGGameplayTags::Get().Abilities_Status_Equipped); //设置技能状态已装配
		GiveAbilityAndActivateOnce(AbilitySpec); //应用技能并激活一次
	}
}

void URPGAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
	if(!InputTag.IsValid()) return;

	//域锁，当ASC的技能被修改时，推荐使用域锁，保证数据结果一致
	FScopedAbilityListLock ActiveScopeLock(*this);
	for(auto AbilitySpec : GetActivatableAbilities())
	{
		if(AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag))
		{
			AbilitySpecInputPressed(AbilitySpec);

			// //如果技能实例已经被激活，按下将会触发按下事件
			// if(AbilitySpec.IsActive())
			// {
			// 	//将按下事件复制到服务器和所有相关的客户端
			// 	InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, AbilitySpec.Handle, AbilitySpec.ActivationInfo.GetActivationPredictionKey());
			// }
		}
	}
}

void URPGAbilitySystemComponent::AbilityInputTagHold(const FGameplayTag& InputTag)
{
	if(!InputTag.IsValid()) return;

	//域锁，当ASC的技能被修改时，推荐使用域锁，保证数据结果一致
	FScopedAbilityListLock ActiveScopeLock(*this);
	for(auto AbilitySpec : GetActivatableAbilities())
	{
		if(AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag))
		{
			//AbilitySpecInputPressed(AbilitySpec);
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

	//域锁，当ASC的技能被修改时，推荐使用域锁，保证数据结果一致
	FScopedAbilityListLock ActiveScopeLock(*this);
	for(auto AbilitySpec : GetActivatableAbilities())
	{
		if(AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag) && AbilitySpec.IsActive())
		{
			AbilitySpecInputReleased(AbilitySpec);
			//将抬起事件复制到服务器和所有相关的客户端
			// InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, AbilitySpec.Handle, AbilitySpec.ActivationInfo.GetActivationPredictionKey());
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

FGameplayTag URPGAbilitySystemComponent::GetStatusTagFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	for(FGameplayTag Tag : AbilitySpec.DynamicAbilityTags) //从技能实例的动态标签容器中遍历所有标签
	{
		if(Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Abilities.Status")))) //查找标签中是否设置以输入标签开头的标签
		{
			return Tag;
		}
	}

	return FGameplayTag();
}

FGameplayAbilitySpec* URPGAbilitySystemComponent::GetSpecFromAbilityTag(const FGameplayTag& AbilityTag)
{
	FScopedAbilityListLock ActiveScopeLoc(*this); //域锁
	//遍历已经应用的技能
	for(FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		for(FGameplayTag Tag : AbilitySpec.Ability.Get()->AbilityTags)
		{
			if(Tag.MatchesTag(AbilityTag))
			{
				return &AbilitySpec;
			}
		}
	}
	return nullptr;
}

FGameplayTag URPGAbilitySystemComponent::GetStatusTagFromAbilityTag(const FGameplayTag& AbilityTag)
{
	if(const FGameplayAbilitySpec* Spec = GetSpecFromAbilityTag(AbilityTag))
	{
		return GetStatusTagFromSpec(*Spec);
	}
	return FGameplayTag();
}

FGameplayTag URPGAbilitySystemComponent::GetInputTagFromAbilityTag(const FGameplayTag& AbilityTag)
{
	if(const FGameplayAbilitySpec* Spec = GetSpecFromAbilityTag(AbilityTag))
	{
		return GetInputTagFromSpec(*Spec);
	}
	return FGameplayTag();
}

void URPGAbilitySystemComponent::ToggleAbilityStatus(FGameplayAbilitySpec& AbilitySpec, const FGameplayTag& StatusTag)
{
	const FRPGGameplayTags GameplayTags = FRPGGameplayTags::Get();
	
	AbilitySpec.DynamicAbilityTags.RemoveTag(GameplayTags.Abilities_Status_Eligible);
	AbilitySpec.DynamicAbilityTags.RemoveTag(GameplayTags.Abilities_Status_Equipped);
	AbilitySpec.DynamicAbilityTags.RemoveTag(GameplayTags.Abilities_Status_Locked);
	AbilitySpec.DynamicAbilityTags.RemoveTag(GameplayTags.Abilities_Status_Unlocked);
	
	AbilitySpec.DynamicAbilityTags.AddTag(StatusTag);
}

bool URPGAbilitySystemComponent::SlotIsEmpty(const FGameplayTag& Slot)
{
	FScopedAbilityListLock ActiveScopeLoc(*this);
	for(FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if(AbilityHasSlot(AbilitySpec, Slot))
		{
			return false;
		}
	}
	return true;
}

bool URPGAbilitySystemComponent::AbilityHasSlot(const FGameplayAbilitySpec& Spec, const FGameplayTag& Slot)
{
	return Spec.DynamicAbilityTags.HasTagExact(Slot);
}

bool URPGAbilitySystemComponent::AbilityHasAnySlot(const FGameplayAbilitySpec& Spec)
{
	//通过判断动态标签是否含有Input的标签来判断技能是否装配到槽位
	return Spec.DynamicAbilityTags.HasTag(FGameplayTag::RequestGameplayTag(FName("InputTag")));
}

FGameplayAbilitySpec* URPGAbilitySystemComponent::GetSpecWithSlot(const FGameplayTag& Slot)
{
	FScopedAbilityListLock ActiveScopeLoc(*this);
	for(FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if(AbilityHasSlot(AbilitySpec, Slot))
		{
			return &AbilitySpec;
		}
	}
	return nullptr;
}

bool URPGAbilitySystemComponent::IsPassiveAbility(const FGameplayAbilitySpec& Spec) const
{
	//从技能配置数据里获取到技能对于的配置信息
	UAbilityInfo* AbilityInfo = URPGAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor());
	const FGameplayTag AbilityTag = GetAbilityTagFromSpec(Spec);
	const FRPGAbilityInfo& Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
	//判断信息里配置的技能类型是否为被动技能
	const FGameplayTag AbilityType = Info.AbilityType;
	return AbilityType.MatchesTagExact(FRPGGameplayTags::Get().Abilities_Type_Passive);
}

void URPGAbilitySystemComponent::AssignSlotToAbility(FGameplayAbilitySpec& Spec, const FGameplayTag& Slot)
{
	const FRPGGameplayTags GameplayTags = FRPGGameplayTags::Get();
	ClearSlot(&Spec);
	Spec.DynamicAbilityTags.AddTag(Slot);
	ToggleAbilityStatus(Spec, GameplayTags.Abilities_Status_Equipped);
}

void URPGAbilitySystemComponent::MulticastActivatePassiveEffect_Implementation(const FGameplayTag& AbilityTag, bool bActivate)
{
	ActivatePassiveEffect.Broadcast(AbilityTag, bActivate);
}

void URPGAbilitySystemComponent::UpgradeAttribute(const FGameplayTag& AttributeTag)
{
	//判断Avatar是否基础角色接口
	if(GetAvatarActor()->Implements<UPlayerInterface>())
	{
		//判断是否用于可分配点数
		if(IPlayerInterface::Execute_GetAttributePoints(GetAvatarActor()) > 0)
		{
			ServerUpgradeAttribute(AttributeTag); //调用服务器升级属性
		}
	}
}

void URPGAbilitySystemComponent::ServerUpgradeAttribute_Implementation(const FGameplayTag& AttributeTag)
{
	FGameplayEventData Payload; //创建一个事件数据
	Payload.EventTag = AttributeTag;
	Payload.EventMagnitude = 1.f;

	//向自身发送事件，通过被动技能接收属性加点
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetAvatarActor(), AttributeTag, Payload);

	//判断Avatar是否基础角色接口
	if(GetAvatarActor()->Implements<UPlayerInterface>())
	{
		IPlayerInterface::Execute_AddToAttributePoints(GetAvatarActor(), -1); //减少一点可分配属性点
	}
}

void URPGAbilitySystemComponent::UpdateAbilityStatuses(const int32 Level)
{
	//从GameMode获取到技能配置数据
	UAbilityInfo* AbilityInfo = URPGAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor());
	for(const FRPGAbilityInfo& Info : AbilityInfo->AbilityInformation)
	{
		if(!Info.AbilityTag.IsValid()) continue; //如果没有技能标签，取消执行
		if(Level < Info.LevelRequirement) continue; //如果角色等级未达到所需等级，取消执行
		//判断ASC中是否已存在当前技能实例
		if(GetSpecFromAbilityTag(Info.AbilityTag) == nullptr)
		{
			//如果没有技能实例，将应用一个新的技能实例
			FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(Info.Ability, 0);
			AbilitySpec.DynamicAbilityTags.AddTag(FRPGGameplayTags::Get().Abilities_Status_Eligible);
			GiveAbility(AbilitySpec);
			MarkAbilitySpecDirty(AbilitySpec); //设置当前技能立即复制到每个客户端
			ClientUpdateAbilityStatus(Info.AbilityTag, FRPGGameplayTags::Get().Abilities_Status_Eligible, 0); //广播
		}
	}
}

void URPGAbilitySystemComponent::ServerSpendSpellPoint_Implementation(const FGameplayTag& AbilityTag)
{
	//获取到技能实例
	if( FGameplayAbilitySpec* AbilitySpec = GetSpecFromAbilityTag(AbilityTag))
	{
		//减少一个可分配的技能点
		if(GetAvatarActor()->Implements<UPlayerInterface>())
		{
			IPlayerInterface::Execute_AddToSpellPoints(GetAvatarActor(), -1);
		}
		//获取状态标签
		FRPGGameplayTags GameplayTags = FRPGGameplayTags::Get();
		FGameplayTag StatusTag = GetStatusTagFromSpec(*AbilitySpec);
		//根据状态标签处理
		if(StatusTag.MatchesTagExact(GameplayTags.Abilities_Status_Eligible))
		{
			//技能升级，如果是可解锁状态，将状态标签从可解锁，切换为已解锁
			StatusTag = GameplayTags.Abilities_Status_Unlocked;
			ToggleAbilityStatus(*AbilitySpec, StatusTag);
			//提升技能等级
			AbilitySpec->Level += 1;
		}
		else if(StatusTag.MatchesTagExact(GameplayTags.Abilities_Status_Equipped) || StatusTag.MatchesTagExact(GameplayTags.Abilities_Status_Unlocked))
		{
			AbilitySpec->Level += 1;
		}
		ClientUpdateAbilityStatus(AbilityTag, StatusTag, AbilitySpec->Level); //广播技能状态修改
		MarkAbilitySpecDirty(*AbilitySpec); //设置当前技能立即复制到每个客户端
	}
}

void URPGAbilitySystemComponent::ServerDemotionSpellPoint_Implementation(const FGameplayTag& AbilityTag)
{
	//获取到技能实例
	if( FGameplayAbilitySpec* AbilitySpec = GetSpecFromAbilityTag(AbilityTag))
	{
		//增加一个可分配的技能点
		if(GetAvatarActor()->Implements<UPlayerInterface>())
		{
			IPlayerInterface::Execute_AddToSpellPoints(GetAvatarActor(), 1);
		}
		//获取状态标签
		FRPGGameplayTags GameplayTags = FRPGGameplayTags::Get();
		FGameplayTag StatusTag = GetStatusTagFromSpec(*AbilitySpec);
		
		if(StatusTag.MatchesTagExact(GameplayTags.Abilities_Status_Equipped) || StatusTag.MatchesTagExact(GameplayTags.Abilities_Status_Unlocked))
		{
			AbilitySpec->Level -= 1;
			if(AbilitySpec->Level < 1)
			{
				//技能小于1级，当前技能将无法装配，直接设置为可解锁状态
				StatusTag = GameplayTags.Abilities_Status_Eligible;
				ToggleAbilityStatus(*AbilitySpec, StatusTag);

				const FGameplayTag& PrevSlot = GetInputTagFromSpec(*AbilitySpec); //获取技能装配的插槽
				ClearSlot(AbilitySpec); //清除掉当前技能的输入标签
				ClientEquipAbility(AbilityTag, StatusTag, FGameplayTag(), PrevSlot);
			}
		}
		
		ClientUpdateAbilityStatus(AbilityTag, StatusTag, AbilitySpec->Level); //广播技能状态修改
		MarkAbilitySpecDirty(*AbilitySpec); //设置当前技能立即复制到每个客户端
	}
}

void URPGAbilitySystemComponent::ServerEquipAbility_Implementation(const FGameplayTag& AbilityTag, const FGameplayTag& Slot)
{
	const FRPGGameplayTags GameplayTags = FRPGGameplayTags::Get();

	//获取到技能实例
	if(FGameplayAbilitySpec* AbilitySpec = GetSpecFromAbilityTag(AbilityTag))
	{
		const FGameplayTag& PrevSlot = GetInputTagFromSpec(*AbilitySpec); //技能之前装配的插槽
		const FGameplayTag& Status = GetStatusTagFromSpec(*AbilitySpec); //当前技能的状态标签

		//判断技能的状态，技能状态只有在已装配或者已解锁的状态才可以装配
		if(Status == GameplayTags.Abilities_Status_Equipped || Status == GameplayTags.Abilities_Status_Unlocked)
		{
			//判断插槽是否有技能，有则需要将其清除
			if(!SlotIsEmpty(Slot))
			{
				//获取目标插槽现在装配的技能
				if(const FGameplayAbilitySpec* SpecWithSlot = GetSpecWithSlot(Slot))
				{
					//技能槽位装配相同的技能，直接返回，不做额外的处理
					if(AbilityTag.MatchesTagExact(GetAbilityTagFromSpec(*SpecWithSlot)))
					{
						ClientEquipAbility(AbilityTag, Status, Slot, PrevSlot);
						return;
					}

					//如果是被动技能，我们需要先将技能取消执行
					if(IsPassiveAbility(*SpecWithSlot))
					{
						MulticastActivatePassiveEffect(GetAbilityTagFromSpec(*SpecWithSlot), false); //关闭特效表现
						DeactivatePassiveAbility.Broadcast(GetAbilityTagFromSpec(*SpecWithSlot));
					}

					ClearAbilitiesOfSlot(Slot); //清除目标插槽装配的技能
				}
			}

			//技能没有设置到插槽（没有激活）
			if(!AbilityHasAnySlot(*AbilitySpec))
			{
				//如果是被动技能，装配即激活
				if(IsPassiveAbility(*AbilitySpec))
				{
					MulticastActivatePassiveEffect(AbilityTag, true); //开启特效表现
					TryActivateAbility(AbilitySpec->Handle);
				}
			}

			//修改技能装配的插槽
			AssignSlotToAbility(*AbilitySpec, Slot);

			//回调更新UI
			ClientEquipAbility(AbilityTag, Status, Slot, PrevSlot);
			MarkAbilitySpecDirty(*AbilitySpec); //立即将其复制到每个客户端
		}
	}
}

void URPGAbilitySystemComponent::ClientEquipAbility_Implementation(const FGameplayTag& AbilityTag, const FGameplayTag& Status, const FGameplayTag& Slot, const FGameplayTag& PreviousSlot)
{
	AbilityEquipped.Broadcast(AbilityTag, Status, Slot, PreviousSlot); //在客户端将更新后的标签广播
}

void URPGAbilitySystemComponent::ServerResetAllAbility_Implementation()
{
	FScopedAbilityListLock ActiveScopeLoc(*this); //域锁
	//遍历已经应用的技能
	for(FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		//获取状态标签
		FRPGGameplayTags GameplayTags = FRPGGameplayTags::Get();
		FGameplayTag StatusTag = GetStatusTagFromSpec(AbilitySpec);

		if(StatusTag.MatchesTagExact(GameplayTags.Abilities_Status_Equipped) || StatusTag.MatchesTagExact(GameplayTags.Abilities_Status_Unlocked))
		{
			const FGameplayTag AbilityTag = GetAbilityTagFromSpec(AbilitySpec);
			const FGameplayTag& PrevSlot = GetInputTagFromSpec(AbilitySpec); //获取技能装配的插槽
			//清空技能等级
			if(GetAvatarActor()->Implements<UPlayerInterface>())
			{
				IPlayerInterface::Execute_AddToSpellPoints(GetAvatarActor(), AbilitySpec.Level);
			}
			AbilitySpec.Level = 0;

			//清除状态标签
			StatusTag = GameplayTags.Abilities_Status_Eligible;
			ToggleAbilityStatus(AbilitySpec, StatusTag);

			//清除掉当前技能的输入标签
			ClearSlot(&AbilitySpec);

			//广播状态修改
			ClientUpdateAbilityStatus(AbilityTag, StatusTag, AbilitySpec.Level);
			ClientEquipAbility(AbilityTag, StatusTag, FGameplayTag(), PrevSlot);

			//立即将其复制到每个客户端
			MarkAbilitySpecDirty(AbilitySpec); 
		}
	}
}

bool URPGAbilitySystemComponent::GetDescriptionByAbilityTag(const FGameplayTag& AbilityTag, FString& OutDescription, FString& OutNextLevelDescription)
{
	//如果当前技能处于锁定状态，将无法获取到对应的技能描述
	if(FGameplayAbilitySpec* AbilitySpec = GetSpecFromAbilityTag(AbilityTag))
	{
		if(URPGGameplayAbility* RPGAbility = Cast<URPGGameplayAbility>(AbilitySpec->Ability))
		{
			OutDescription = RPGAbility->GetDescription(AbilitySpec->Level);
			OutNextLevelDescription = RPGAbility->GetNextLevelDescription(AbilitySpec->Level + 1);
			return true;
		}
	}

	//如果技能是锁定状态，将显示锁定技能描述
	const UAbilityInfo* AbilityInfo = URPGAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor());
	if(!AbilityTag.IsValid() || AbilityTag.MatchesTagExact(FRPGGameplayTags::Get().Abilities_None))
	{
		OutDescription = FString();
	}
	else
	{
		OutDescription = URPGGameplayAbility::GetLockedDescription(AbilityInfo->FindAbilityInfoForTag(AbilityTag).LevelRequirement);
	}
	OutNextLevelDescription = FString();
	return  false;
	
}

void URPGAbilitySystemComponent::ClearSlot(FGameplayAbilitySpec* Spec)
{
	const FGameplayTag Slot = GetInputTagFromSpec(*Spec);
	Spec->DynamicAbilityTags.RemoveTag(Slot);
	// MarkAbilitySpecDirty(*Spec);
}

void URPGAbilitySystemComponent::ClearAbilitiesOfSlot(const FGameplayTag& Slot)
{
	FScopedAbilityListLock ActiveScopeLock(*this);
	for(FGameplayAbilitySpec& Spec : GetActivatableAbilities())
	{
		if(AbilityHasSlot(&Spec, Slot))
		{
			ClearSlot(&Spec);
		}
	}
}

bool URPGAbilitySystemComponent::AbilityHasSlot(FGameplayAbilitySpec* Spec, const FGameplayTag& Slot)
{
	for(FGameplayTag Tag : Spec->DynamicAbilityTags)
	{
		if(Tag.MatchesTagExact(Slot))
		{
			return true;
		}
	}
	return false;
}

void URPGAbilitySystemComponent::OnRep_ActivateAbilities()
{
	Super::OnRep_ActivateAbilities();

	if(!bStartupAbilitiesGiven)
	{
		bStartupAbilitiesGiven = true;
		AbilityGivenDelegate.Broadcast();
	}
}

void URPGAbilitySystemComponent::ClientUpdateAbilityStatus_Implementation(const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag, const int32 AbilityLevel)
{
	AbilityStatusChanged.Broadcast(AbilityTag, StatusTag, AbilityLevel);
}

void URPGAbilitySystemComponent::ClientEffectApplied_Implementation(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveGameplayEffectHandle) const
{
	// GEngine->AddOnScreenDebugMessage(1, 8.f, FColor::Blue, FString("Effect Applied!"));
	FGameplayTagContainer TagContainer;
	EffectSpec.GetAllAssetTags(TagContainer);
	EffectAssetTags.Broadcast(TagContainer);
}
