// 版权归暮志未晚所有。

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "Data/CharacterClassInfo.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "RPGAbilitySystemLibrary.generated.h"

struct FDamageEffectParams;
class UAbilityInfo;
class ARPGHUD;
struct FWidgetControllerParams;
class USpellMenuWidgetController;
class UAttributeMenuWidgetController;
class UOverlayWidgetController;
/**
 * 
 */
UCLASS()
class RPG_API URPGAbilitySystemLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * 创建用于生成UI的控制器的配置项结构体
	 *
	 * @param WorldContextObject 一个世界场景的对象，用于获取当前所在的世界
	 * @param OutWcParams 返回成功创建的配置项结构体
	 * @param OutRPGHUD 返回当前世界的HUD对象
	 *
	 * @return bool 如果创建成功 则返回true
	 *
	 * @note 这个函数用于在获取当前世界中的对应的UI面板使用。
	 */
	UFUNCTION(BlueprintPure, Category="RPGAbilitySystemLibrary|WidgetController", meta=(DefaultToSelf = "WorldContextObject"))
	static bool MakeWidgetControllerParams(const UObject* WorldContextObject, FWidgetControllerParams& OutWcParams, ARPGHUD*& OutRPGHUD);

	/**
	 * 获取OverlayWidget的控制器对象
	 *
	 * @param WorldContextObject 一个世界场景的对象，用于获取当前所在的世界
	 *
	 * @return UOverlayWidgetController* 返回当前世界的OverlayWidgetController
	 *
	 * @note 这个函数用于在获取当前世界中的对应的UI面板控制器使用。
	 */
	UFUNCTION(BlueprintPure, Category="RPGAbilitySystemLibrary|WidgetController", meta=(DefaultToSelf = "WorldContextObject"))
	static UOverlayWidgetController* GetOverlayWidgetController(const UObject* WorldContextObject);

	/**
	 * 获取属性面板的控制器
	 *
	 * @param WorldContextObject 一个世界场景的对象，用于获取当前所在的世界
	 *
	 * @return UAttributeMenuWidgetController* 返回当前世界的UAttributeMenuWidgetController
	 *
	 * @note 这个函数用于在获取当前世界中的对应的UI面板控制器使用。
	 */
	UFUNCTION(BlueprintPure, Category="RPGAbilitySystemLibrary|WidgetController", meta=(DefaultToSelf = "WorldContextObject"))
	static UAttributeMenuWidgetController* GetAttributeMenuWidgetController(const UObject* WorldContextObject);

	/**
	 * 获取技能面板的控制器
	 *
	 * @param WorldContextObject 一个世界场景的对象，用于获取当前所在的世界
	 *
	 * @return USpellMenuWidgetController* 返回当前世界的USpellMenuWidgetController
	 *
	 * @note 这个函数用于在获取当前世界中的对应的UI面板控制器使用。
	 */
	UFUNCTION(BlueprintPure, Category="RPGAbilitySystemLibrary|WidgetController", meta=(DefaultToSelf = "WorldContextObject"))
	static USpellMenuWidgetController* GetSpellMenuWidgetController(const UObject* WorldContextObject);

	/**
	 * 从FScalableFloat中按等级获得对应的伤害数值
	 *
	 * @param ScalableFloat 一条浮点数曲线，每个位置可以获取到不同的值
	 * @param Level 获取的目标的位置的曲线的值
	 *
	 * @return float 返回对应位置的浮点数
	 *
	 * @note 这个函数用于在蓝图内获取FScalableFloat的值。
	 */
	UFUNCTION(BlueprintCallable, Category="RPGAbilitySystemLibrary|GameplayEffects")
	static float GetScalableFloatValue(const FScalableFloat& ScalableFloat, const int32& Level);

	/**
	 * 初始化角色的属性
	 *
	 * @param WorldContextObject 一个世界场景的对象，用于获取当前所在的世界
	 * @param CharacterClass 当前角色的类
	 * @param Level 角色等级
	 * @param ASC 角色的技能系统组件
	 *
	 * @return void 
	 *
	 * @note 这个函数用于初始化小怪的属性，相关属性配置需要在GameMode里去设置。
	 */
	UFUNCTION(BlueprintCallable, Category="RPGAbilitySystemLibrary|CharacterClassDefaults", meta=(DefaultToSelf = "WorldContextObject"))
	static void InitializeDefaultAttributes(const UObject* WorldContextObject, ECharacterClass CharacterClass, float Level, UAbilitySystemComponent* ASC);

	/**
	 * 初始化角色的技能
	 *
	 * @param WorldContextObject 一个世界场景的对象，用于获取当前所在的世界
	 * @param ASC 角色的技能系统组件
	 * @param CharacterClass 当前角色的类
	 *
	 * @return void 
	 *
	 * @note 这个函数用于初始化小怪的技能，相关属性配置需要在GameMode里去设置。
	 */
	UFUNCTION(BlueprintCallable, Category="RPGAbilitySystemLibrary|CharacterClassDefaults", meta=(DefaultToSelf = "WorldContextObject"))
	static void GiveStartupAbilities(const UObject* WorldContextObject, UAbilitySystemComponent* ASC, ECharacterClass CharacterClass);

	/**
	 * 获取敌人配置数据
	 *
	 * @param WorldContextObject 一个世界场景的对象，用于获取当前所在的世界
	 *
	 * @return UCharacterClassInfo* 返回敌人的配置对象
	 *
	 * @note 这个函数用于提供数据对象用于初始化技能和属性，相关属性配置需要在GameMode里去设置。
	 */
	UFUNCTION(BlueprintCallable, Category="RPGAbilitySystemLibrary|CharacterClassDefaults", meta=(DefaultToSelf = "WorldContextObject"))
	static UCharacterClassInfo* GetCharacterClassInfo(const UObject* WorldContextObject);

	/**
	 * 获取玩家角色技能配置数据
	 *
	 * @param WorldContextObject 一个世界场景的对象，用于获取当前所在的世界
	 *
	 * @return UAbilityInfo* 返回玩家角色的技能配置数据
	 *
	 * @note 返回技能对应的标签和技能显示按钮相关数据。
	 */
	UFUNCTION(BlueprintCallable, Category="RPGAbilitySystemLibrary|CharacterClassDefaults", meta=(DefaultToSelf = "WorldContextObject"))
	static UAbilityInfo* GetAbilityInfo(const UObject* WorldContextObject);

	/**
	 * 获取根据敌人类型和等级获取敌人产生的经验
	 *
	 * @param WorldContextObject 一个世界场景的对象，用于获取当前所在的世界
	 * @param CharacterClass 角色的类
	 * @param CharacterLevel 角色等级
	 *
	 * @return 当前角色能够提供的经验值
	 *
	 * @note 返回技能对应的标签和技能显示按钮相关数据。
	 */
	UFUNCTION(BlueprintCallable, Category="RPGAbilitySystemLibrary|CharacterClassDefaults", meta=(DefaultToSelf = "WorldContextObject"))
	static int32 GetXPRewardForClassAndLevel(const UObject* WorldContextObject, ECharacterClass CharacterClass, int32 CharacterLevel);

	/**
	 * 获取当前GE是否触发格挡
	 *
	 * @param EffectContextHandle 当前GE的上下文句柄
	 *
	 * @return 格挡成功 返回true
	 *
	 * @note 此属性是RPGAbilityTypes.h内自定义属性，可实现复制。
	 */
	UFUNCTION(BlueprintPure, Category="RPGAbilitySystemLibrary|GameplayEffects")
	static bool IsBlockedHit(const FGameplayEffectContextHandle& EffectContextHandle);

	/**
	 * 获取当前GE是否触发暴击
	 *
	 * @param EffectContextHandle 当前GE的上下文句柄
	 *
	 * @return 伤害触发暴击 返回true
	 *
	 * @note 此属性是RPGAbilityTypes.h内自定义属性，可实现复制。
	 */
	UFUNCTION(BlueprintPure, Category="RPGAbilitySystemLibrary|GameplayEffects")
	static bool IsCriticalHit(const FGameplayEffectContextHandle& EffectContextHandle);

	/**
	 * 获取当前GE是否成功应用负面效果
	 *
	 * @param EffectContextHandle 当前GE的上下文句柄
	 *
	 * @return 成功应用负面效果 返回true
	 *
	 * @note 此属性是RPGAbilityTypes.h内自定义属性，可实现复制。
	 */
	UFUNCTION(BlueprintPure, Category="RPGAbilitySystemLibrary|GameplayEffects")
	static bool IsSuccessfulDeBuff(const FGameplayEffectContextHandle& EffectContextHandle);

	/**
	 * 获取当前GE负面效果伤害
	 *
	 * @param EffectContextHandle 当前GE的上下文句柄
	 *
	 * @return 返回负面效果伤害数值
	 *
	 * @note 此属性是RPGAbilityTypes.h内自定义属性，可实现复制。
	 */
	UFUNCTION(BlueprintPure, Category="RPGAbilitySystemLibrary|GameplayEffects")
	static float GetDeBuffDamage(const FGameplayEffectContextHandle& EffectContextHandle);

	/**
	 * 获取当前GE负面效果持续时间
	 *
	 * @param EffectContextHandle 当前GE的上下文句柄
	 *
	 * @return 返回负面效果持续时间
	 *
	 * @note 此属性是RPGAbilityTypes.h内自定义属性，可实现复制。
	 */
	UFUNCTION(BlueprintPure, Category="RPGAbilitySystemLibrary|GameplayEffects")
	static float GetDeBuffDuration(const FGameplayEffectContextHandle& EffectContextHandle);

	/**
	 * 获取当前GE负面效果触发间隔
	 *
	 * @param EffectContextHandle 当前GE的上下文句柄
	 *
	 * @return 返回负面效果触发间隔
	 *
	 * @note 此属性是RPGAbilityTypes.h内自定义属性，可实现复制。
	 */
	UFUNCTION(BlueprintPure, Category="RPGAbilitySystemLibrary|GameplayEffects")
	static float GetDeBuffFrequency(const FGameplayEffectContextHandle& EffectContextHandle);

	/**
	 * 获取当前GE负面效果伤害类型
	 *
	 * @param EffectContextHandle 当前GE的上下文句柄
	 *
	 * @return 返回负面效果伤害类型标签
	 *
	 * @note 此属性是RPGAbilityTypes.h内自定义属性，可实现复制。
	 */
	UFUNCTION(BlueprintPure, Category="RPGAbilitySystemLibrary|GameplayEffects")
	static FGameplayTag GetDeBuffDamageType(const FGameplayEffectContextHandle& EffectContextHandle);

	/**
	 * 获取当前GE死亡冲击的方向和力度
	 *
	 * @param EffectContextHandle 当前GE的上下文句柄
	 *
	 * @return 此次伤害如果能够造成目标死亡，将应用冲击的方向和力度的值
	 *
	 * @note 此属性是RPGAbilityTypes.h内自定义属性，可实现复制。
	 */
	UFUNCTION(BlueprintPure, Category="RPGAbilitySystemLibrary|GameplayEffects")
	static FVector GetDeathImpulse(const FGameplayEffectContextHandle& EffectContextHandle);

	/**
	 * 获取当前GE攻击击退的方向和力度
	 *
	 * @param EffectContextHandle 当前GE的上下文句柄
	 *
	 * @return 攻击的击退会根据概率计算，如果有值，则为应用成功
	 *
	 * @note 此属性是RPGAbilityTypes.h内自定义属性，可实现复制。
	 */
	UFUNCTION(BlueprintPure, Category="RPGAbilitySystemLibrary|GameplayEffects")
	static FVector GetKnockbackForce(const FGameplayEffectContextHandle& EffectContextHandle);

	/**
	 * 设置GE是否触发格挡
	 *
	 * @param EffectContextHandle 当前GE的上下文句柄
	 * @param bInIsBlockedHit 格挡值
	 *
	 * @note 此属性是RPGAbilityTypes.h内自定义属性，可实现复制。
	 */
	UFUNCTION(BlueprintCallable, Category="RPGAbilitySystemLibrary|GameplayEffects")
	static void SetIsBlockHit(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, bool bInIsBlockedHit);

	/**
	 * 设置GE是否触发暴击
	 *
	 * @param EffectContextHandle 当前GE的上下文句柄
	 * @param bInIsCriticalHit 暴击是否触发
	 *
	 * @note 此属性是RPGAbilityTypes.h内自定义属性，可实现复制。
	 */
	UFUNCTION(BlueprintCallable, Category="RPGAbilitySystemLibrary|GameplayEffects")
	static void SetIsCriticalHit(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, bool bInIsCriticalHit);

	/**
	 * 设置GE是否应用负面效果
	 *
	 * @param EffectContextHandle 当前GE的上下文句柄
	 * @param bInIsSuccessfulDeBuff 应用DeBuff
	 *
	 * @note 此属性是RPGAbilityTypes.h内自定义属性，可实现复制。
	 */
	UFUNCTION(BlueprintCallable, Category="RPGAbilitySystemLibrary|GameplayEffects")
	static void SetIsSuccessfulDeBuff(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, bool bInIsSuccessfulDeBuff);

	/**
	 * 设置GE负面效果相关数值
	 *
	 * @param EffectContextHandle 当前GE的上下文句柄
	 * @param InDamageType 负面效果伤害类型
	 * @param InDamage 负面效果伤害
	 * @param InDuration 负面效果持续时间
	 * @param InFrequency 负面效果触发间隔时间
	 *
	 * @note 此属性是RPGAbilityTypes.h内自定义属性，可实现复制。
	 */
	UFUNCTION(BlueprintCallable, Category="RPGAbilitySystemLibrary|GameplayEffects")
	static void SetDeBuff(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, FGameplayTag& InDamageType, float InDamage, float InDuration, float InFrequency);
	
	/**
	 * 设置GE死亡冲击的方向和力度
	 *
	 * @param EffectContextHandle 当前GE的上下文句柄
	 * @param InDeathImpulse 死亡冲击的值
	 *
	 * @note 此属性是RPGAbilityTypes.h内自定义属性，可实现复制。
	 */
	UFUNCTION(BlueprintCallable, Category="RPGAbilitySystemLibrary|GameplayEffects")
	static void SetDeathImpulse(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, const FVector& InDeathImpulse);
	
	/**
	 * 设置GE攻击击退的方向和力度
	 *
	 * @param EffectContextHandle 当前GE的上下文句柄
	 * @param InKnockbackForce 攻击击退的方向和力度
	 *
	 * @note 此属性是RPGAbilityTypes.h内自定义属性，可实现复制。
	 */
	UFUNCTION(BlueprintCallable, Category="RPGAbilitySystemLibrary|GameplayEffects")
	static void SetKnockbackForce(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, const FVector& InKnockbackForce);
	
	/**
	 * 通过GE的配置项生成GE并应用给目标ASC
	 *
	 * @param DamageEffectParams GE的配置数据
	 *
	 * @return 生成的应用的GE上下文句柄
	 *
	 * @note 此属性是RPGAbilityTypes.h内自定义属性，可实现复制。
	 */
	UFUNCTION(BlueprintCallable, Category="RPGAbilitySystemLibrary|GameplayEffects")
	static FGameplayEffectContextHandle ApplyDamageEffect(const FDamageEffectParams& DamageEffectParams);

	/**
	 * 获取到攻击位置半径内的所有动态Actor
	 *
	 * @param WorldContextObject 一个世界场景的对象，用于获取当前所在的世界
	 * @param OutOverlappingActors 计算得出的所有半径内的角色对象
	 * @param ActorsToIgnore 需要忽略的对象数组
	 * @param Radius 获取半径
	 * @param SphereOrigin 半径的中心位置
	 *
	 * @note 此属性是RPGAbilityTypes.h内自定义属性，可实现复制。
	 */
	UFUNCTION(BlueprintCallable, Category="RPGAbilitySystemLibrary|GameplayMechanics", meta=(DefaultToSelf = "WorldContextObject"))
	static void GetLivePlayersWithinRadius(const UObject* WorldContextObject, TArray<AActor*>& OutOverlappingActors, const TArray<AActor*>& ActorsToIgnore, float Radius, const FVector& SphereOrigin);

	/**
	 * 获取距离目标位置最近的几个目标
	 * @param MaxTargets 获取最大目标的数量
	 * @param Actors 需要计算的目标数组
	 * @param OutClosestTargets 返回获取到的最近的目标
	 * @param Origin 计算的位置
	 */
	UFUNCTION(BlueprintCallable, Category="RPGAbilitySystemLibrary|GameplayMechanics")
	static void GetClosestTargets(int32 MaxTargets, const TArray<AActor*>& Actors, TArray<AActor*>& OutClosestTargets, const FVector& Origin);
	
	/**
	 * 判断两个角色是否是友方单位
	 *
	 * @param FirstActor 第一个角色对象
	 * @param SecondActor 第二个角色对象
	 *
	 * @return bool 如果两个角色是友方单位，将返回false，如果两个角色是敌对单位，将返回true
	 *
	 * @note 用于判断一个目标是否可以攻击。
	 */
	UFUNCTION(BlueprintPure, Category="RPGAbilitySystemLibrary|GameplayMechanics")
	static bool IsNotFriend(const AActor* FirstActor, const AActor* SecondActor);

	/**
	 * 这个函数根据传入的值计算均匀分布的多段角度，
	 *
	 * @param Forward 正前方向
	 * @param Axis 基于旋转的轴
	 * @param Spread 角度范围
	 * @param NumRotators 分段数
	 *
	 * @return TArray<FRotator&> 返回每段角度的中间角度的数组
	 *
	 * @note 这个函数用于在技能生成投掷物的函数逻辑中。
	 */
	UFUNCTION(BlueprintPure, Category="RPGAbilitySystemLibrary|GameplayMechanics")
	static TArray<FRotator> EvenlySpacedRotators(const FVector& Forward, const FVector & Axis, float Spread, int32 NumRotators);

	/**
	 * 这个函数根据传入的值计算均匀分布的多段朝向
	 *
	 * @param Forward 正前方向
	 * @param Axis 基于旋转的轴
	 * @param Spread 角度范围
	 * @param NumVectors 分段数
	 *
	 * @return TArray<FVector&> 返回每段角度的中间角度的朝向数组
	 *
	 * @note 这个函数用于在技能生成投掷物的函数逻辑中。
	 */
	UFUNCTION(BlueprintPure, Category="RPGAbilitySystemLibrary|GameplayMechanics")
	static TArray<FVector> EvenlyRotatedVectors(const FVector& Forward, const FVector & Axis, float Spread, int32 NumVectors);
};
