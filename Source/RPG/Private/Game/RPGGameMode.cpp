// 版权归暮志未晚所有。


#include "Game/RPGGameMode.h"

#include "EngineUtils.h"
#include "Checkpoints/CheckPoint.h"
#include "Game/LoadScreenSaveGame.h"
#include "Game/RPGGameInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "RPG/RPGLogChannels.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
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
	LoadScreenSaveGame->MapName = LoadSlot->GetMapName();
	LoadScreenSaveGame->SlotName = LoadSlot->GetSlotName();
	LoadScreenSaveGame->SlotIndex = SlotIndex;
	LoadScreenSaveGame->SaveSlotStatus = Taken;
	LoadScreenSaveGame->PlayerStartTag = LoadSlot->PlayerStartTag;

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

ULoadScreenSaveGame* ARPGGameMode::RetrieveInGameSaveData() const
{
	const URPGGameInstance* RPGGameInstance = Cast<URPGGameInstance>(GetGameInstance());

	//从游戏实例获取到存档名称和索引
	const FString InGameLoadSlotName = RPGGameInstance->LoadSlotName;
	const int32 InGameLoadSlotIndex = RPGGameInstance->LoadSlotIndex;

	//获取已保存的存档数据
	return GetSaveSlotData(InGameLoadSlotName, InGameLoadSlotIndex);
}

void ARPGGameMode::SaveInGameProgressData(ULoadScreenSaveGame* SaveObject) const
{
	URPGGameInstance* RPGGameInstance = Cast<URPGGameInstance>(GetGameInstance());

	//修改下一次复活的检测点
	RPGGameInstance->PlayerStartTag = SaveObject->PlayerStartTag;

	//从游戏实例获取到存档名称和索引
	const FString InGameLoadSlotName = RPGGameInstance->LoadSlotName;
	const int32 InGameLoadSlotIndex = RPGGameInstance->LoadSlotIndex;

	//保存存档
	UGameplayStatics::SaveGameToSlot(SaveObject, InGameLoadSlotName, InGameLoadSlotIndex);
}

void ARPGGameMode::SaveWorldState(UWorld* World, const FString& DestinationMapAssetName)
{
	//获取关卡名称
	FString WorldName = World->GetMapName();
	WorldName.RemoveFromStart(World->StreamingLevelsPrefix); //从关卡名称这里移除指定前缀，当前为移除通常用于标识流式加载的关卡文件前缀

	//获取到游戏实例
	URPGGameInstance* RPGGI = Cast<URPGGameInstance>(GetGameInstance());
	check(RPGGI);

	//获取存档
	if(ULoadScreenSaveGame* SaveGame = GetSaveSlotData(RPGGI->LoadSlotName, RPGGI->LoadSlotIndex))
	{
		//修改存档地图资源名称和地图名称
		if(DestinationMapAssetName != FString(""))
		{
			SaveGame->MapAssetName = DestinationMapAssetName;
			SaveGame->MapName = GetMapNameFromMapAssetName(DestinationMapAssetName);
		}
		
		if(!SaveGame->HasMap(WorldName))
		{
			//如果存档不包含对应关卡内容，将创建一个对应的数据结构体存储
			FSavedMap NewSavedMap;
			NewSavedMap.MapAssetName = WorldName;
			SaveGame->SavedMaps.Add(NewSavedMap);
		}

		//获取对应的存档关卡数据结构体
		FSavedMap SavedMap = SaveGame->GetSavedMapWithMapName(WorldName);
		SavedMap.SavedActors.Empty(); //存储的内容

		//使用迭代起，便利场景里的每一个Actor，将需要保存Actor数据保存到结构体内
		for(FActorIterator It(World); It; ++It)
		{
			AActor* Actor = *It;

			//判断Actor是否存在，并判断Actor是否需要存储
			if(!IsValid(Actor) || !Actor->Implements<USaveInterface>()) continue;

			//创建存储结构体
			FSavedActor SavedActor;
			SavedActor.ActorName = Actor->GetFName();
			SavedActor.Transform = Actor->GetTransform();

			//创建一个 FMemoryWriter，用于将数据写入SavedActor.Bytes
			FMemoryWriter MemoryWriter(SavedActor.Bytes);

			//创建一个序列化器，将对象的成员以名称和值的形式保存到 MemoryWriter。
			FObjectAndNameAsStringProxyArchive Archive(MemoryWriter, true);
			Archive.ArIsSaveGame = true; //设置序列化方式为保存到存档的模式

			//将Actor所需要保存的数据写入到Archive，Archive将把数据存储到SavedActor.Bytes
			Actor->Serialize(Archive);

			SavedMap.SavedActors.AddUnique(SavedActor);
		}

		//找到对应的名称的结构体，将数据存储到存档对象内
		for(FSavedMap& MapToReplace : SaveGame->SavedMaps)
		{
			if(MapToReplace.MapAssetName == WorldName)
			{
				MapToReplace = SavedMap;
			}
		}

		//保存存档
		UGameplayStatics::SaveGameToSlot(SaveGame, RPGGI->LoadSlotName, RPGGI->LoadSlotIndex);
	}
}

