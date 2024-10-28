#pragma once //预处理指令 确保这个头文件只被包含（include）一次，防止重复定义。

#include "GameplayEffectTypes.h"
#include "RPGAbilityTypes.generated.h"

class UGameplayEffect;

USTRUCT(BlueprintType)
struct FDamageEffectParams
{
	GENERATED_BODY()

	FDamageEffectParams(){}

	//当前场景上下文对象
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UObject> WorldContextObject = nullptr; 

	//需要应用的GE的类
	UPROPERTY(BlueprintReadWrite)
	TSubclassOf<UGameplayEffect> DamageGameplayEffectClass = nullptr; 

	//源ASC
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UAbilitySystemComponent> SourceAbilitySystemComponent; 

	//目标ASC
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UAbilitySystemComponent> TargetAbilitySystemComponent; 

	//技能造成的多种伤害和伤害类型
	UPROPERTY(BlueprintReadWrite)
	TMap<FGameplayTag, float> DamageTypes; 

	//技能等级
	UPROPERTY(BlueprintReadWrite)
	float AbilityLevel = 1.f; 

	//负面效果伤害类型
	UPROPERTY(BlueprintReadWrite)
	FGameplayTag DeBuffDamageType = FGameplayTag(); 

	//触发负面效果概率
	UPROPERTY(BlueprintReadWrite)
	float DeBuffChance = 0.f; 

	//负面效果伤害
	UPROPERTY(BlueprintReadWrite)
	float DeBuffDamage = 0.f; 

	//负面效果持续时间
	UPROPERTY(BlueprintReadWrite)
	float DeBuffDuration = 0.f; 

	//负面效果触发频率
	UPROPERTY(BlueprintReadWrite)
	float DeBuffFrequency = 0.f; 

	//死亡时受到的冲击力
	UPROPERTY(BlueprintReadWrite)
	float DeathImpulseMagnitude = 0.f; 

	//死亡时受到冲击的朝向
	UPROPERTY(BlueprintReadWrite)
	FVector DeathImpulse = FVector::ZeroVector; 

	//攻击击退的强度
	UPROPERTY(BlueprintReadWrite)
	float KnockbackForceMagnitude = 0.f;

	//攻击时击退的方向
	UPROPERTY(BlueprintReadWrite)
	FVector KnockbackForce = FVector::ZeroVector;

	//攻击时击退概率
	UPROPERTY(BlueprintReadWrite)
	float KnockbackChance = 0.f;

	//当前伤害类型是否为范围伤害
	UPROPERTY(BlueprintReadWrite)
	bool bIsRadialDamage = false;

	//内半径：在此半径内的所有目标都将受到完整的伤害
	UPROPERTY(BlueprintReadWrite)
	float RadialDamageInnerRadius = 0.f;

	//外半径：超过这个距离的目标受到最小伤害，最小伤害如果设置为0，则圈外不受到伤害
	UPROPERTY(BlueprintReadWrite)
	float RadialDamageOuterRadius = 0.f;

	//伤害源的中心点
	UPROPERTY(BlueprintReadWrite)
	FVector RadialDamageOrigin = FVector::ZeroVector;
};

USTRUCT(BlueprintType) //在蓝图中可作为类型使用
struct FRPGGameplayEffectContext : public FGameplayEffectContext
{
	GENERATED_BODY() //宏 自动生成构造函数、析构函数、拷贝构造函数等

public:

	bool IsBlockedHit() const { return bIsBlockedHit; } //获取 格挡
	bool IsCriticalHit() const { return bIsCriticalHit; } //获取 暴击
	bool IsSuccessfulDeBuff() const { return bIsSuccessfulDeBuff; } //获取 应用负面效果
	float GetDeBuffDamage() const { return DeBuffDamage; } //获取 负面效果伤害
	float GetDeBuffDuration() const { return DeBuffDuration; } //获取 负面效果持续时间
	float GetDeBuffFrequency() const { return DeBuffFrequency; } //获取 负面效果伤害触发间隔
	TSharedPtr<FGameplayTag> GetDeBuffDamageType() const { return DamageType; } //获取 负面效果伤害类型
	FVector GetDeathImpulse() const { return DeathImpulse; } //获取到死亡冲击的方向和力度
	FVector GetKnockbackForce() const { return KnockbackForce; } //获取到攻击击退的方向和力度
	bool IsRadialDamage() const { return bIsRadialDamage; } //获取当前是否为范围伤害
	float GetRadialDamageInnerRadius() const { return RadialDamageInnerRadius; } //返回内半径距离
	float GetRadialDamageOuterRadius() const { return RadialDamageOuterRadius; } //返回外半径距离
	FVector GetRadialDamageOrigin() const { return RadialDamageOrigin; } //返回伤害源的中心点

