// 版权归暮志未晚所有。

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AbilitySystem/Data/CharacterClassInfo.h"
#include "UObject/Interface.h"
#include "CombatInterface.generated.h"

class UAbilitySystemComponent;
class UNiagaraSystem;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnASCRegistered, UAbilitySystemComponent*); //Actor初始化ASC完成后委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeath, AActor*, DeadActor); //Actor死亡后的委托
DECLARE_MULTICAST_DELEGATE_OneParam(FOnDamageSignature, float /*范围伤害造成的最终数值*/); //返回范围伤害能够对自身造成的伤害，在TakeDamage里广播

//配置敌人攻击的结构体，可以获取动画，释放位置，击中特效
USTRUCT(BlueprintType)
struct FTaggedMontage
{
	GENERATED_BODY()

	//使用的蒙太奇
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAnimMontage* Montage = nullptr;

	//当前数据的索引
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag MontageTag;

	//部位对应的标签
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag SocketTag;

	//攻击时的触发伤害的骨骼插槽或设置技能释放的位置
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName CombatTipSocketName;
	
	//击中敌人时的触发音效
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	USoundBase* ImpactSound = nullptr;
};

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType)
class UCombatInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class RPG_API ICombatInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	/**
	 * 获取玩家等级
	 * @return 玩家等级
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	int32 GetPlayerLevel();

	/**
	 * 获取技能释放位置，通过在蓝图中设置获取WeaponTipSocketName的位置
	 * @return 此函数只返回在武器上的施法位置
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	FVector GetCombatSocketLocation() const;

	/**
	 * 获取技能释放位置
	 * @param SocketTag 骨骼标签，用于判断骨骼位置是属于身体还是武器
	 * @param SocketName 骨骼名称，用于获取实际发射位置
	 * @return 技能发射位置
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	FVector GetCombatSocketLocationByTag(const FGameplayTag SocketTag, const FName SocketName) const;

	/**
	 * 获取角色使用的武器指针
	 * @return 武器骨骼网格体组件
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	USkeletalMeshComponent* GetWeapon();

	/**
	 * 通过结构体获取对应的骨骼插槽的位置
	 * @param TaggedMontage 动画和骨骼映射的结构体
	 * @return 返回发射位置
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	FVector GetCombatSocketLocationByStruct(const FTaggedMontage TaggedMontage) const;

	/**
	 * 设置蒙太奇的Motion Wrapping更新朝向的目标位置，需要在蓝图中实现
	 * @param Target 目标位置
	 */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateFacingTarget(const FVector& Target);

	/**
	 * 获取角色受击时播放的蒙太奇动画
	 * @return 蒙太奇动画指针
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	UAnimMontage* GetHitReactMontage(); //获取受击蒙太奇动画

	/**
	 * 角色死亡时调用函数
	 * @param DeathImpulse 死亡时受到的冲击力 
	 */
	virtual void Die(const FVector& DeathImpulse) = 0;

	/**
	 * 获取当前角色是否死亡
	 * @return 死亡返回true
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool IsDead() const;

	/**
	 * 获取当前角色实例
	 * @return 角色实例，可以获取基础AActor相关函数和属性
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	AActor* GetAvatar();

	/**
	 * 获取设置的蒙太奇标签对应结构体数组
	 * @return 敌人角色所有攻击技能蒙太奇结构体组成的数组
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	TArray<FTaggedMontage> GetAttackMontages();

	/**
	 * 获取角色的受伤特效
	 * @return 角色受伤特效系统
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	UNiagaraSystem* GetBloodEffect();

	/**
	 * 通过标签获取对应的结构体
	 * @param MontageTag 结构体内对应的标签
	 * @return 结构体对象
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	FTaggedMontage GetTaggedMontageByTag(const FGameplayTag& MontageTag);

	/**
	 * 获取召唤角色当前存活的仆从数量
	 * @return 数量个数
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	int32 GetMinionCount();

	/**
	 * 设置角色仆从的增长数量，负数为负增长
	 * @param Amount 增长数量，负数为减少
	 * @note 当前召唤的数量是记录在角色身上的，如果仆从死亡将通过此函数减少召唤师身上的数量
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void IncrementMinionCount(const int32 Amount); //设置角色仆从的增长数量，负数为负增长

	/**
	 * 获取角色的职业类型
	 * @return 职业类型
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	ECharacterClass GetCharacterClass();

	/**
	 * 获取角色注册成功的广播委托
	 * @return 返回委托
	 */
	virtual FOnASCRegistered& GetOnASCRegisteredDelegate() = 0;

	/**
	 * 获取角色死亡触发的委托
	 * @return 委托
	 */
	virtual FOnDeath& GetOnDeathDelegate() = 0; 

	/**
	 * 获取角色受到伤害触发的委托，由于委托是创建在角色基类里的，这里可以通过添加struct来实现前向声明，不需要在头部声明一遍。
	 * @return 委托
	 */
	virtual FOnDamageSignature& GetOnDamageDelegate() = 0; 

	/**
	 * 设置角色是否在循环施法中
	 * @param bInLoop 开启或关闭角色循环施法状态
	 */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetInShockLoop(bool bInLoop);

	/**
	 * 获取角色是否处于闪电链攻击状态
	 * @return 布尔值，如果处于返回true
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool IsBeingShocked() const;

	/**
	 * 设置角色是否处于闪电链攻击状态
	 * @param bInShock 
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SetIsBeingShocked(bool bInShock);
};
