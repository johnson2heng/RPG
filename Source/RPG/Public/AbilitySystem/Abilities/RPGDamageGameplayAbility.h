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

	UFUNCTION(BlueprintCallable)
	void CauseDamage(AActor* TargetActor);

	//创建技能负面效果使用的结构体
	FDamageEffectParams MakeDamageEffectParamsFromClassDefaults(AActor* TargetActor = nullptr);
	
protected:
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Damage")
	TMap<FGameplayTag, FScalableFloat> DamageTypes;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Damage")
	FGameplayTag DeBuffDamageType = FGameplayTag(); //负面效果伤害类型

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Damage")
	float DeBuffChance = 20.f; //触发负面的机率
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Damage")
	float DeBuffDamage = 5.f; //负面伤害

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Damage")
	float DeBuffFrequency = 1.f; //负面伤害触发间隔时间

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Damage")
	float DeBuffDuration = 5.f; //负面效果持续时间

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Damage")
	float DeathImpulseMagnitude = 60.f; //死亡时，受到的冲击的数值

	//从角色设置的蒙太奇数组总，随机一个蒙太奇使用
	UFUNCTION(BlueprintPure)
	static FTaggedMontage GetRandomTaggedMontageFromArray(const TArray<FTaggedMontage>& TaggedMontages);

	float GetDamageByDamageType(float InLevel, const FGameplayTag& DamageType); //根据伤害类型获取伤害
};
