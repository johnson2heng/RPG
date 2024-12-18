// 版权归暮志未晚所有。

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "MVVM_LoadScreen.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSlotSelected, bool, bSelected); //选中存档委托定义

class UMVVM_LoadSlot;
/**
 * 
 */
UCLASS()
class RPG_API UMVVM_LoadScreen : public UMVVMViewModelBase
{
	GENERATED_BODY()
	
public:
	
	void InitializeLoadSlots();

	//选中存档后的委托
	UPROPERTY(BlueprintAssignable)
	FSlotSelected SlotSelected;

	//每个存档插槽使用的MVVM类
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UMVVM_LoadSlot> LoadSlotViewModelClass;

	UFUNCTION(BlueprintPure)
	UMVVM_LoadSlot* GetLoadSlotViewModelByIndex(int32 Index) const;

	/**
	 * 创建新存档按下事件
	 * @param Slot 当前存档的位置，以索引代表
	 * @param EnterName 输入的存档名称
	 */
	UFUNCTION(BlueprintCallable)
	void NewSlotButtonPressed(int32 Slot, const FString& EnterName);

	/**
	 * 开始新游戏按下事件，进入创建新存档
	 * @param Slot 点击的按钮的存档位置
	 */
	UFUNCTION(BlueprintCallable)
	void NewGameButtonPressed(int32 Slot);

	/**
	 * 选择存档按下事件，触发进入游戏
	 * @param Slot 点击的存档的位置，以索引来代表
	 */
	UFUNCTION(BlueprintCallable)
	void SelectSlotButtonPressed(int32 Slot);

	/**
	 * 进入游戏按钮按下事件
	 * @param Slot 点击存档的位置，以索引来代表
	 */
	UFUNCTION(BlueprintCallable)
	void EnterGameButtonPressed(int32 Slot);

	/**
	 * 删除存档
	 * @param Slot 点击存档的位置，以索引来代表
	 */
	UFUNCTION(BlueprintCallable)
	void DeleteButtonPressed(int32 Slot);

	//从存档读取数据，并设置每个插槽
	void LoadData();
	
	void SetWidgetName(const FString& InSlotName);
	FString GetWidgetName() const { return WidgetName; };
	
private:

	//用户控件的名称
	UPROPERTY(BlueprintReadOnly, FieldNotify, Setter, Getter, meta=(AllowPrivateAccess))
	FString WidgetName;

	//索引和对应MVVM实例的映射
	UPROPERTY()
	TMap<int32, UMVVM_LoadSlot*> LoadSlots;

	//对象对MVVM实例的引用，防止垃圾回收机制对其进行回收
	UPROPERTY()
	TObjectPtr<UMVVM_LoadSlot> LoadSlot_0;
	
	UPROPERTY()
	TObjectPtr<UMVVM_LoadSlot> LoadSlot_1;
	
	UPROPERTY()
	TObjectPtr<UMVVM_LoadSlot> LoadSlot_2;
};
