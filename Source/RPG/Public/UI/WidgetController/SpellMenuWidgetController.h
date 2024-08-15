// 版权归暮志未晚所有。

#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetController/RPGWidgetController.h"
#include "GameplayTagContainer.h"
#include "RPGGameplayTags.h"
#include "SpellMenuWidgetController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FSpellGlobeSelectedSignature, bool, bSpendPointsEnabled, bool, bEquipEnabled, bool, bDemotionPointsEnabled, int32, AbilityLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSpellDescriptionSignature, FString, SpellDescription, FString, SpellNextLevelDescription); //技能描述委托

//在技能面板选中的技能的标签结构体
struct FSelectedAbility
{
	FGameplayTag Ability = FGameplayTag(); //技能标签
	FGameplayTag Status = FGameplayTag(); //技能状态标签
	int32 Level = 0; //技能等级
};

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class RPG_API USpellMenuWidgetController : public URPGWidgetController
{
	GENERATED_BODY()

public:

	virtual void BroadcastInitialValues() override;
	virtual void BindCallbacksToDependencies() override;

	UPROPERTY(BlueprintAssignable)
	FOnPlayerStateChangedSignature SpellPointChanged; //升级技能点数变动回调

	UPROPERTY(BlueprintAssignable)
	FSpellGlobeSelectedSignature SpellGlobeSelectedSignature; //选中技能按钮后，升级和装备按钮的变动回调

	UPROPERTY(BlueprintAssignable)
	FSpellDescriptionSignature SpellDescriptionSignature; //选中技能按钮后，技能和下一级描述委托

	UFUNCTION(BlueprintCallable)
	FGameplayTag SpellGlobeSelected(const FGameplayTag& AbilityTag); //技能按钮选中调用函数，处理升级按钮和装配

	UFUNCTION(BlueprintCallable)
	void GlobeDeselect(); //取消按钮选中处理

	UFUNCTION(BlueprintCallable)
	void SpendPointButtonPressed(const FGameplayTag& AbilityTag); //升级按钮调用函数

	UFUNCTION(BlueprintCallable)
	void DemotionPointButtonPressed(const FGameplayTag& AbilityTag); //降级按钮调用函数

private:

	//通过技能状态标签和可分配技能点数来获取技能是否可以装配和技能是否可以升级
	static void ShouldEnableButtons(const FGameplayTag& AbilityStatus, bool HasSpellPoints, bool& bShouldEnableSpellPoints, bool& bShouldEnableEquip, bool& bShouldDemotionPoints);

	//广播当前技能按钮升降级按钮状态和可装配状态
	void BroadcastSpellGlobeSelected();

	//设置选中技能默认值
	FSelectedAbility SelectedAbility = {FRPGGameplayTags::Get().Abilities_None, FRPGGameplayTags::Get().Abilities_Status_Locked, 0};

	//保存当前技能可分配点数
	int32 CurrentSpellPoints = 0;
};
