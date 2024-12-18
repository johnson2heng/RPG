// 版权归暮志未晚所有。

#pragma once

#include "CoreMinimal.h"
#include "LoadScreenSaveGame.h"
#include "GameFramework/GameModeBase.h"
#include "RPGGameMode.generated.h"

class ULootTiers;
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

	//战利品数据配置
	UPROPERTY(EditDefaultsOnly, Category="Loot Tiers")
	TObjectPtr<ULootTiers> LootTiers;
	
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

	//获取到当前游戏进行中所使用的存档数据
	ULoadScreenSaveGame* RetrieveInGameSaveData() const;

	/**
	 * 保存游戏中的进度
	 * @param SaveObject 需要保存的数据
	 */
	void SaveInGameProgressData(ULoadScreenSaveGame* SaveObject) const;

	/**
	 * 保存关卡中的状态到当前存档中
	 * @param World 当前所在世界
	 * @param DestinationMapAssetName 地图资源名称
	 */
	void SaveWorldState(UWorld* World, const FString& DestinationMapAssetName = FString(""));
	//从存档中加载当前关卡的状态
	void LoadWorldState(UWorld* World) const;
	
	//通过存档进入地图
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

	//角色切换关卡后默认生成位置的PlayerStart的标签
	UPROPERTY(EditDefaultsOnly)
	FName DefaultPlayerStartTag;

	//地图名称和地图的映射
	UPROPERTY(EditDefaultsOnly)
	TMap<FString, TSoftObjectPtr<UWorld>> Maps;

	//根据地图名称获取地图资源路径
	FString GetMapNameFromMapAssetName(const FString& MapAssetName);

	/**
	 * 覆写GameMode基类的获取关卡中的PlayerStart，修改为可以通过Tag获取生成位置
	 * @param Player 需要获取的控制器
	 * @return 
	 */
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	/**
	 * 角色死亡调用函数
	 * @param DeadCharacter 玩家角色实例
	 */
	void PlayerDied(const ACharacter* DeadCharacter) const;
	
protected:
	
	virtual void BeginPlay() override;

private:
	
	//高亮已经激活的检查点
	void HighlightEnabledCheckpoints(TArray<AActor*> CheckPoints) const;
};
