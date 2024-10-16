// 版权归暮志未晚所有。


#include "UI/WidgetController/AttributeMenuWidgetController.h"

#include "RPGGameplayTags.h"
#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "AbilitySystem/RPGAttributeSet.h"
#include "AbilitySystem/Data/AttributeInfo.h"
#include "Player/RPGPlayerState.h"

void UAttributeMenuWidgetController::BindCallbacksToDependencies()
{
	for (auto& Pair : GetRPGAS()->TagsToAttributes)
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Pair.Value()).AddLambda(
			[this,Pair](const FOnAttributeChangeData& Data)
			{
				BroadcastAttributeInfo(Pair.Key, Pair.Value());
			}
		);
	}
	
	//绑定PlayerState的属性点委托
	GetRPGPS()->OnAttributePointsChangedDelegate.AddLambda([this](const int32 Points)
	{
		AttributePointsChangedDelegate.Broadcast(Points);
	});
}

void UAttributeMenuWidgetController::BroadcastInitialValues()
{
	for (auto& Pair : GetRPGAS()->TagsToAttributes)
	{
		BroadcastAttributeInfo(Pair.Key, Pair.Value());
	}

	AttributePointsChangedDelegate.Broadcast(GetRPGPS()->GetAttributePoints());
}

void UAttributeMenuWidgetController::ClearAllDelegate()
{
	Super::ClearAllDelegate();

	AttributeInfoDelegate.Clear();
	AttributePointsChangedDelegate.Clear();
}

void UAttributeMenuWidgetController::UpgradeAttribute(const FGameplayTag& AttributeTag)
{
	GetRPGASC()->UpgradeAttribute(AttributeTag);
}

void UAttributeMenuWidgetController::BroadcastAttributeInfo(const FGameplayTag& AttributeTag, const FGameplayAttribute& Attribute) const
{
	FRPGAttributeInfo Info = AttributeInfo->FindAttributeInfoForTag(AttributeTag);
	Info.AttributeValue = Attribute.GetNumericValue(AttributeSet);
	AttributeInfoDelegate.Broadcast(Info);
}
