// 版权归暮志未晚所有。

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Data/AbilityInfo.h"
#include "RPGWidgetController.generated.h"

class URPGAttributeSet;
class URPGAbilitySystemComponent;
class ARPGPlayerState;
class ARPGPlayerController;
class UAttributeSet;
class UAbilitySystemComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerStateChangedSignature, int32, NewValue); //当玩家状态更改回调类型（等级，技能点）
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerLevelChangedSignature, int32, NewValue, bool, bLevelUp); //当玩家状态更改回调类型（等级，技能点）
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAbilityInfoSignature, const FRPGAbilityInfo, Info); //技能更新UI回调

/**
 * 生成用户控件控制器的结构体，需要四项内容去生成。
 */
USTRUCT(BlueprintType)
struct FWidgetControllerParams
{
	GENERATED_BODY()
	FWidgetControllerParams(){}
	FWidgetControllerParams(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS)
	: PlayerController(PC),
	PlayerState(PS),
	AbilitySystemComponent(ASC),
	AttributeSet(AS)
	{}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<APlayerController> PlayerController = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<APlayerState> PlayerState = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAttributeSet> AttributeSet = nullptr;
};

/**
 * 用户控件控制器层，用户控件可从控制器层更新数据显示，以及为控制器层提供输入。
 */
UCLASS()
class RPG_API URPGWidgetController : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetWidgetControllerParams(const FWidgetControllerParams& WCParams);

	UFUNCTION(BlueprintCallable)
	virtual void BroadcastInitialValues(); //广播初始化的值
	virtual void BindCallbacksToDependencies(); //绑定数值变动后回调的广播

	UPROPERTY(BlueprintAssignable, Category="GAS|Messages")
	FAbilityInfoSignature AbilityInfoDelegate; //技能数据更新委托回调

	void BroadcastAbilityInfo(); //广播技能信息

	//监听技能装配后的处理
	void OnAbilityEquipped(const FGameplayTag& AbilityTag, const FGameplayTag& Status, const FGameplayTag& Slot, const FGameplayTag& PreviousSlot) const;

	UFUNCTION(BlueprintCallable)
	virtual void ClearAllDelegate();  //清除全部委托绑定

protected:

	//技能的表格数据
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Widget Data")
	TObjectPtr<UAbilityInfo> AbilityInfo;

	UPROPERTY(BlueprintReadOnly, Category="WidgetController")
	TObjectPtr<APlayerController> PlayerController;

	UPROPERTY(BlueprintReadOnly, Category="WidgetController")
	TObjectPtr<APlayerState> PlayerState;

	UPROPERTY(BlueprintReadOnly, Category="WidgetController")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(BlueprintReadOnly, Category="WidgetController")
	TObjectPtr<UAttributeSet> AttributeSet;

	UPROPERTY()
	TObjectPtr<ARPGPlayerController> RPGPlayerController;

	UPROPERTY()
	TObjectPtr<ARPGPlayerState> RPGPlayerState;

	UPROPERTY()
	TObjectPtr<URPGAbilitySystemComponent> RPGAbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<URPGAttributeSet> RPGAttributeSet;

	ARPGPlayerController* GetRPGPC();
	ARPGPlayerState* GetRPGPS();
	URPGAbilitySystemComponent* GetRPGASC();
	URPGAttributeSet* GetRPGAS();
};
