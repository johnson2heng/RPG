// 版权归暮志未晚所有。


#include "UI/WidgetController/OverlayWidgetController.h"

#include "AbilitySystem/AbilitySystemComponentBase.h"
#include "AbilitySystem/AttributeSetBase.h"

void UOverlayWidgetController::BroadcastInitialValues()
{
	const UAttributeSetBase* AttributeSetBase = CastChecked<UAttributeSetBase>(AttributeSet);

	OnHealthChanged.Broadcast(AttributeSetBase->GetHealth());
	OnMaxHealthChanged.Broadcast(AttributeSetBase->GetMaxHealth());
	OnManaChanged.Broadcast(AttributeSetBase->GetMana());
	OnMaxManaChanged.Broadcast(AttributeSetBase->GetMaxMana());
}

void UOverlayWidgetController::BindCallbacksToDependencies()
{
	const UAttributeSetBase* AttributeSetBase = CastChecked<UAttributeSetBase>(AttributeSet);

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSetBase->GetHealthAttribute()).AddLambda([this](const FOnAttributeChangeData& Data){OnHealthChanged.Broadcast(Data.NewValue);});
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSetBase->GetMaxHealthAttribute()).AddLambda([this](const FOnAttributeChangeData& Data){OnMaxHealthChanged.Broadcast(Data.NewValue);});
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSetBase->GetManaAttribute()).AddLambda([this](const FOnAttributeChangeData& Data){OnManaChanged.Broadcast(Data.NewValue);});
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSetBase->GetMaxManaAttribute()).AddLambda([this](const FOnAttributeChangeData& Data){OnMaxManaChanged.Broadcast(Data.NewValue);});

	//AddLambda 绑定匿名函数
	Cast<UAbilitySystemComponentBase>(AbilitySystemComponent)->EffectAssetTags.AddLambda(
		[this](const FGameplayTagContainer& AssetTags) //中括号添加this是为了保证内部能够获取类的对象
		{
			for(const FGameplayTag& Tag : AssetTags)
			{

				//对标签进行检测，如果不是信息标签，将无法进行广播
				FGameplayTag MessageTag = FGameplayTag::RequestGameplayTag(FName("Message"));
				// "A.1".MatchesTag("A") will return True, "A".MatchesTag("A.1") will return False
				if(Tag.MatchesTag(MessageTag))
				{
					const FUIWidgetRow* Row = GetDataTableRowByTag<FUIWidgetRow>(MessageWidgetDataTable, Tag);
					MessageWidgetRowDelegate.Broadcast(*Row); //前面加*取消指针引用
				}
				
				//将tag广播给Widget Controller 测试代码
				// const FString Msg = FString::Printf(TEXT("GE Tag in Widget Controller: %s"), *Tag.ToString()); //获取Asset Tag
				// GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Cyan, Msg); //打印到屏幕上 -1 不会被覆盖

			}
		}
	);
}



