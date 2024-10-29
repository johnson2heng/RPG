// 版权归暮志未晚所有。


#include "AbilitySystem/Abilities/RPGFireBlast.h"

#include "RPGGameplayTags.h"

FString URPGFireBlast::GetDescription(const int32 Level)
{
	return GetDescriptionAtLevel(Level, L"火焰爆炸");
}

FString URPGFireBlast::GetNextLevelDescription(const int32 Level)
{
	return GetDescriptionAtLevel(Level, L"下一等级");
}

FString URPGFireBlast::GetDescriptionAtLevel(const int32 Level, const FString& Title)
{
	const int32 Damage = GetDamageByDamageType(Level, FRPGGameplayTags::Get().Damage_Fire);
	const float ManaCost = GetManaCost(Level);
	const float Cooldown = GetCooldown(Level);
	
	return FString::Printf(
		TEXT(
		// 标题
		"<Title>%s</>\n"

		// 细节
		"<Small>等级：</> <Level>%i</>\n"
		"<Small>技能冷却：</> <Cooldown>%.1f</>\n"
		"<Small>蓝量消耗：</> <ManaCost>%.1f</>\n\n"//%.1f会四舍五入到小数点后一位

		// 技能描述
		"<Default>向四面八方发射 %i 颗火球，每颗火球会在返回时发生爆炸，并造成</> <Damage>%i</> <Default>点径向火焰伤害，并有一定几率触发燃烧。</>"),

		// 动态修改值
		*Title,
		Level,
		Cooldown,
		-ManaCost,
		NumFireBalls,
		Damage
	);
}