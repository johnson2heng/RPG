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

	/**
	 * 创建技能负面效果使用的结构体
	 * @param TargetActor 攻击的目标actor
	 * @param InRadialDamageOrigin 技能攻击时的位置
	 * @param bOverrideKnockbackDirection 覆盖技能攻击的朝向
	 * @param KnockbackDirectionOverride 技能攻击朝向
	 * @param bOverrideDeathImpulse 覆盖死亡冲击的方向
	 * @param DeathImpulseDirectionOverride 死亡冲击的方向
	 * @return 
	 */
	UFUNCTION(BlueprintPure)
	FDamageEffectParams MakeDamageEffectParamsFromClassDefaults(
		AActor* TargetActor = nullptr,
		FVector InRadialDamageOrigin = FVector::ZeroVector,
		bool bOverrideKnockbackDirection = false,
		FVector KnockbackDirectionOverride = FVector::ZeroVector,
		bool bOverrideDeathImpulse = false,
		FVector DeathImpulseDirectionOverride = FVector::ZeroVector);
	
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

	//当前伤害类型是否为范围伤害
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Damage")
	bool bIsRadialDamage = false;

	//内半径：在此半径内的所有目标都将受到完整的伤害
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Damage")
	float RadialDamageInnerRadius = 0.f;

	//外半径：超过这个距离的目标受到最小伤害，最小伤害如果设置为0，则圈外不受到伤害
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Damage")
	float RadialDamageOuterRadius = 0.f;

	//伤害源的中心点
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Damage")
	FVector RadialDamageOrigin = FVector::ZeroVector;

	//从角色设置的蒙太奇数组总，随机一个蒙太奇使用
	UFUNCTION(BlueprintPure)
	static FTaggedMontage GetRandomTaggedMontageFromArray(const TArray<FTaggedMontage>& TaggedMontages);

	float GetDamageByDamageType(float InLevel, const FGameplayTag& DamageType); //根据伤害类型获取伤害
};
