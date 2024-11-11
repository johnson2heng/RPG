// 版权归暮志未晚所有。

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "MVVM_LoadSlot.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSetWidgetSwitcherIndex, int32, WidgetSwitecherIndex);

/**
 * 
 */
UCLASS()
class RPG_API UMVVM_LoadSlot : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:

	//切换存档显示的用户控件的委托
	UPROPERTY(BlueprintAssignable)
	FSetWidgetSwitcherIndex SetWidgetSwitcherIndex;

	void InitializeSlot() const;

	void SetSlotName(const FString& InSlotName);
	FString GetSlotName() const { return SlotName; };
private:
	
	//用户控件的名称
	UPROPERTY(BlueprintReadOnly, FieldNotify, Setter, Getter, meta=(AllowPrivateAccess))
	FString SlotName;
};
