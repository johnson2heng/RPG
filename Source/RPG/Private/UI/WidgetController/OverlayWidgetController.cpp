// 版权归暮志未晚所有。


#include "UI/WidgetController/OverlayWidgetController.h"

#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "AbilitySystem/RPGAttributeSet.h"
#include "AbilitySystem/Data/AbilityInfo.h"
#include "AbilitySystem/Data/LevelUpInfo.h"
#include "Player/RPGPlayerState.h"

void UOverlayWidgetController::BroadcastInitialValues()
{
	const URPGAttributeSet* AttributeSetBase = CastChecked<URPGAttributeSet>(AttributeSet);

	OnHealthChanged.Broadcast(AttributeSetBase->GetHealth());
	OnMaxHealthChanged.Broadcast(AttributeSetBase->GetMaxHealth());
	OnManaChanged.Broadcast(AttributeSetBase->GetMana());
	OnMaxManaChanged.Broadcast(AttributeSetBase->GetMaxMana());
}

void UOverlayWidgetController::BindCallbacksToDependencies()
{
	const URPGAttributeSet* AttributeSetBase = CastChecked<URPGAttributeSet>(AttributeSet);
	ARPGPlayerState* RPGPlayerState = CastChecked<ARPGPlayerState>(PlayerState);

	//绑定等级相关回调
	RPGPlayerState->OnXPChangedDelegate.AddUObject(this, &ThisClass::OnXPChanged);

	//绑定属性参数的回调
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSetBase->GetHealthAttribute()).AddLambda([this](const FOnAttributeChangeData& Data){OnHealthChanged.Broadcast(Data.NewValue);});
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSetBase->GetMaxHealthAttribute()).AddLambda([this](const FOnAttributeChangeData& Data){OnMaxHealthChanged.Broadcast(Data.NewValue);});
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSetBase->GetManaAttribute()).AddLambda([this](const FOnAttributeChangeData& Data){OnManaChanged.Broadcast(Data.NewValue);});
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSetBase->GetMaxManaAttribute()).AddLambda([this](const FOnAttributeChangeData& Data){OnMaxManaChanged.Broadcast(Data.NewValue);});

	//绑定ASC相关委托的回调
	if(URPGAbilitySystemComponent* RPGASC = Cast<URPGAbilitySystemComponent>(AbilitySystemComponent))
	{
		if(RPGASC->bStartupAbilitiesGiven)
		{
			//如果执行到此处时，技能的初始化工作已经完成，则直接调用初始化回调
			OnInitializeStartupAbilities(RPGASC);
		}
		else
		{
			//如果执行到此处，技能初始化还未完成，将通过绑定委托，监听广播的形式触发初始化完成回调
			RPGASC->AbilityGivenDelegate.AddUObject(this, &ThisClass::OnInitializeStartupAbilities);
		}
		
		//AddLambda 绑定匿名函数
		RPGASC->EffectAssetTags.AddLambda(
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

void UOverlayWidgetController::OnInitializeStartupAbilities(URPGAbilitySystemComponent* RPGAbilitySystemComponent) const
{
	if(!RPGAbilitySystemComponent->bStartupAbilitiesGiven) return; //判断当前技能初始化是否完成，触发回调时都已经完成

	//创建单播委托
	FForEachAbility BroadcastDelegate;
	//委托绑定回调匿名函数，委托广播时将会触发函数内部逻辑
	BroadcastDelegate.BindLambda([this](const FGameplayAbilitySpec& AbilitySpec)
	{
		//通过静态函数获取到技能实例的技能标签，并通过标签获取到技能数据
		FRPGAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(URPGAbilitySystemComponent::GetAbilityTagFromSpec(AbilitySpec));
		//获取到技能的输入标签
		Info.InputTag = URPGAbilitySystemComponent::GetInputTagFromSpec(AbilitySpec);
		//广播技能数据
		AbilityInfoDelegate.Broadcast(Info); 
	});
	//遍历技能并触发委托回调
	RPGAbilitySystemComponent->ForEachAbility(BroadcastDelegate);
}

void UOverlayWidgetController::OnXPChanged(int32 NewXP) const
{
	const ARPGPlayerState* RPGPlayerState = CastChecked<ARPGPlayerState>(PlayerState);
	const ULevelUpInfo* LevelUpInfo = RPGPlayerState->LevelUpInfo;
	checkf(LevelUpInfo, TEXT("无法查询到等级相关数据，请查看PlayerState是否设置了对应的数据"));

	const int32 Level =  LevelUpInfo->FindLevelForXP(NewXP); //获取当前等级
	const int32 MaxLevel = LevelUpInfo->LevelUpInformation.Num(); //获取当前最大等级

	if(Level <= MaxLevel && Level > 0)
	{
		const int32 LevelUpRequirement = LevelUpInfo->LevelUpInformation[Level].LevelUpRequirement; //当前等级升级所需经验值
		const int32 PreviousLevelUpRequirement = LevelUpInfo->LevelUpInformation[Level-1].LevelUpRequirement; //上一级升级所需经验值

		const float XPPercent = static_cast<float>((NewXP - PreviousLevelUpRequirement) / (LevelUpRequirement - PreviousLevelUpRequirement)); //计算经验百分比
		OnXPPercentChangedDelegate.Broadcast(XPPercent); //广播经验条比例
	}
}



