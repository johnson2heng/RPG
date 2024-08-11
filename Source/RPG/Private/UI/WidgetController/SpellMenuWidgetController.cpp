// 版权归暮志未晚所有。


#include "UI/WidgetController/SpellMenuWidgetController.h"

#include "RPGGameplayTags.h"
#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "Player/RPGPlayerState.h"

void USpellMenuWidgetController::BroadcastInitialValues()
{
	BroadcastAbilityInfo();

	SpellPointChanged.Broadcast(GetRPGPS()->GetSpellPoints()); //广播拥有的技能点
}

void USpellMenuWidgetController::BindCallbacksToDependencies()
{
	//绑定技能状态修改后的委托回调
	GetRPGASC()->AbilityStatusChanged.AddLambda([this](const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag, int32 NewLevel)
	{
		
		//技能状态修改，修改技能的升降级按钮的状态
		if(SelectedAbility.Ability.MatchesTagExact(AbilityTag))
		{
			SelectedAbility.Status = StatusTag;
			SelectedAbility.Level = NewLevel;

			BroadcastSpellGlobeSelected(); //广播升降级按钮状态
		}

		//广播技能数据更新，用于更新技能按钮显示状态
		if(AbilityInfo)
		{
			FRPGAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag); //获取到技能数据
			Info.StatusTag = StatusTag;
			AbilityInfoDelegate.Broadcast(Info);
		}
	});

	//绑定技能点变动回调
	GetRPGPS()->OnSpellPointsChangedDelegate.AddLambda([this](const int32 SpellPoints)
	{
		SpellPointChanged.Broadcast(SpellPoints); //广播拥有的技能点

		CurrentSpellPoints = SpellPoints;

		BroadcastSpellGlobeSelected(); //广播升降级按钮状态
	});
}

FGameplayTag USpellMenuWidgetController::SpellGlobeSelected(const FGameplayTag& AbilityTag)
{
	const FRPGGameplayTags GameplayTags = FRPGGameplayTags::Get();
	const int32 SpellPoints = GetRPGPS()->GetSpellPoints(); //获取技能点数
	FGameplayTag AbilityStatus;

	const bool bTagValid = AbilityTag.IsValid(); //判断传入的标签是否存在
	const bool bTagNone = AbilityTag.MatchesTag(GameplayTags.Abilities_None); //判断传入的是否为空技能标签
	const FGameplayAbilitySpec* AbilitySpec = GetRPGASC()->GetSpecFromAbilityTag(AbilityTag); //通过技能标签获取技能
	const bool bSpecValid = AbilitySpec != nullptr; //判断技能实例是否存在
	
	if(!bTagValid || bTagNone || !bSpecValid)
	{
		//传入标签不存在，或传入的为空技能标签，或者技能实例不存在时，设置为技能按钮显示上锁状态
		AbilityStatus = GameplayTags.Abilities_Status_Locked;
	}
	else
	{
		//从技能实例获取技能的状态标签
		AbilityStatus = GetRPGASC()->GetStatusTagFromSpec(*AbilitySpec);
		SelectedAbility.Level = AbilitySpec->Level;
	}

	//选中时，更新控制器缓存数据
	SelectedAbility.Ability = AbilityTag;
	SelectedAbility.Status = AbilityStatus;
	CurrentSpellPoints = SpellPoints;

	BroadcastSpellGlobeSelected(); //广播升降级按钮状态

	return AbilityStatus;
}

void USpellMenuWidgetController::SpendPointButtonPressed(const FGameplayTag& AbilityTag)
{
	if(GetRPGASC())
	{
		GetRPGASC()->ServerSpendSpellPoint(AbilityTag); //调用ASC等级提升函数
	}
}

void USpellMenuWidgetController::DemotionPointButtonPressed(const FGameplayTag& AbilityTag)
{
	if(GetRPGASC())
	{
		GetRPGASC()->ServerDemotionSpellPoint(AbilityTag); //调用ASC降低技能等级
	}
}

void USpellMenuWidgetController::ShouldEnableButtons(const FGameplayTag& AbilityStatus, bool HasSpellPoints, bool& bShouldEnableSpellPoints, bool& bShouldEnableEquip, bool& bShouldDemotionPoints)
{
	const FRPGGameplayTags GameplayTags = FRPGGameplayTags::Get();

	if (AbilityStatus.MatchesTagExact(GameplayTags.Abilities_Status_Equipped))
	{
		bShouldEnableSpellPoints = HasSpellPoints;
		bShouldEnableEquip = true;
		bShouldDemotionPoints = true;
	}
	else if (AbilityStatus.MatchesTagExact(GameplayTags.Abilities_Status_Eligible))
	{
		bShouldEnableSpellPoints = HasSpellPoints;
		bShouldEnableEquip = false;
		bShouldDemotionPoints = false;
	}
	else if (AbilityStatus.MatchesTagExact(GameplayTags.Abilities_Status_Unlocked))
	{
		bShouldEnableSpellPoints = HasSpellPoints;
		bShouldEnableEquip = true;
		bShouldDemotionPoints = true;
	}
	else if (AbilityStatus.MatchesTagExact(GameplayTags.Abilities_Status_Locked))
	{
		bShouldEnableSpellPoints = false;
		bShouldEnableEquip = false;
		bShouldDemotionPoints = false;
	}
}

void USpellMenuWidgetController::BroadcastSpellGlobeSelected()
{
	bool bEnableSpendPoints = false; //技能是否可以升级
	bool bEnableEquip = false; //技能是否可以装配
	bool bEnableDemotion = false; //技能是否可以降级

	ShouldEnableButtons(SelectedAbility.Status, CurrentSpellPoints > 0, bEnableSpendPoints, bEnableEquip, bEnableDemotion); //获取结果

	SpellGlobeSelectedSignature.Broadcast(bEnableSpendPoints, bEnableEquip, bEnableDemotion, SelectedAbility.Level); //广播状态

	//广播技能描述
	FString Description;
	FString NextLevelDescription;
	GetRPGASC()->GetDescriptionByAbilityTag(SelectedAbility.Ability, Description, NextLevelDescription);
	SpellDescriptionSignature.Broadcast(Description, NextLevelDescription);
}
