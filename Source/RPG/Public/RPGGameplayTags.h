// 版权归暮志未晚所有。

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

/**
 * GameplayTags 标签 单例模式
 * 内部包含原生的项目中使用的标签
 */

struct FRPGGameplayTags
{
public:
	static const FRPGGameplayTags& Get() { return GameplayTags; }
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

	//元属性
	FGameplayTag Attributes_Meta_IncomingXP; //元属性经验 标签

	FGameplayTag InputTag_LMB; //鼠标左键
	FGameplayTag InputTag_RMB; //鼠标右键
	FGameplayTag InputTag_1; //1键
	FGameplayTag InputTag_2; //2键
	FGameplayTag InputTag_3; //3键
	FGameplayTag InputTag_4; //4键
	FGameplayTag InputTag_Passive_1; //被动技能1
	FGameplayTag InputTag_Passive_2; //被动技能2

	//属性伤害抗性
	FGameplayTag Attributes_Resistance_Fire; //火属性伤害抵抗 标签
	FGameplayTag Attributes_Resistance_Lightning; //雷属性伤害抵抗 标签
	FGameplayTag Attributes_Resistance_Arcane; //魔法伤害抵抗 标签
	FGameplayTag Attributes_Resistance_Physical; //物理伤害抵抗 标签

	FGameplayTag Damage; //伤害 标签
	FGameplayTag Damage_Fire; //火属性伤害 标签
	FGameplayTag Damage_Lightning; //雷属性伤害 标签
	FGameplayTag Damage_Arcane; //魔法伤害 标签
	FGameplayTag Damage_Physical; //物理伤害 标签

	FGameplayTag DeBuff_Burn; //火属性负面效果 燃烧
	FGameplayTag DeBuff_Stun; //雷属性负面效果 眩晕
	FGameplayTag DeBuff_Arcane; //魔法伤害负面效果
	FGameplayTag DeBuff_Physical; //物理伤害负面效果 流血

	FGameplayTag DeBuff_Chance; //负面效果触发几率标签
	FGameplayTag DeBuff_Damage; //负面效果伤害标签
	FGameplayTag DeBuff_Duration; //负面效果持续时间标签
	FGameplayTag DeBuff_Frequency; //负面效果触发间隔标签

	FGameplayTag Abilities_None; //空技能标签
	FGameplayTag Abilities_Attack; //攻击技能激活标签
	FGameplayTag Abilities_Summon; //召唤技能激活标签
	
	FGameplayTag Abilities_Fire_FireBolt; //火球术技能标签
	FGameplayTag Abilities_Fire_FireBlast; //火焰爆发技能标签
	FGameplayTag Abilities_Lightning_Electrocute; //闪电链技能标签
	FGameplayTag Abilities_Arcane_ArcaneShards; //奥数爆发技能标签
	
	FGameplayTag Abilities_HitReact; //受击技能标签
	FGameplayTag Abilities_Stunned; //眩晕技能标签，在被应用眩晕状态时，自动激活

	FGameplayTag Abilities_Status_Locked; //技能状态 已锁定
	FGameplayTag Abilities_Status_Eligible; //技能状态 可解锁
	FGameplayTag Abilities_Status_Unlocked; //技能状态 已解锁
	FGameplayTag Abilities_Status_Equipped; //技能状态 已装配
	
	FGameplayTag Abilities_Type_Offensive; //技能类型 主动技能
	FGameplayTag Abilities_Type_Passive; //技能类型 被动技能
	FGameplayTag Abilities_Type_None; //技能类型 空 受击等技能设置

	FGameplayTag Abilities_Passive_HaloOfProtection; //被动技能-守护光环
	FGameplayTag Abilities_Passive_LifeSiphon; //被动技能-生命回复
	FGameplayTag Abilities_Passive_ManaSiphon; //被动技能-蓝量回复
	
	FGameplayTag Cooldown_Fire_FireBolt; //火球术冷却标签
	
	FGameplayTag CombatSocket_Weapon; //使用武器攻击部位标签
	FGameplayTag CombatSocket_RightHand; //右手攻击部位标签
	FGameplayTag CombatSocket_LeftHand; //左手攻击部位标签
	FGameplayTag CombatSocket_Tail; //尾巴攻击部位标签

	//使用攻击动作索引
	FGameplayTag Montage_Attack_1; 
	FGameplayTag Montage_Attack_2;
	FGameplayTag Montage_Attack_3;
	FGameplayTag Montage_Attack_4;

	TMap<FGameplayTag, FGameplayTag> DamageTypesToResistance; //属性伤害标签对应属性抵抗标签
	TMap<FGameplayTag, FGameplayTag> DeBuffsToResistance; //负面标签对应属性抵抗标签
	
	FGameplayTag Effects_HitReact; //受击 标签

	//阻止输入相关事件触发
	FGameplayTag Player_Block_InputPressed; //阻挡键位按下输入
	FGameplayTag Player_Block_InputHold; //阻挡键位悬停输入
	FGameplayTag Player_Block_InputReleased; //阻挡键位抬起输入
	FGameplayTag Player_Block_CursorTrace; //阻挡鼠标拾取事件

	FGameplayTag GameplayCue_FireBlast; //火焰爆炸火球爆炸时的表现效果

private:
	static FRPGGameplayTags GameplayTags;
	
	static void InitializeAttributeGameplayTags(); //初始化属性的GameplayTag
	static void InitializeInputGameplayTags(); //初始化输入的GameplayTag
	static void InitializeGAGameplayTags(); //初始化GA里使用的GameplayTag
	static void InitializeGEGameplayTags(); //初始化GE里使用的GameplayTag
 
};











