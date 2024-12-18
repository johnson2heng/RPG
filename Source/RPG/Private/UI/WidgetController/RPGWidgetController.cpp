// 版权归暮志未晚所有。


#include "UI/WidgetController/RPGWidgetController.h"

#include "RPGGameplayTags.h"
#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "AbilitySystem/RPGAttributeSet.h"
#include "Player/RPGPlayerController.h"
#include "Player/RPGPlayerState.h"

void URPGWidgetController::SetWidgetControllerParams(const FWidgetControllerParams& WCParams)
{
	PlayerController = WCParams.PlayerController;
	PlayerState = WCParams.PlayerState;
	AbilitySystemComponent = WCParams.AbilitySystemComponent;
	AttributeSet = WCParams.AttributeSet;
}

void URPGWidgetController::BroadcastInitialValues()
{
}

void URPGWidgetController::BindCallbacksToDependencies()
{
}

void URPGWidgetController::BroadcastAbilityInfo()
{
	if(!GetRPGASC()->bStartupAbilitiesGiven) return; //判断当前技能初始化是否完成，触发回调时都已经完成

	//创建单播委托
	FForEachAbility BroadcastDelegate;
	//委托绑定回调匿名函数，委托广播时将会触发函数内部逻辑
	BroadcastDelegate.BindLambda([this](const FGameplayAbilitySpec& AbilitySpec)
	{
		//通过静态函数获取到技能实例的技能标签，并通过标签获取到技能数据
		FRPGAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(URPGAbilitySystemComponent::GetAbilityTagFromSpec(AbilitySpec));
		//获取到技能的输入标签
		Info.InputTag = URPGAbilitySystemComponent::GetInputTagFromSpec(AbilitySpec);
		//获取技能的状态标签
		Info.StatusTag = URPGAbilitySystemComponent::GetStatusTagFromSpec(AbilitySpec);
		//获取技能等级
		Info.Level = AbilitySpec.Level;
		//广播技能数据
		AbilityInfoDelegate.Broadcast(Info); 
	});
	//遍历技能并触发委托回调
	GetRPGASC()->ForEachAbility(BroadcastDelegate);
}

void URPGWidgetController::OnAbilityEquipped(const FGameplayTag& AbilityTag, const FGameplayTag& Status, const FGameplayTag& Slot, const FGameplayTag& PreviousSlot) const
{
	const FRPGGameplayTags GameplayTags = FRPGGameplayTags::Get();

	//清除旧插槽的数据
	FRPGAbilityInfo LastSlotInfo;
	LastSlotInfo.StatusTag = GameplayTags.Abilities_Status_Unlocked;
	LastSlotInfo.InputTag = PreviousSlot;
	LastSlotInfo.AbilityTag = GameplayTags.Abilities_None;
	AbilityInfoDelegate.Broadcast(LastSlotInfo);

	//更新新插槽的数据
	FRPGAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
	Info.StatusTag = Status;
	Info.InputTag = Slot;
	AbilityInfoDelegate.Broadcast(Info);
}

void URPGWidgetController::ClearAllDelegate()
{
	AbilityInfoDelegate.Clear();
}

ARPGPlayerController* URPGWidgetController::GetRPGPC()
{
	if(RPGPlayerController == nullptr)
	{
		RPGPlayerController = Cast<ARPGPlayerController>(PlayerController);
	}
	return RPGPlayerController;
}

ARPGPlayerState* URPGWidgetController::GetRPGPS()
{
	if(RPGPlayerState == nullptr)
	{
		RPGPlayerState = Cast<ARPGPlayerState>(PlayerState);
	}
	return RPGPlayerState;
}

URPGAbilitySystemComponent* URPGWidgetController::GetRPGASC()
{
	if(RPGAbilitySystemComponent == nullptr)
	{
		RPGAbilitySystemComponent = Cast<URPGAbilitySystemComponent>(AbilitySystemComponent);
	}
	return RPGAbilitySystemComponent;
}

URPGAttributeSet* URPGWidgetController::GetRPGAS()
{
	if(RPGAttributeSet == nullptr)
	{
		RPGAttributeSet = Cast<URPGAttributeSet>(AttributeSet);
	}
	return RPGAttributeSet;
}
