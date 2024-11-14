// 版权归暮志未晚所有。


#include "Game/RPGGameMode.h"

#include "Game/LoadScreenSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "UI/MVVM/MVVM_LoadSlot.h"

void ARPGGameMode::SaveSlotData(const UMVVM_LoadSlot* LoadSlot, const int32 SlotIndex) const
{
	//检查是否有对应名称的存档
	if(UGameplayStatics::DoesSaveGameExist(LoadSlot->GetSlotName(), SlotIndex))
	{
		//删除已保存的存档
		UGameplayStatics::DeleteGameInSlot(LoadSlot->GetSlotName(), SlotIndex);
	}
	
	//创建一个新的存档
	USaveGame* SaveGameObject = UGameplayStatics::CreateSaveGameObject(LoadScreenSaveGameClass);
	ULoadScreenSaveGame* LoadScreenSaveGame = Cast<ULoadScreenSaveGame>(SaveGameObject);

	//设置需要保存的数据
	LoadScreenSaveGame->PlayerName = LoadSlot->GetPlayerName();
	LoadScreenSaveGame->SlotName = LoadSlot->GetSlotName();
	LoadScreenSaveGame->SlotIndex = SlotIndex;
	LoadScreenSaveGame->SaveSlotStatus = Taken;

	//保存存档
	UGameplayStatics::SaveGameToSlot(LoadScreenSaveGame, LoadSlot->GetSlotName(), SlotIndex);
}

void ARPGGameMode::DeleteSlotData(const FString& SlotName, const int32 SlotIndex)
{
	//检查是否有对应名称的存档
	if(UGameplayStatics::DoesSaveGameExist(SlotName, SlotIndex))
	{
		//删除已保存的存档
		UGameplayStatics::DeleteGameInSlot(SlotName, SlotIndex);
	}
}

ULoadScreenSaveGame* ARPGGameMode::GetSaveSlotData(const FString& SlotName, int32 SlotIndex) const
{
	USaveGame* SaveGameObject;
	//检查是否有对应名称的存档
	if(UGameplayStatics::DoesSaveGameExist(SlotName, SlotIndex))
	{
		//获取存档
		SaveGameObject = UGameplayStatics::LoadGameFromSlot(SlotName, SlotIndex);
	}
	else
	{
		//创建新存档
		SaveGameObject = UGameplayStatics::CreateSaveGameObject(LoadScreenSaveGameClass);
	}

	//转换类型
	ULoadScreenSaveGame* LoadScreenSaveGame = Cast<ULoadScreenSaveGame>(SaveGameObject);

	return LoadScreenSaveGame;
}
