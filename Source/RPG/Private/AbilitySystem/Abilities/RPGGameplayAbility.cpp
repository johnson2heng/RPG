// 版权归暮志未晚所有。


#include "AbilitySystem/Abilities/RPGGameplayAbility.h"

FString URPGGameplayAbility::GetDescription(int32 Level)
{
	return FString::Printf(TEXT("%s, <Level>%d</>"), L"默认技能名称 - 火球术火球术火球术火球术", Level);
}

FString URPGGameplayAbility::GetNextLevelDescription(int32 Level)
{
	return FString::Printf(TEXT("下一等级：<Level>%d</> 造成更多的伤害。"), Level);
}

FString URPGGameplayAbility::GetLockedDescription(int32 Level)
{
	return FString::Printf(TEXT("技能将在角色等级达到<Level>%d</>时解锁"), Level);
}
