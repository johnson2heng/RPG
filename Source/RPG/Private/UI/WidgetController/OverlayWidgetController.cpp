// 版权归暮志未晚所有。


#include "UI/WidgetController/OverlayWidgetController.h"

#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "AbilitySystem/RPGAttributeSet.h"
#include "AbilitySystem/Data/AbilityInfo.h"
#include "AbilitySystem/Data/LevelUpInfo.h"
#include "Player/RPGPlayerState.h"

void UOverlayWidgetController::BroadcastInitialValues()
{
	OnHealthChanged.Broadcast(GetRPGAS()->GetHealth());
	OnMaxHealthChanged.Broadcast(GetRPGAS()->GetMaxHealth());
	OnManaChanged.Broadcast(GetRPGAS()->GetMana());
	OnMaxManaChanged.Broadcast(GetRPGAS()->GetMaxMana());
}

void UOverlayWidgetController::BindCallbacksToDependencies()
{
	//绑定等级相关回调
	GetRPGPS()->OnXPChangedDelegate.AddUObject(this, &ThisClass::OnXPChanged);
	GetRPGPS()->OnLevelChangedDelegate.AddLambda([this](int32 NewLevel, bool bLevelUp)
	{
		OnPlayerLevelChangeDelegate.Broadcast(NewLevel, bLevelUp);
	});

	//绑定属性参数的回调
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetRPGAS()->GetHealthAttribute()).AddLambda([this](const FOnAttributeChangeData& Data){OnHealthChanged.Broadcast(Data.NewValue);});
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetRPGAS()->GetMaxHealthAttribute()).AddLambda([this](const FOnAttributeChangeData& Data){OnMaxHealthChanged.Broadcast(Data.NewValue);});
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetRPGAS()->GetManaAttribute()).AddLambda([this](const FOnAttributeChangeData& Data){OnManaChanged.Broadcast(Data.NewValue);});
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetRPGAS()->GetMaxManaAttribute()).AddLambda([this](const FOnAttributeChangeData& Data){OnMaxManaChanged.Broadcast(Data.NewValue);});

	//绑定ASC相关委托的回调
	if(GetRPGASC())
	{
		if(GetRPGASC()->bStartupAbilitiesGiven)
		{
			//如果执行到此处时，技能的初始化工作已经完成，则直接调用初始化回调
			BroadcastAbilityInfo();
		}
		else
		{
			//如果执行到此处，技能初始化还未完成，将通过绑定委托，监听广播的形式触发初始化完成回调
			GetRPGASC()->AbilityGivenDelegate.AddUObject(this, &ThisClass::BroadcastAbilityInfo);
		}

		//监听技能装配的回调
		GetRPGASC()->AbilityEquipped.AddUObject(this, &UOverlayWidgetController::OnAbilityEquipped);
		
		//绑定拾取信息回调
		GetRPGASC()->EffectAssetTags.AddLambda(
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
}

void UOverlayWidgetController::ClearAllDelegate()
{
	Super::ClearAllDelegate();

	OnHealthChanged.Clear();
	OnMaxHealthChanged.Clear();
	OnManaChanged.Clear();
	OnMaxManaChanged.Clear();
	MessageWidgetRowDelegate.Clear();
	OnXPPercentChangedDelegate.Clear();
	OnPlayerLevelChangeDelegate.Clear();
}

void UOverlayWidgetController::OnXPChanged(const int32 NewXP)
{
	const ULevelUpInfo* LevelUpInfo = GetRPGPS()->LevelUpInfo;
	checkf(LevelUpInfo, TEXT("无法查询到等级相关数据，请查看PlayerState是否设置了对应的数据"));

	const int32 Level =  LevelUpInfo->FindLevelForXP(NewXP); //获取当前等级
	const int32 MaxLevel = LevelUpInfo->LevelUpInformation.Num(); //获取当前最大等级

	if(Level <= MaxLevel && Level > 0)
	{
		const int32 LevelUpRequirement = LevelUpInfo->LevelUpInformation[Level].LevelUpRequirement; //当前等级升级所需经验值
		const int32 PreviousLevelUpRequirement = LevelUpInfo->LevelUpInformation[Level-1].LevelUpRequirement; //上一级升级所需经验值

		const float XPPercent = static_cast<float>(NewXP - PreviousLevelUpRequirement) / (LevelUpRequirement - PreviousLevelUpRequirement); //计算经验百分比
		OnXPPercentChangedDelegate.Broadcast(XPPercent); //广播经验条比例
	}
}

int32 UOverlayWidgetController::GetPlayerLevel()
{
	return GetRPGPS()->GetPlayerLevel();
}



