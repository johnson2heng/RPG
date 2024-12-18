// 版权归暮志未晚所有。

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "Game/LoadScreenSaveGame.h"
#include "MVVM_LoadSlot.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSetWidgetSwitcherIndex, int32, WidgetSwitecherIndex); //当前对应的用户控件修改委托定义
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEnableSelectSlotButton, bool, bEnable); //当前存档是否选中委托定义

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

	//当前存档选中状态切换委托
	UPROPERTY(BlueprintAssignable)
	FEnableSelectSlotButton EnableSelectSlotButton;

	void InitializeSlot() const;

	//当前视图模型的索引，对应存档的索引
	UPROPERTY(BlueprintReadWrite)
	int32 SlotIndex;

	//存储玩家关卡出生位置的标签
	UPROPERTY()
	FName PlayerStartTag;

	//当前进入加载存档界面时，此存档应该显示的用户控件界面。
	UPROPERTY()
	TEnumAsByte<ESaveSlotStatus> LoadSlotStatus;

	void SetSlotName(const FString& InSlotName);
	FString GetSlotName() const { return SlotName; };

	void SetPlayerName(const FString& InPlayerName);
	FString GetPlayerName() const { return PlayerName; };

	void SetPlayerLevel(const int32 InPlayerLevel);
	int32 GetPlayerLevel() const { return PlayerLevel; };

	void SetMapName(const FString& InMapName);
	FString GetMapName() const { return MapName; };
	
private:
	
	//用户控件的名称
	UPROPERTY(BlueprintReadOnly, FieldNotify, Setter, Getter, meta=(AllowPrivateAccess))
	FString SlotName;

	//用户设置的角色名称
	UPROPERTY(BlueprintReadOnly, FieldNotify, Setter, Getter, meta=(AllowPrivateAccess)) //meta=(AllowPrivateAccess)允许设置私有，但在蓝图公开
	FString PlayerName;

	//角色的等级
	UPROPERTY(BlueprintReadOnly, FieldNotify, Setter, Getter, meta=(AllowPrivateAccess)) //meta=(AllowPrivateAccess)允许设置私有，但在蓝图公开
	int32 PlayerLevel;

	//当前角色所在的地图名称
	UPROPERTY(BlueprintReadOnly, FieldNotify, Setter, Getter, meta=(AllowPrivateAccess)) //meta=(AllowPrivateAccess)允许设置私有，但在蓝图公开
	FString MapName;
};
