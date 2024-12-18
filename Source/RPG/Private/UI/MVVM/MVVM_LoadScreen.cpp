// 版权归暮志未晚所有。


#include "UI/MVVM/MVVM_LoadScreen.h"

#include "Game/RPGGameInstance.h"
#include "Game/RPGGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "UI/MVVM/MVVM_LoadSlot.h"

void UMVVM_LoadScreen::InitializeLoadSlots()
{
	LoadSlot_0 = NewObject<UMVVM_LoadSlot>(this, LoadSlotViewModelClass);
	LoadSlot_0->SetSlotName("LoadSlot_0");
	LoadSlots.Add(0, LoadSlot_0);
	LoadSlot_1 = NewObject<UMVVM_LoadSlot>(this, LoadSlotViewModelClass);
	LoadSlot_1->SetSlotName("LoadSlot_1");
	LoadSlots.Add(1, LoadSlot_1);
	LoadSlot_2 = NewObject<UMVVM_LoadSlot>(this, LoadSlotViewModelClass);
	LoadSlot_2->SetSlotName("LoadSlot_2");
	LoadSlots.Add(2, LoadSlot_2);
}

UMVVM_LoadSlot* UMVVM_LoadScreen::GetLoadSlotViewModelByIndex(const int32 Index) const
{
	return LoadSlots.FindChecked(Index);
}

void UMVVM_LoadScreen::NewSlotButtonPressed(const int32 Slot, const FString& EnterName)
{
	const ARPGGameMode* RPGGameMode = Cast<ARPGGameMode>(UGameplayStatics::GetGameMode(this));

	LoadSlots[Slot]->SetMapName(RPGGameMode->DefaultMapName); //设置地图名称
	LoadSlots[Slot]->SetPlayerName(EnterName); //修改MVVM上存储的角色名称
	LoadSlots[Slot]->LoadSlotStatus = Taken; //修改进入界面为加载界面
	LoadSlots[Slot]->SetPlayerLevel(1); //修改角色等级
	LoadSlots[Slot]->PlayerStartTag = RPGGameMode->DefaultPlayerStartTag; //存储关卡生成点
	
	RPGGameMode->SaveSlotData(LoadSlots[Slot], Slot); //保存数据
	LoadSlots[Slot]->InitializeSlot(); //调用初始化
}

void UMVVM_LoadScreen::NewGameButtonPressed(const int32 Slot)
{
	LoadSlots[Slot]->SetWidgetSwitcherIndex.Broadcast(1);
}

void UMVVM_LoadScreen::SelectSlotButtonPressed(int32 Slot)
{
	SlotSelected.Broadcast(true);
	for(const TTuple<int32, UMVVM_LoadSlot*> LoadSlot : LoadSlots)
	{
		//广播设置
		LoadSlot.Value->EnableSelectSlotButton.Broadcast(LoadSlot.Key != Slot);
	}
}

void UMVVM_LoadScreen::EnterGameButtonPressed(const int32 Slot)
{
	ARPGGameMode* RPGGameMode = Cast<ARPGGameMode>(UGameplayStatics::GetGameMode(this));

	//设置全局数据，方便后续使用
	URPGGameInstance* RPGGameInstance = Cast<URPGGameInstance>(RPGGameMode->GetGameInstance());
	RPGGameInstance->LoadSlotName = LoadSlots[Slot]->GetSlotName();
	RPGGameInstance->LoadSlotIndex = LoadSlots[Slot]->SlotIndex;
	RPGGameInstance->PlayerStartTag = LoadSlots[Slot]->PlayerStartTag;

	//进入场景
	RPGGameMode->TravelToMap(LoadSlots[Slot]);
}

void UMVVM_LoadScreen::DeleteButtonPressed(const int32 Slot)
{
	//删除存档
	const ARPGGameMode* RPGGameMode = Cast<ARPGGameMode>(UGameplayStatics::GetGameMode(this));
	RPGGameMode->DeleteSlotData(LoadSlots[Slot]->GetSlotName(), Slot);

	//修改用户控件显示
	LoadSlots[Slot]->LoadSlotStatus = Vacant; //修改为创建存档
	LoadSlots[Slot]->InitializeSlot(); //修改存档显示
}

void UMVVM_LoadScreen::LoadData()
{
	//获取到加载存档界面的GameMode
	const ARPGGameMode* RPGGameMode = Cast<ARPGGameMode>(UGameplayStatics::GetGameMode(this));

	//遍历映射，获取对应存档
	for(const TTuple<int32, UMVVM_LoadSlot*> Slot : LoadSlots)
	{
		const ULoadScreenSaveGame* SaveGame = RPGGameMode->GetSaveSlotData(Slot.Value->GetSlotName(), Slot.Key);

		//获取存档数据
		const FString PlayerName = SaveGame->PlayerName;
		const TEnumAsByte<ESaveSlotStatus> SaveSlotStatus = SaveGame->SaveSlotStatus;

		//设置存档视图模型数据
		Slot.Value->SetPlayerName(PlayerName);
		Slot.Value->SetMapName(SaveGame->MapName);
		Slot.Value->SetPlayerLevel(SaveGame->PlayerLevel);
		Slot.Value->LoadSlotStatus = SaveSlotStatus;
		Slot.Value->PlayerStartTag = SaveGame->PlayerStartTag;

		//调用视图模型初始化
		Slot.Value->InitializeSlot();
	}
}

void UMVVM_LoadScreen::SetWidgetName(const FString& InSlotName)
{
	if (UE_MVVM_SET_PROPERTY_VALUE(WidgetName, InSlotName))
	{
		// UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(GetHealthPercent); //通过宏调用其它函数的广播
	}
}
