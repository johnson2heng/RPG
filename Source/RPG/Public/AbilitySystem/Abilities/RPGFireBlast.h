// 版权归暮志未晚所有。

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/RPGDamageGameplayAbility.h"
#include "RPGFireBlast.generated.h"

class ARPGFireBall;
/**
 * 火焰爆发技能类
 */
UCLASS()
class RPG_API URPGFireBlast : public URPGDamageGameplayAbility
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

	/**
	 * 生成技能所需的火球
	 * @return NumFireBalls个数火球的数组
	 */
	UFUNCTION(BlueprintCallable)
	TArray<ARPGFireBall*> SpawnFireBall();
protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FireBlast") //技能可以生成的火球的数量
	int32 NumFireBalls = 12;

private:

	UPROPERTY(EditDefaultsOnly, Category="FireBlast") //生成火球使用的类
	TSubclassOf<ARPGFireBall> FireBallClass;
};
