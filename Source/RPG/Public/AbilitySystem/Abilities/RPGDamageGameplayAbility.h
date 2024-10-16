// 版权归暮志未晚所有。

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/RPGGameplayAbility.h"
#include "Interaction/CombatInterface.h"
#include "RPGDamageGameplayAbility.generated.h"

struct FDamageEffectParams;
/**
 * 
 */
UCLASS()
class RPG_API URPGDamageGameplayAbility : public URPGGameplayAbility
{
	GENERATED_BODY()
public:

	//通过自身设置的配置创建GE实例并应用给目标
	UFUNCTION(BlueprintCallable)
	void CauseDamage(AActor* TargetActor);

	//创建技能负面效果使用的结构体
	UFUNCTION(BlueprintPure)
	FDamageEffectParams MakeDamageEffectParamsFromClassDefaults(AActor* TargetActor = nullptr);
	
protected:

	//技能命中敌人应用的计算伤害的GE
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	//技能对角色造成的伤害和类型
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Damage")
	TMap<FGameplayTag, FScalableFloat> DamageTypes;

	//负面效果伤害类型
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Damage")
	FGameplayTag DeBuffDamageType = FGameplayTag(); 

	//触发负面的机率
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Damage")
	float DeBuffChance = 20.f; 

	//负面伤害
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Damage")
	float DeBuffDamage = 5.f; 

	//负面伤害触发间隔时间
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Damage")
	float DeBuffFrequency = 1.f; 

	//负面效果持续时间
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Damage")
	float DeBuffDuration = 5.f; 

	//死亡时，受到的冲击的数值
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Damage")
	float DeathImpulseMagnitude = 3000.f; 

	//技能击中敌人后，敌人受到的击退的力度
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Damage")
	float KnockbackForceMagnitude = 1000.f; 

	//技能命中敌人触发击退的概率
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Damage")
	float KnockbackChance = 0.f; 

	//从角色设置的蒙太奇数组总，随机一个蒙太奇使用
	UFUNCTION(BlueprintPure)
	static FTaggedMontage GetRandomTaggedMontageFromArray(const TArray<FTaggedMontage>& TaggedMontages);

	float GetDamageByDamageType(float InLevel, const FGameplayTag& DamageType); //根据伤害类型获取伤害
};
