#pragma once //预处理指令 确保这个头文件只被包含（include）一次，防止重复定义。

#include "GameplayEffectTypes.h"
#include "RPGAbilityTypes.generated.h"

class UGameplayEffect;

USTRUCT(BlueprintType)
struct FDamageEffectParams
{
	GENERATED_BODY()

	FDamageEffectParams(){}

	UPROPERTY()
	TObjectPtr<UObject> WorldContextObject = nullptr; //当前场景上下文对象

	UPROPERTY()
	TSubclassOf<UGameplayEffect> DamageGameplayEffectClass = nullptr; //需要应用的GE的类

	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> SourceAbilitySystemComponent; //源ASC

	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> TargetAbilitySystemComponent; //目标ASC

	UPROPERTY()
	TMap<FGameplayTag, float> DamageTypes; //技能造成的多种伤害和伤害类型

	UPROPERTY()
	float AbilityLevel = 1.f; //技能等级

	UPROPERTY()
	FGameplayTag DeBuffDamageType = FGameplayTag(); //负面效果伤害类型

	UPROPERTY()
	float DeBuffChance = 0.f; //触发负面效果概率

	UPROPERTY()
	float DeBuffDamage = 0.f; //负面效果伤害

	UPROPERTY()
	float DeBuffDuration = 0.f; //负面效果持续时间

	UPROPERTY()
	float DeBuffFrequency = 0.f; //负面效果触发频率

	UPROPERTY()
	float DeathImpulseMagnitude = 0.f; //死亡时受到的冲击力

	UPROPERTY()
	FVector DeathImpulse = FVector::ZeroVector; //死亡时受到冲击的朝向
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

	void SetIsBlockedHit(const bool bInIsBlockedHit) { bIsBlockedHit = bInIsBlockedHit; } // 设置 格挡
	void SetIsCriticalHit(const bool bInIsCriticalHit) { bIsCriticalHit = bInIsCriticalHit; } // 设置 暴击
	void SetIsSuccessfulDeBuff(const bool bInIsSuccessfulDeBuff) { bIsSuccessfulDeBuff = bInIsSuccessfulDeBuff; } //设置 应用负面效果
	void SetDeBuffDamage(const float InDamage) { DeBuffDamage = InDamage; } //设置 负面效果伤害
	void SetDeBuffDuration(const float InDuration) { DeBuffDuration = InDuration; } //设置 负面效果伤害
	void SetDeBuffFrequency(const float InFrequency) { DeBuffFrequency = InFrequency; } //设置 负面效果伤害
	void SetDeBuffDamageType(const TSharedPtr<FGameplayTag>& InDamageType) { DamageType = InDamageType; } //设置 负面效果伤害类型
	void SetDeathImpulse(const FVector& InImpulse) { DeathImpulse = InImpulse; } //设置死亡冲击的方向和力度
	
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

	UPROPERTY()
	bool bIsBlockedHit = false; //格挡

	UPROPERTY()
	bool bIsCriticalHit = false; //暴击

	UPROPERTY()
	bool bIsSuccessfulDeBuff = false; //成功应用负面效果
	
	UPROPERTY()
	float DeBuffDamage = 0.f; //负面效果每次造成的伤害

	UPROPERTY()
	float DeBuffDuration = 0.f; //负面效果持续时间

	UPROPERTY()
	float DeBuffFrequency = 0.f; //负面效果触发频率间隔

	TSharedPtr<FGameplayTag> DamageType; //负面效果的伤害类型

	UPROPERTY()
	FVector DeathImpulse = FVector::ZeroVector; //死亡时冲击的方向
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
