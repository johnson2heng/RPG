// 版权归暮志未晚所有。


#include "AbilitySystem/RPGAttributeSet.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffectExtension.h"
#include "RPGAbilityTypes.h"
#include "RPGGameplayTags.h"
#include "AbilitySystem/RPGAbilitySystemLibrary.h"
#include "GameFramework/Character.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"
#include "Interaction/CombatInterface.h"
#include "Interaction/PlayerInterface.h"
#include "Net/UnrealNetwork.h"
#include "Player/RPGPlayerController.h"

URPGAttributeSet::URPGAttributeSet()
{
	const FRPGGameplayTags& GameplayTags = FRPGGameplayTags::Get();

	//------------------------------------------------属性标签和属性关联------------------------------------------------

	/*
	 * Primary Attribute
	 */
	TagsToAttributes.Add(GameplayTags.Attributes_Primary_Strength, GetStrengthAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Primary_Intelligence, GetIntelligenceAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Primary_Resilience, GetResilienceAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Primary_Vigor, GetVigorAttribute);

	/*
	 * Secondary Attribute
	 */
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_Armor, GetArmorAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_ArmorPenetration, GetArmorPenetrationAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_BlockChance, GetBlockChanceAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_CriticalHitChance, GetCriticalHitChanceAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_CriticalHitDamage, GetCriticalHitDamageAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_CriticalHitResistance, GetCriticalHitResistanceAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_HealthRegeneration, GetHealthRegenerationAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_ManaRegeneration, GetManaRegenerationAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_MaxHealth, GetMaxHealthAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_MaxMana, GetMaxManaAttribute);
	
	/*
	 * Resistance Attribute
	 */
	TagsToAttributes.Add(GameplayTags.Attributes_Resistance_Fire, GetFireResistanceAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Resistance_Lightning, GetLightningResistanceAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Resistance_Arcane, GetArcaneResistanceAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Resistance_Physical, GetPhysicalResistanceAttribute);
}

void URPGAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	//主要属性
	DOREPLIFETIME_CONDITION_NOTIFY(URPGAttributeSet, Strength, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URPGAttributeSet, Intelligence, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URPGAttributeSet, Resilience, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URPGAttributeSet, Vigor, COND_None, REPNOTIFY_Always);
	//次级属性
	DOREPLIFETIME_CONDITION_NOTIFY(URPGAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URPGAttributeSet, MaxMana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URPGAttributeSet, Armor, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URPGAttributeSet, ArmorPenetration, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URPGAttributeSet, BlockChance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URPGAttributeSet, CriticalHitChance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URPGAttributeSet, CriticalHitDamage, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URPGAttributeSet, CriticalHitResistance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URPGAttributeSet, HealthRegeneration, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URPGAttributeSet, ManaRegeneration, COND_None, REPNOTIFY_Always);
	//抗性属性
	DOREPLIFETIME_CONDITION_NOTIFY(URPGAttributeSet, FireResistance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URPGAttributeSet, LightningResistance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URPGAttributeSet, ArcaneResistance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URPGAttributeSet, PhysicalResistance, COND_None, REPNOTIFY_Always);
	//至关重要的属性
	DOREPLIFETIME_CONDITION_NOTIFY(URPGAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URPGAttributeSet, Mana, COND_None, REPNOTIFY_Always);
}

void URPGAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if(Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
		// UE_LOG(LogTemp, Warning, TEXT("Health: %f"), NewValue);
	}
	else if (Attribute == GetManaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxMana());
	}
	
}

void URPGAttributeSet::SetEffectProperties(const FGameplayEffectModCallbackData& Data, FEffectProperties& Props)
{
	//Source 效果的所有者   Target 效果应用的目标
	Props.EffectContextHandle = Data.EffectSpec.GetContext();
	Props.SourceASC = Props.EffectContextHandle.GetOriginalInstigatorAbilitySystemComponent(); //获取效果所有者的ASC

	//获取GE源的相关对象
	if(IsValid(Props.SourceASC) && Props.SourceASC->AbilityActorInfo.IsValid() && Props.SourceASC->AbilityActorInfo->AvatarActor.IsValid())
	{
		Props.SourceAvatarActor = Props.SourceASC->AbilityActorInfo->AvatarActor.Get(); //获取Actor
		Props.SourceController = Props.SourceASC->AbilityActorInfo->PlayerController.Get(); //获取PlayerController
		if(Props.SourceController == nullptr && Props.SourceAvatarActor != nullptr)
		{
			if(const APawn* Pawn = Cast<APawn>(Props.SourceAvatarActor))
			{
				Props.SourceController = Pawn->GetController();
			}
		}

		if(Props.SourceController)
		{
			Props.SourceCharacter = Cast<ACharacter>(Props.SourceController->GetPawn());
		}
	}

	//获取GE目标的相关对象
	if(Data.Target.AbilityActorInfo.IsValid() && Data.Target.AbilityActorInfo->AvatarActor.IsValid())
	{
		Props.TargetAvatarActor = Data.Target.AbilityActorInfo->AvatarActor.Get();
		Props.TargetController = Data.Target.AbilityActorInfo->PlayerController.Get();
		Props.TargetCharacter = Cast<ACharacter>(Props.TargetAvatarActor);
		Props.TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Props.TargetAvatarActor);
	}
}

void URPGAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	FEffectProperties Props;
	SetEffectProperties(Data, Props);

	//判断当前目标是否已经死亡，如果死亡，将不再进行处理
	if(Props.TargetCharacter->Implements<UCombatInterface>() && ICombatInterface::Execute_IsDead(Props.TargetCharacter)) return;

	if(Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
		// UE_LOG(LogTemp, Warning, TEXT("%s 的生命值发生了修改，当前生命值：%f"), *Props.TargetAvatarActor->GetName(), GetHealth());
	}

	if(Data.EvaluatedData.Attribute == GetManaAttribute())
	{
		SetMana(FMath::Clamp(GetMana(), 0.f, GetMaxMana()));
	}

	if(Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
	{
		HandleIncomingDamage(Props);
	}

	if(Data.EvaluatedData.Attribute == GetIncomingXPAttribute())
	{
		HandleIncomingXP(Props);
	}
	
}


void URPGAttributeSet::HandleIncomingDamage(const FEffectProperties& Props)
{
	const float LocalIncomingDamage = GetIncomingDamage();
	SetIncomingDamage(0.f);
	if(LocalIncomingDamage > 0.f)
	{
		const float NewHealth = GetHealth() - LocalIncomingDamage;
		SetHealth(FMath::Clamp(NewHealth, 0.f, GetMaxHealth()));

		//如果血量小于等于0，角色将会死亡
		if(NewHealth <= 0.f)
		{
			//调用死亡函数
			if(ICombatInterface* CombatInterface = Cast<ICombatInterface>(Props.TargetAvatarActor))
			{
				CombatInterface->Die(URPGAbilitySystemLibrary::GetDeathImpulse(Props.EffectContextHandle));
			}
			//死亡时，发送经验事件
			SendXPEvent(Props);
		}
		else
		{
			//在受到闪电链攻击时，不会激活受击
			if(Props.TargetCharacter->Implements<UCombatInterface>() && !ICombatInterface::Execute_IsBeingShocked(Props.SourceCharacter))
			{
				//激活受击技能
				FGameplayTagContainer TagContainer;
				TagContainer.AddTag(FRPGGameplayTags::Get().Effects_HitReact);
				// Props.TargetASC->CancelAbilities(&TagContainer); //先取消之前的受击
				Props.TargetASC->TryActivateAbilitiesByTag(TagContainer); //根据tag标签激活技能
			}

			//判断当前是否应用负面效果
			if(URPGAbilitySystemLibrary::IsSuccessfulDeBuff(Props.EffectContextHandle))
			{
				HandleDeBuff(Props);
			}

			//设置攻击击退效果
			const FVector& KnockbackForce = URPGAbilitySystemLibrary::GetKnockbackForce(Props.EffectContextHandle);
			if(!KnockbackForce.IsNearlyZero(1.f)) //如果击退的值不接近于0，则触发击退效果
			{
				Props.TargetCharacter->LaunchCharacter(KnockbackForce, true, true);
			}
		}

		//获取格挡和暴击
		const bool IsBlockedHit = URPGAbilitySystemLibrary::IsBlockedHit(Props.EffectContextHandle);
		const bool IsCriticalHit = URPGAbilitySystemLibrary::IsCriticalHit(Props.EffectContextHandle);

		//显示伤害数字
		ShowFloatingText(Props, LocalIncomingDamage, IsBlockedHit, IsCriticalHit);
	}
}