	void SetIsBlockedHit(const bool bInIsBlockedHit) { bIsBlockedHit = bInIsBlockedHit; } // 设置 格挡
	void SetIsCriticalHit(const bool bInIsCriticalHit) { bIsCriticalHit = bInIsCriticalHit; } // 设置 暴击
	void SetIsSuccessfulDeBuff(const bool bInIsSuccessfulDeBuff) { bIsSuccessfulDeBuff = bInIsSuccessfulDeBuff; } //设置 应用负面效果
	void SetDeBuffDamage(const float InDamage) { DeBuffDamage = InDamage; } //设置 负面效果伤害
	void SetDeBuffDuration(const float InDuration) { DeBuffDuration = InDuration; } //设置 负面效果伤害
	void SetDeBuffFrequency(const float InFrequency) { DeBuffFrequency = InFrequency; } //设置 负面效果伤害
	void SetDeBuffDamageType(const TSharedPtr<FGameplayTag>& InDamageType) { DamageType = InDamageType; } //设置 负面效果伤害类型
	void SetDeathImpulse(const FVector& InImpulse) { DeathImpulse = InImpulse; } //设置死亡冲击的方向和力度
	void SetKnockbackForce(const FVector& InKnockbackForce) { KnockbackForce = InKnockbackForce; } //设置攻击击退的方向和力度
	void SetIsRadialDamage(bool bInIsRadialDamage) {bIsRadialDamage = bInIsRadialDamage; } //设置当前是否为范围伤害
	void SetRadialDamageInnerRadius(float InRadialDamageInnerRadius) { RadialDamageInnerRadius = InRadialDamageInnerRadius; } //设置内半径距离
	void SetRadialDamageOuterRadius(float InRadialDamageOuterRadius) { RadialDamageOuterRadius = InRadialDamageOuterRadius; } //设置外半径距离
	void SetRadialDamageOrigin(const FVector& InRadialDamageOrigin) { RadialDamageOrigin = InRadialDamageOrigin; } //设置伤害源的中心点
	
	/** 返回用于序列化的实际结构体 */
	virtual UScriptStruct* GetScriptStruct() const override
	{
		return StaticStruct();
	}

	/** 用于序列化类的参数 */
	virtual bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess) override;

	/** 创建一个副本，用于后续网络复制或者后续修改 */
	virtual FRPGGameplayEffectContext* Duplicate() const override
	{
		FRPGGameplayEffectContext* NewContext = new FRPGGameplayEffectContext();
		*NewContext = *this;//WithCopy 设置为true，就可以通过赋值操作进行拷贝
		if (GetHitResult())
		{
			// 深拷贝 hit result
			NewContext->AddHitResult(*GetHitResult(), true);
		}
		return NewContext;
	}
protected:

	//格挡
	UPROPERTY()
	bool bIsBlockedHit = false; 

	//暴击
	UPROPERTY()
	bool bIsCriticalHit = false; 

	//成功应用负面效果
	UPROPERTY()
	bool bIsSuccessfulDeBuff = false; 

	//负面效果每次造成的伤害
	UPROPERTY()
	float DeBuffDamage = 0.f; 

	//负面效果持续时间
	UPROPERTY()
	float DeBuffDuration = 0.f; 

	//负面效果触发频率间隔
	UPROPERTY()
	float DeBuffFrequency = 0.f; 

	//负面效果的伤害类型
	TSharedPtr<FGameplayTag> DamageType; 

	//死亡时冲击的方向
	UPROPERTY()
	FVector DeathImpulse = FVector::ZeroVector; 

	//攻击时击退的方向
	UPROPERTY()
	FVector KnockbackForce = FVector::ZeroVector;
	
	//当前伤害类型是否为范围伤害
	UPROPERTY()
	bool bIsRadialDamage = false;

	//内半径：在此半径内的所有目标都将受到完整的伤害
	UPROPERTY()
	float RadialDamageInnerRadius = 0.f;

	//外半径：超过这个距离的目标受到最小伤害，最小伤害如果设置为0，则圈外不受到伤害
	UPROPERTY()
	float RadialDamageOuterRadius = 0.f;

	//伤害源的中心点
	UPROPERTY()
	FVector RadialDamageOrigin = FVector::ZeroVector;
};

template<>
struct TStructOpsTypeTraits< FRPGGameplayEffectContext > : public TStructOpsTypeTraitsBase2< FRPGGameplayEffectContext >
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true		// Necessary so that TSharedPtr<FHitResult> Data is copied around
	};
};
