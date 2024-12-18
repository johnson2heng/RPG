// 版权归暮志未晚所有。

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/RPGBeamSpell.h"
#include "Electrocute.generated.h"

/**
 * 
 */
UCLASS()
class RPG_API UElectrocute : public URPGBeamSpell
{
	GENERATED_BODY()
	
public:
	
	/**
	 * 获取当前等级技能描述
	 * @param Level 技能等级
	 * @return 技能描述富文本
	 */
	virtual FString GetDescription(int32 Level) override;

	/**
	 * 获取下一等级技能描述
	 * @param Level 下一等级
	 * @return 技能描述附文本
	 */
	virtual FString GetNextLevelDescription(int32 Level) override;

	/**
	 * 当前技能的技能描述，将共用部分抽离出来为单个函数
	 * @param Level 显示的技能等级
	 * @param Title 技能标题
	 * @return 技能描述文本
	 */
	virtual FString GetDescriptionAtLevel(int32 Level, const FString& Title) override;
	
};