void URPGAttributeSet::HandleDeBuff(const FEffectProperties& Props)
{
	const FRPGGameplayTags& GameplayTags = FRPGGameplayTags::Get();
	//获取负面效果相关参数
	const FGameplayTag DeBuffType = URPGAbilitySystemLibrary::GetDeBuffDamageType(Props.EffectContextHandle);
	const float DeBuffDamage = URPGAbilitySystemLibrary::GetDeBuffDamage(Props.EffectContextHandle);
	const float DeBuffDuration = URPGAbilitySystemLibrary::GetDeBuffDuration(Props.EffectContextHandle);
	const float DeBuffFrequency = URPGAbilitySystemLibrary::GetDeBuffFrequency(Props.EffectContextHandle);

	//创建GE所使用的名称，并创建一个可实例化的GE
	FString DeBuffName = FString::Printf(TEXT("DynamicDeBuff_%s"), *DeBuffType.ToString());
	UGameplayEffect* Effect = NewObject<UGameplayEffect>(GetTransientPackage(), FName(DeBuffName));

	//设置动态创建GE的属性
	Effect->DurationPolicy = EGameplayEffectDurationType::HasDuration; //设置GE为有时间限制的效果
	Effect->DurationMagnitude = FScalableFloat(DeBuffDuration); //设置GE的持续时间
	
	Effect->Period = FScalableFloat(DeBuffFrequency); //设置GE的触发策略，间隔时间
	Effect->bExecutePeriodicEffectOnApplication = false; //在应用后不会立即触发，而是在经过了Period后才会触发
	Effect->PeriodicInhibitionPolicy = EGameplayEffectPeriodInhibitionRemovedPolicy::NeverReset; //设置每次应用后不会重置触发时间

	//设置可叠加层数
	Effect->StackingType = EGameplayEffectStackingType::AggregateBySource; //设置GE应用基于释放者查看
	Effect->StackLimitCount = 1; //设置叠加层数
	Effect->StackDurationRefreshPolicy = EGameplayEffectStackingDurationPolicy::RefreshOnSuccessfulApplication; //在应用后重置时，重置持续时间
	Effect->StackPeriodResetPolicy = EGameplayEffectStackingPeriodPolicy::ResetOnSuccessfulApplication; //在应用时，触发并重置Period时间
	Effect->StackExpirationPolicy = EGameplayEffectStackingExpirationPolicy::ClearEntireStack; //GE时间到了默认清除所有层数，还有可以清除单层的设置
	//Effect->OverflowEffects.Add() //在叠加层数超出时，将触发此数组内的GE应用到角色
	// Effect->bDenyOverflowApplication = true; //设置为true时，叠加层数超出时，将不会刷新GE实例
	// Effect->bClearStackOnOverflow = true; //设置为true时，叠加层数超出时，将清除GE

	//在5.3版本修改为通过GEComponent来设置GE应用的标签，向目标Actor增加对应的标签
	UTargetTagsGameplayEffectComponent& TargetTagsGameplayEffectComponent = Effect->AddComponent<UTargetTagsGameplayEffectComponent>();
	FInheritedTagContainer InheritableOwnedTagsContainer; //创建组件所需的标签容器
	InheritableOwnedTagsContainer.Added.AddTag(DeBuffType); //添加标签
	TargetTagsGameplayEffectComponent.SetAndApplyTargetTagChanges(InheritableOwnedTagsContainer); //应用并更新

	//设置属性修改
	const int32 Index = Effect->Modifiers.Num(); //获取当前修改属性的Modifiers的长度，也就是下一个添加的modify的下标索引
	Effect->Modifiers.Add(FGameplayModifierInfo()); //添加一个新的Modify
	FGameplayModifierInfo& ModifierInfo = Effect->Modifiers[Index]; //通过下标索引获取Modify

	ModifierInfo.ModifierMagnitude = FScalableFloat(DeBuffDamage); //设置应用的属性值
	ModifierInfo.ModifierOp = EGameplayModOp::Additive; //设置属性运算符号
	ModifierInfo.Attribute = GetIncomingDamageAttribute(); //设置修改的属性

	// const FGameplayTagContainer TagContainer =  Effect->GetGrantedTags();
	// const bool bHas = TagContainer.HasTag(DeBuffType);
	// UE_LOG(LogTemp, Warning, TEXT("添加了负面效果，buff为：%s"), *LexToString(bHas));

	//创建GE实例，并添加伤害类型标签，应用GE
	FGameplayEffectContextHandle EffectContextHandle = Props.SourceASC->MakeEffectContext();
	EffectContextHandle.AddSourceObject(Props.SourceCharacter);
	if(const FGameplayEffectSpec* MutableSpec = new FGameplayEffectSpec(Effect, EffectContextHandle, 1.f))
	{
		FRPGGameplayEffectContext* RPGContext = static_cast<FRPGGameplayEffectContext*>(MutableSpec->GetContext().Get());
		const TSharedPtr<FGameplayTag> DeBuffDamageType = MakeShareable(new FGameplayTag(DeBuffType));
		RPGContext->SetDeBuffDamageType(DeBuffDamageType);
		
		Props.TargetASC->ApplyGameplayEffectSpecToSelf(*MutableSpec);
	}

}

