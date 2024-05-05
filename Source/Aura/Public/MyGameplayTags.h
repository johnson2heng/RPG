// 版权归暮志未晚所有。

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

/**
 * GameplayTags 标签 单例模式
 * 内部包含原生的项目中使用的标签
 */

struct FMyGameplayTags
{
public:
	static const FMyGameplayTags& Get() { return GameplayTags; }
	static void InitializeNativeGameplayTags();

	/* Primary Attributes */
	FGameplayTag Attributes_Primary_Strength;
	FGameplayTag Attributes_Primary_Intelligence;
	FGameplayTag Attributes_Primary_Resilience;
	FGameplayTag Attributes_Primary_Vigor;

	/* Secondary Attributes */
	FGameplayTag Attributes_Secondary_MaxHealth;
	FGameplayTag Attributes_Secondary_MaxMana;
	FGameplayTag Attributes_Secondary_Armor;
	FGameplayTag Attributes_Secondary_ArmorPenetration;
	FGameplayTag Attributes_Secondary_BlockChance;
	FGameplayTag Attributes_Secondary_CriticalHitChance;
	FGameplayTag Attributes_Secondary_CriticalHitDamage;
	FGameplayTag Attributes_Secondary_CriticalHitResistance;
	FGameplayTag Attributes_Secondary_HealthRegeneration;
	FGameplayTag Attributes_Secondary_ManaRegeneration;

	FGameplayTag InputTag_LMB; //鼠标左键
	FGameplayTag InputTag_RMB; //鼠标右键
	FGameplayTag InputTag_1; //1键
	FGameplayTag InputTag_2; //2键
	FGameplayTag InputTag_3; //3键
	FGameplayTag InputTag_4; //4键

	FGameplayTag Damage; //伤害 标签
	FGameplayTag Effects_HitReact; //受击 标签
protected:

private:
	static FMyGameplayTags GameplayTags;
	
	static void InitializeAttributeGameplayTags(); //初始化属性的GameplayTag
	static void InitializeInputGameplayTags(); //初始化输入的GameplayTag
	static void InitializeGEGameplayTags(); //初始化GE或者GA里使用的GameplayTag
 
};











