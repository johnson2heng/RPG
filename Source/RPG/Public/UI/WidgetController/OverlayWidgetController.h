// 版权归暮志未晚所有。

#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetController/RPGWidgetController.h"
#include "OverlayWidgetController.generated.h"

class URPGAbilitySystemComponent;
class UAbilityInfo;
class URPGUserWidget;

//用于显示拾取物信息的结构体，从鼠标表格中获取对应的数据。
USTRUCT(BlueprintType)
struct FUIWidgetRow : public FTableRowBase
{
	GENERATED_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag MessageTag = FGameplayTag();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Message = FText();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<class URPGUserWidget> MessageWidget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UTexture2D* Image = nullptr;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttributeChangedSignature, float, NewValue); //当玩家属性改变回调类型
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMessageWidgetRowSignature, FUIWidgetRow, Row); //拾取物品UI回调

/**
 * 屏幕覆盖用户控件控制器层基类，继承与用户控件控制器
 */
UCLASS(BlueprintType, Blueprintable)
class RPG_API UOverlayWidgetController : public URPGWidgetController
{
	GENERATED_BODY()

public:
	virtual void BroadcastInitialValues() override;
	virtual void BindCallbacksToDependencies() override;
	virtual void ClearAllDelegate() override;

	UPROPERTY(BlueprintAssignable, Category="GAS|Attributes")
	FOnAttributeChangedSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category="GAS|Attributes")
	FOnAttributeChangedSignature OnMaxHealthChanged;

	UPROPERTY(BlueprintAssignable, Category="GAS|Attributes")
	FOnAttributeChangedSignature OnManaChanged;

	UPROPERTY(BlueprintAssignable, Category="GAS|Attributes")
	FOnAttributeChangedSignature OnMaxManaChanged;

	UPROPERTY(BlueprintAssignable, Category="GAS|Messages")
	FMessageWidgetRowSignature MessageWidgetRowDelegate; //拾取信息委托回调

	UPROPERTY(BlueprintAssignable, Category="GAS|XP")
	FOnAttributeChangedSignature OnXPPercentChangedDelegate; //经验条百分比变动回调

	UPROPERTY(BlueprintAssignable, Category="GAS|Level")
	FOnPlayerLevelChangedSignature OnPlayerLevelChangeDelegate; //等级变动回调

protected:
	//拾取物的表格数据
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Widget Data")
	TObjectPtr<UDataTable> MessageWidgetDataTable;

	//根据传入的表格和Tag返回查找到的数据，表格类型不确定，所以使用T来表示，在使用此函数时，需要指定对应类型
	template<typename T>
	T* GetDataTableRowByTag(UDataTable* DataTable, const FGameplayTag& Tag);

	void OnXPChanged(int32 NewXP); //经验变动后的回调

	/**
	 * 获取角色的当前等级
	 * @return 
	 */
	UFUNCTION(BlueprintCallable)
	int32 GetPlayerLevel();
};

template <typename T>
T* UOverlayWidgetController::GetDataTableRowByTag(UDataTable* DataTable, const FGameplayTag& Tag)
{
	T* Row = DataTable->FindRow<T>(Tag.GetTagName(), TEXT(""));
	return Row;
}
