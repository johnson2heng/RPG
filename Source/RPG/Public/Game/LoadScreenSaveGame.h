// 版权归暮志未晚所有。

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/SaveGame.h"
#include "LoadScreenSaveGame.generated.h"

class UGameplayAbility;
//当前存档可以显示的用户控件的枚举
UENUM(BlueprintType)
enum ESaveSlotStatus
{
	Vacant,
	EnterName,
	Taken
};

//存储技能的相关信息结构体
USTRUCT(BlueprintType)
struct FSavedAbility
{
	GENERATED_BODY()

	//需要存储的技能
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ClassDefaults")
	TSubclassOf<UGameplayAbility> GameplayAbility;

	//当前技能的等级
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int32 AbilityLevel = 0;

	//当前技能的标签
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FGameplayTag AbilityTag = FGameplayTag();

	//当前技能的状态标签
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FGameplayTag AbilityStatus = FGameplayTag();

	//当前技能装配到的插槽，如果技能未装配则为空
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FGameplayTag AbilityInputTag = FGameplayTag();

	//当前技能的类型（主动技能还是被动技能）
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FGameplayTag AbilityType = FGameplayTag();
};

//自定义运算符==，如果左右都是FSavedAbility类型的值，将通过函数内的值判断是否相等。
inline bool operator==(const FSavedAbility& Left, const FSavedAbility& Right)
{
	return Left.AbilityTag.MatchesTagExact(Right.AbilityTag);
}

/**
 * 
 */
UCLASS()
class RPG_API ULoadScreenSaveGame : public ULocalPlayerSaveGame
{
	GENERATED_BODY()

public:

	//存档名称
	UPROPERTY()
	FString SlotName = FString();

	//存档索引
	UPROPERTY()
	int32 SlotIndex = 0;

	//玩家姓名
	UPROPERTY()
	FString PlayerName = FString(L"默认名称");

	//地图名称
	UPROPERTY()
	FString MapName = FString(L"默认地图名称");

	//当前存档进入存档界面时，默认显示的用户界面
	UPROPERTY()
	TEnumAsByte<ESaveSlotStatus> SaveSlotStatus = Vacant;

	//存储玩家关卡出生位置的标签
	UPROPERTY()
	FName PlayerStartTag;

	//当前已经激活的检测点
	UPROPERTY()
	TArray<FName> ActivatedPlayerStatTags = TArray<FName>();

	//第一次加载存档
	UPROPERTY()
	bool bFirstTimeLoadIn = true;

	/************************** 玩家相关 **************************/

	//角色等级
	UPROPERTY()
	int32 PlayerLevel = 1;

	//经验值
	UPROPERTY()
	int32 XP = 0;

	//可分配技能点
	UPROPERTY()
	int32 SpellPoints = 0;

	//可分配属性点
	UPROPERTY()
	int32 AttributePoints = 0;

	/************************** 主要属性 **************************/
	
	//力量
	UPROPERTY()
	float Strength = 0;

	//智力
	UPROPERTY()
	float Intelligence = 0;

	//韧性
	UPROPERTY()
	float Resilience = 0;

	//体力
	UPROPERTY()
	float Vigor = 0;

	/************************** 技能 **************************/

	UPROPERTY()
	TArray<FSavedAbility> SavedAbilities;
};
