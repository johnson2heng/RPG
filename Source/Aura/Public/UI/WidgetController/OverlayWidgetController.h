// 版权归暮志未晚所有。

#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetController/MyWidgetController.h"
#include "OverlayWidgetController.generated.h"

class UMyUserWidget;

USTRUCT(BlueprintType)
struct FUIWidgetRow : public FTableRowBase
{
	GENERATED_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag MessageTag = FGameplayTag();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Message = FText();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<class UMyUserWidget> MessageWidget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UTexture2D* Image = nullptr;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthChangedSignature, float, NewHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMaxHealthChangedSignature,float, NewMaxHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnManaChangedSignature,float, NewMana);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMaxManaChangedSignature,float, NewMaxMana);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMessageWidgetRowSignature,FUIWidgetRow, Row);

/**
 * 屏幕覆盖用户控件控制器层基类，继承与用户控件控制器
 */
UCLASS(BlueprintType, Blueprintable)
class AURA_API UOverlayWidgetController : public UMyWidgetController
{
	GENERATED_BODY()

public:
	virtual void BroadcastInitialValues() override;
	virtual void BindCallbacksToDependencies() override;

	UPROPERTY(BlueprintAssignable, Category="GAS|Attributes")
	FOnHealthChangedSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category="GAS|Attributes")
	FOnMaxHealthChangedSignature OnMaxHealthChanged;

	UPROPERTY(BlueprintAssignable, Category="GAS|Attributes")
	FOnManaChangedSignature OnManaChanged;

	UPROPERTY(BlueprintAssignable, Category="GAS|Attributes")
	FOnMaxManaChangedSignature OnMaxManaChanged;

	UPROPERTY(BlueprintAssignable, Category="GAS|Messages")
	FMessageWidgetRowSignature MessageWidgetRowDelegate;

protected:
	//EditDefaultsOnly 说明此属性可以通过属性窗口编辑，但只能在原型上进行。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Widget Data")
	TObjectPtr<UDataTable> MessageWidgetDataTable;
	
	void HealthChanged(const FOnAttributeChangeData& Data) const;
	void MaxHealthChanged(const FOnAttributeChangeData& Data) const;
	void ManaChanged(const FOnAttributeChangeData& Data) const;
	void MaxManaChanged(const FOnAttributeChangeData& Data) const;

	//根据传入的表格和Tag返回查找到的数据，表格类型不确定，所以使用T来表示，在使用此函数时，需要指定对应类型
	template<typename T>
	T* GetDataTableRowByTag(UDataTable* DataTable, const FGameplayTag& Tag);
};

template <typename T>
T* UOverlayWidgetController::GetDataTableRowByTag(UDataTable* DataTable, const FGameplayTag& Tag)
{
	T* Row = DataTable->FindRow<T>(Tag.GetTagName(), TEXT(""));
	return Row;
}
