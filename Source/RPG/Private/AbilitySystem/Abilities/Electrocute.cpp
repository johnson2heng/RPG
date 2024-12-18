// 版权归暮志未晚所有。


#include "AbilitySystem/Abilities/Electrocute.h"

#include "RPGGameplayTags.h"

FString UElectrocute::GetDescription(int32 Level)
{
	return GetDescriptionAtLevel(Level, L"闪电弧");
}

FString UElectrocute::GetNextLevelDescription(int32 Level)
{
	return GetDescriptionAtLevel(Level, L"下一等级");
}

FString UElectrocute::GetDescriptionAtLevel(int32 Level, const FString& Title)
{
	const int32 Damage = GetDamageByDamageType(Level, FRPGGameplayTags::Get().Damage_Lightning);
	const float ManaCost = GetManaCost(Level);
	const float Cooldown = GetCooldown(Level);
	
	return FString::Printf(TEXT(
		// 标题
		"<Title>%s</>\n"

		// 细节
		"<Small>等级：</> <Level>%i</>\n"
		"<Small>技能冷却：</> <Cooldown>%.1f</>\n"
		"<Small>蓝量消耗：</> <ManaCost>%.1f</>\n\n"//%.1f会四舍五入到小数点后一位

		// 技能描述
		"<Default>使用闪电攻击敌人，并在命中敌人后，向外扩散 %i 条闪电链，造成</> <Damage>%i</> <Default>点雷电伤害，在技能结束时对敌人造成</><Time>%i</><Default>秒的眩晕效果。</>"),

		// 动态修改值
		*Title,
		Level,
		Cooldown,
		-ManaCost,
		FMath::Min(Level, MaxNumShockTargets - 1),
		Damage,
		static_cast<int>(DeBuffDuration));
}