void URPGAttributeSet::HandleIncomingXP(const FEffectProperties& Props)
{
	const float LocalIncomingXP = GetIncomingXP();
	SetIncomingXP(0);
	// UE_LOG(LogRPG, Log, TEXT("获取传入经验值：%f"), LocalIncomingXP);
		
	if(Props.SourceCharacter->Implements<UPlayerInterface>() && Props.SourceCharacter->Implements<UCombatInterface>())
	{
		//获取角色当前等级和经验
		const int32 CurrentLevel = ICombatInterface::Execute_GetPlayerLevel(Props.SourceCharacter);
		const int32 CurrentXP = IPlayerInterface::Execute_GetXP(Props.SourceCharacter);

		//获取获得经验后的新等级
		const int32 NewLevel = IPlayerInterface::Execute_FindLevelForXP(Props.SourceCharacter, CurrentXP + LocalIncomingXP);
		const int32 NumLevelUps = NewLevel - CurrentLevel; //查看等级是否有变化
		if(NumLevelUps > 0)
		{
			//升级提供的属性点
			int32 AttributePointsReward = 0;
			//升级提供的技能点
			int32 SpellPointsReward = 0;
			
			//如果连升多级，我们通过for循环获取每个等级的奖励
			for(int32 i = CurrentLevel; i < NewLevel; i++)
			{
				//获取升级提供的技能点和属性点
				AttributePointsReward += IPlayerInterface::Execute_GetAttributePointsReward(Props.SourceCharacter, i);
				SpellPointsReward += IPlayerInterface::Execute_GetSpellPointsReward(Props.SourceCharacter, i);
			}

			//增加角色技能点和属性点
			IPlayerInterface::Execute_AddToAttributePoints(Props.SourceCharacter, AttributePointsReward);
			IPlayerInterface::Execute_AddToSpellPoints(Props.SourceCharacter, SpellPointsReward);

			//提升等级
			IPlayerInterface::Execute_AddToPlayerLevel(Props.SourceCharacter, NumLevelUps);

			//播放升级效果
			IPlayerInterface::Execute_LevelUp(Props.SourceCharacter);

			//将血量和蓝量填充满, 我们将设置变量
			SetHealth(GetMaxHealth());
			SetMana(GetMaxMana());
		}
			
		//将经验应用给自身，通过事件传递，在玩家角色被动技能GA_ListenForEvents里接收
		IPlayerInterface::Execute_AddToXP(Props.SourceCharacter, LocalIncomingXP);
	}
}

void URPGAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	if (!FMath::IsNearlyEqual(OldValue, NewValue)) //判断值是否产生了变化
	{
		if (Attribute == GetMaxHealthAttribute())
		{
			const float CurrentValue = GetHealth(); //获取当前的实际值
			float NewDelta = (OldValue > 0.f) ? (CurrentValue * NewValue / OldValue) - CurrentValue : NewValue; //获取到最大值变动后,按比列修改后的值
			NewDelta = FMath::Max(NewDelta, 0.f);
			SetHealth(NewDelta + CurrentValue);
		}
		else if (Attribute == GetMaxManaAttribute())
		{
			const float CurrentValue = GetMana(); //获取当前的实际值
			float NewDelta = (OldValue > 0.f) ? (CurrentValue * NewValue / OldValue) - CurrentValue : NewValue; //获取到最大值变动后,按比列修改后的值
			NewDelta = FMath::Max(NewDelta, 0.f);
			SetMana(NewDelta + CurrentValue);
		}
	}
}

void URPGAttributeSet::ShowFloatingText(const FEffectProperties& Props, const float Damage, bool IsBlockedHit, bool IsCriticalHit)
{
	//调用显示伤害数字
	if(Props.SourceCharacter != Props.TargetCharacter)
	{
		//从技能释放者身上获取PC并显示伤害数字
		if(ARPGPlayerController* PC = Cast<ARPGPlayerController>(Props.SourceCharacter->Controller))
		{
			PC->ShowDamageNumber(Damage, Props.TargetCharacter, IsBlockedHit, IsCriticalHit); //调用显示伤害数字
		}
		//从目标身上获取PC并显示伤害数字
		if(ARPGPlayerController* PC = Cast<ARPGPlayerController>(Props.TargetCharacter->Controller))
		{
			PC->ShowDamageNumber(Damage, Props.TargetCharacter, IsBlockedHit, IsCriticalHit); //调用显示伤害数字
		}
	}
}

