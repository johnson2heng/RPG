// 版权归暮志未晚所有。

#pragma once

#include "CoreMinimal.h"
#include "LoadScreenSaveGame.h"
#include "GameFramework/GameModeBase.h"
#include "RPGGameMode.generated.h"

class USaveGame;
class UMVVM_LoadSlot;
class UAbilityInfo;
class UCharacterClassInfo;
/**
 * 游戏模式基类
 */
UCLASS()
class RPG_API ARPGGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:

	//敌人的资产数据配置
	UPROPERTY(EditDefaultsOnly, Category="Character Class Defaults")
	TObjectPtr<UCharacterClassInfo> CharacterClassInfo;

	//玩家技能数据配置
	UPROPERTY(EditDefaultsOnly, Category="Ability Info")
	TObjectPtr<UAbilityInfo> AbilityInfo;

	/**
	 * 创建新存档
	 * @param LoadSlot 需要保存的视图模型实例
	 * @param SlotIndex 存档索引
	 */
	void SaveSlotData(const UMVVM_LoadSlot* LoadSlot, int32 SlotIndex) const;

	/**
	 * 删除存档
	 * @param SlotName 需要删除存档对应的视图模型实例名称
	 * @param SlotIndex 存档索引
	 */
	static void DeleteSlotData(const FString& SlotName, int32 SlotIndex);

	/**
	 * 获取保存的存档
	 * @param SlotName 存档名称（每个存档名称固定）
	 * @param SlotIndex 存档索引
	 * @return 
	 */
	ULoadScreenSaveGame* GetSaveSlotData(const FString& SlotName, int32 SlotIndex) const;

	void TravelToMap(const UMVVM_LoadSlot* Slot);

	//存档使用的数据结构
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<USaveGame> LoadScreenSaveGameClass;

	//初始地图名称
	UPROPERTY(EditDefaultsOnly)
	FString DefaultMapName;

	//游戏初始地图
	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UWorld> DefaultMap;

	//地图名称和地图的映射
	UPROPERTY(EditDefaultsOnly)
	TMap<FString, TSoftObjectPtr<UWorld>> Maps;

protected:
	virtual void BeginPlay() override;
};
