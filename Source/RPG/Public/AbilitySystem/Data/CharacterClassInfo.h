// 版权归暮志未晚所有。

#pragma once

#include "CoreMinimal.h"
#include "ScalableFloat.h"
#include "Engine/DataAsset.h"
#include "CharacterClassInfo.generated.h"

class UGameplayAbility;
class UGameplayEffect;

//角色职业类型的枚举
UENUM(BlueprintType)
enum class ECharacterClass : uint8
{
	Elementalist, //法师
	Warrior, //战士
	Ranger //游侠
};

//对应每个职业的属性和技能
USTRUCT()
struct FCharacterClassDefaultInfo
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category="Class Defaults")
	TSubclassOf<UGameplayEffect> PrimaryAttributes;

	UPROPERTY(EditDefaultsOnly, Category="Class Defaults")
	FScalableFloat XPReward = FScalableFloat();

	UPROPERTY(EditDefaultsOnly, Category="Class Defaults")
	TArray<TSubclassOf<UGameplayAbility>> StartupAbilities;
};

/**
 * 根据职业选择初始化角色的数据
 */
UCLASS()
class RPG_API UCharacterClassInfo : public UDataAsset
{
	GENERATED_BODY()

public:

	//不同职业对应的基础数值和技能
	UPROPERTY(EditDefaultsOnly, Category="Class Defaults")
	TMap<ECharacterClass, FCharacterClassDefaultInfo> CharacterClassInformation;

	//主要属性，玩家的基础属性，通过SetByCaller设置
	UPROPERTY(EditDefaultsOnly, Category="Common Class Defaults")
	TSubclassOf<UGameplayEffect> PrimaryAttributes_SetByCaller;

	//次级属性，根据主要属性去生成的数值的属性
	UPROPERTY(EditDefaultsOnly, Category="Common Class Defaults")
	TSubclassOf<UGameplayEffect> SecondaryAttributes;

	//额外属性，自动设置的额外的一些属性
	UPROPERTY(EditDefaultsOnly, Category="Common Class Defaults")
	TSubclassOf<UGameplayEffect> VitalAttributes;

	//每个角色都可以拥有的技能
	UPROPERTY(EditDefaultsOnly, Category="Common Class Defaults")
	TArray<TSubclassOf<UGameplayAbility>> CommonAbilities; 

	//角色随着等级改变的数值影响的系数
	UPROPERTY(EditDefaultsOnly, Category="Common Class Defaults|Damgage")
	TObjectPtr<UCurveTable> DamageCalculationCoefficients; 

	//通过枚举获取对应的初始化类
	FCharacterClassDefaultInfo GetClassDefaultInfo(ECharacterClass CharacterClass);
};