void URPGAttributeSet::SendXPEvent(const FEffectProperties& Props)
{
	int32 TargetLevel = 1;
	if(Props.TargetCharacter->Implements<UCombatInterface>())
	{
		TargetLevel = ICombatInterface::Execute_GetPlayerLevel(Props.TargetCharacter);
	}
	const ECharacterClass TargetClass = ICombatInterface::Execute_GetCharacterClass(Props.TargetCharacter); //c++内调用BlueprintNativeEvent函数需要这样调用
	const int32 XPReward = URPGAbilitySystemLibrary::GetXPRewardForClassAndLevel(Props.TargetCharacter, TargetClass, TargetLevel);

	const FRPGGameplayTags& GameplayTags = FRPGGameplayTags::Get();
	FGameplayEventData Payload; //创建Payload
	Payload.EventTag = GameplayTags.Attributes_Meta_IncomingXP;
	Payload.EventMagnitude = XPReward;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Props.SourceCharacter, GameplayTags.Attributes_Meta_IncomingXP, Payload);
	
}

void URPGAttributeSet::OnRep_Strength(const FGameplayAttributeData& OldStrength) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URPGAttributeSet, Strength, OldStrength);
}

void URPGAttributeSet::OnRep_Intelligence(const FGameplayAttributeData& OldIntelligence) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URPGAttributeSet, Intelligence, OldIntelligence);
}

void URPGAttributeSet::OnRep_Resilience(const FGameplayAttributeData& OldResilience) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URPGAttributeSet, Resilience, OldResilience);
}

void URPGAttributeSet::OnRep_Vigor(const FGameplayAttributeData& OldVigor) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URPGAttributeSet, Vigor, OldVigor);
}

void URPGAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URPGAttributeSet, Health, OldHealth);
}

void URPGAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URPGAttributeSet, MaxHealth, OldMaxHealth);
}

void URPGAttributeSet::OnRep_Mana(const FGameplayAttributeData& OldMana) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URPGAttributeSet, Mana, OldMana);
}

void URPGAttributeSet::OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URPGAttributeSet, MaxMana, OldMaxMana);
}

void URPGAttributeSet::OnRep_Armor(const FGameplayAttributeData& OldArmor) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URPGAttributeSet, Armor, OldArmor);
}

void URPGAttributeSet::OnRep_ArmorPenetration(const FGameplayAttributeData& OldArmorPenetration) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URPGAttributeSet, ArmorPenetration, OldArmorPenetration);
}

void URPGAttributeSet::OnRep_BlockChance(const FGameplayAttributeData& OldBlockChance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URPGAttributeSet, BlockChance, OldBlockChance);
}

void URPGAttributeSet::OnRep_CriticalHitChance(const FGameplayAttributeData& OldCriticalHitChance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URPGAttributeSet, CriticalHitChance, OldCriticalHitChance);
}

void URPGAttributeSet::OnRep_CriticalHitDamage(const FGameplayAttributeData& OldCriticalHitDamage) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URPGAttributeSet, CriticalHitDamage, OldCriticalHitDamage);
}

void URPGAttributeSet::OnRep_CriticalHitResistance(const FGameplayAttributeData& OldCriticalHitResistance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URPGAttributeSet, CriticalHitResistance, OldCriticalHitResistance);
}

void URPGAttributeSet::OnRep_HealthRegeneration(const FGameplayAttributeData& OldHealthRegeneration) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URPGAttributeSet, HealthRegeneration, OldHealthRegeneration);
}

void URPGAttributeSet::OnRep_ManaRegeneration(const FGameplayAttributeData& OldManaRegeneration) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URPGAttributeSet, ManaRegeneration, OldManaRegeneration);
}

void URPGAttributeSet::OnRep_FireResistance(const FGameplayAttributeData& OldFireResistance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URPGAttributeSet, FireResistance, OldFireResistance);
}

void URPGAttributeSet::OnRep_LightningResistance(const FGameplayAttributeData& OldLightningResistance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URPGAttributeSet, LightningResistance, OldLightningResistance);
}

void URPGAttributeSet::OnRep_ArcaneResistance(const FGameplayAttributeData& OldArcaneResistance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URPGAttributeSet, ArcaneResistance, OldArcaneResistance);
}

void URPGAttributeSet::OnRep_PhysicalResistance(const FGameplayAttributeData& OldPhysicalResistance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URPGAttributeSet, PhysicalResistance, OldPhysicalResistance);
}

