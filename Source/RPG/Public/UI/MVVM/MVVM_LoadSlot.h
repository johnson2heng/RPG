// 版权归暮志未晚所有。

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "Game/LoadScreenSaveGame.h"
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

	//当前视图模型的索引，对应存档的索引
	UPROPERTY()
	int32 SlotIndex;

	//当前进入加载存档界面时，此存档应该显示的用户控件界面。
	UPROPERTY()
	TEnumAsByte<ESaveSlotStatus> LoadSlotStatus;

	void SetSlotName(const FString& InSlotName);
	FString GetSlotName() const { return SlotName; };

	void SetPlayerName(const FString& InPlayerName);
	FString GetPlayerName() const { return PlayerName; };
private:
	
	//用户控件的名称
	UPROPERTY(BlueprintReadOnly, FieldNotify, Setter, Getter, meta=(AllowPrivateAccess))
	FString SlotName;

	//用户设置的角色名称
	UPROPERTY(BlueprintReadOnly, FieldNotify, Setter, Getter, meta=(AllowPrivateAccess))
	FString PlayerName;
};
