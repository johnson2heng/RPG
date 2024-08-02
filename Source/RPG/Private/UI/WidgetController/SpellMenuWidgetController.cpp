// 版权归暮志未晚所有。


#include "UI/WidgetController/SpellMenuWidgetController.h"
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
	GetRPGASC()->AbilityStatusChanged.AddLambda([this](const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag)
	{
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
	});
}