void ARPGGameMode::LoadWorldState(UWorld* World) const
{
	//获取关卡名称
	FString WorldName = World->GetMapName();
	WorldName.RemoveFromStart(World->StreamingLevelsPrefix); //从关卡名称这里移除指定前缀，当前为移除通常用于标识流式加载的关卡文件前缀
	
	//获取到游戏实例
	URPGGameInstance* RPGGI = Cast<URPGGameInstance>(GetGameInstance());
	check(RPGGI);

	//判断获取的存档是否存在
	if(UGameplayStatics::DoesSaveGameExist(RPGGI->LoadSlotName, RPGGI->LoadSlotIndex))
	{
		//获取存档
		ULoadScreenSaveGame* SaveGame = Cast<ULoadScreenSaveGame>(UGameplayStatics::LoadGameFromSlot(RPGGI->LoadSlotName, RPGGI->LoadSlotIndex));
		if(SaveGame == nullptr)
		{
			UE_LOG(LogRPG, Error, TEXT("加载对应存档失败"));
		}

		//判断存档是否含有对应关卡的数据
		if(SaveGame->HasMap(WorldName))
		{
			//获取到关卡内的所有存档数据
			TArray<FSavedActor> SavedActors = SaveGame->GetSavedMapWithMapName(WorldName).SavedActors;
			
			//遍历场景内的所有Actor，寻找存档内对应的数据并应用到场景
			for(FActorIterator It(World); It; ++It)
			{
				AActor* Actor = *It;

				if(!Actor->Implements<USaveInterface>()) continue;

				//遍历存档里对应关卡的所有actor数据
				for(FSavedActor SavedActor : SavedActors)
				{
					//查找到对应的actor的存档数据
					if(SavedActor.ActorName == Actor->GetFName())
					{
						//判断当前Actor是否需要设置位置变换
						if(ISaveInterface::Execute_ShouldLoadTransform(Actor))
						{
							Actor->SetActorTransform(SavedActor.Transform);
						}

						//反序列化，创建一个FMemoryReader实例用于从二进制数据中读取内容
						FMemoryReader MemoryReader(SavedActor.Bytes);

						//FObjectAndNameAsStringProxyArchive 代理类，用于序列化和反序列化对象的属性 true：表示允许使用字符串形式的对象和属性名称（便于调试和可读性）。
						FObjectAndNameAsStringProxyArchive Archive(MemoryReader, true);
						Archive.ArIsSaveGame = true; //指定反序列化是用于加载存档数据。
						Actor->Serialize(Archive); //执行反序列化，将二进制数据设置到actor属性上

						//修改Actor上的属性后，调用函数更新Actor的显示
						ISaveInterface::Execute_LoadActor(Actor);
					}
				}
			}			
		}
	}
}

void ARPGGameMode::TravelToMap(const UMVVM_LoadSlot* Slot)
{
	const FString SlotName = Slot->GetSlotName();
	const int32 SlotIndex = Slot->SlotIndex;
	
	//打开地图
	UGameplayStatics::OpenLevelBySoftObjectPtr(Slot, Maps.FindChecked(Slot->GetMapName()));
}

FString ARPGGameMode::GetMapNameFromMapAssetName(const FString& MapAssetName)
{
	for(auto& Map : Maps)
	{
		if(Map.Value.ToSoftObjectPath().GetAssetName() == MapAssetName)
		{
			return Map.Key;
		}
	}
	return FString();
}

AActor* ARPGGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	const URPGGameInstance* RPGGameInstance = Cast<URPGGameInstance>(GetGameInstance());
	
	//获取关卡里的所有PlayerStart实例
	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), Actors);

	//高亮已激活的检查点
	// HighlightEnabledCheckpoints(Actors);
	
	if(Actors.Num() > 0)
	{
		//获取到第一个实例对象
		AActor* SelectedActor = Actors[0];
		for(AActor* Actor : Actors)
		{
			if(APlayerStart* PlayerStart = Cast<APlayerStart>(Actor))
			{
				//判断PlayerStart的Tag设置是否为指定的Tag
				if(PlayerStart->PlayerStartTag == RPGGameInstance->PlayerStartTag)
				{
					SelectedActor = PlayerStart;
					break;
				}
			}
		}
		return SelectedActor;
	}
	return nullptr;
}

void ARPGGameMode::PlayerDied(const ACharacter* DeadCharacter) const
{
	//获取存档数据
	const ULoadScreenSaveGame* SaveGame = RetrieveInGameSaveData();
	if(!IsValid(SaveGame)) return;

	//通过地图命名打开地图
	UGameplayStatics::OpenLevelBySoftObjectPtr(DeadCharacter, Maps.FindChecked(SaveGame->MapName));
}

void ARPGGameMode::BeginPlay()
{
	Super::BeginPlay();

	Maps.Add(DefaultMapName, DefaultMap);
}

void ARPGGameMode::HighlightEnabledCheckpoints(TArray<AActor*> CheckPoints) const
{
	//获取存档
	ULoadScreenSaveGame* SaveGameData = RetrieveInGameSaveData();
	if(SaveGameData == nullptr) return;

	//遍历关卡内的所有的检查点，如果数组里存在，将高亮显示
	for(AActor* Actor : CheckPoints)
	{
		if(ACheckPoint* CheckPoint = Cast<ACheckPoint>(Actor))
		{
			if(SaveGameData->ActivatedPlayerStatTags.Contains(CheckPoint->PlayerStartTag))
			{
				CheckPoint->HandleGlowEffects();
			}
		}
	}
}
