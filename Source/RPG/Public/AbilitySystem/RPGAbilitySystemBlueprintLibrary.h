// 版权归暮志未晚所有。

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "Data/CharacterClassInfo.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "RPGAbilitySystemBlueprintLibrary.generated.h"

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
class RPG_API URPGAbilitySystemBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	//创建控制器所需的参数配置项
	UFUNCTION(BlueprintPure, Category="RPGAbilitySystemLibrary|WidgetController", meta=(DefaultToSelf = "WorldContextObject"))
	static bool MakeWidgetControllerParams(const UObject* WorldContextObject, FWidgetControllerParams& OutWcParams, ARPGHUD*& OutRPGHUD);

	//获取UI的控制器
	UFUNCTION(BlueprintPure, Category="RPGAbilitySystemLibrary|WidgetController", meta=(DefaultToSelf = "WorldContextObject"))
	static UOverlayWidgetController* GetOverlayWidgetController(const UObject* WorldContextObject);

	//获取属性面板的控制器
	UFUNCTION(BlueprintPure, Category="RPGAbilitySystemLibrary|WidgetController", meta=(DefaultToSelf = "WorldContextObject"))
	static UAttributeMenuWidgetController* GetAttributeMenuWidgetController(const UObject* WorldContextObject);

	//获取属性面板的控制器
	UFUNCTION(BlueprintPure, Category="RPGAbilitySystemLibrary|WidgetController", meta=(DefaultToSelf = "WorldContextObject"))
	static USpellMenuWidgetController* GetSpellMenuWidgetController(const UObject* WorldContextObject);

	//从FScalableFloat中按等级获得对应的伤害数值
	UFUNCTION(BlueprintCallable, Category="RPGAbilitySystemLibrary|GameplayEffects")
	static float GetScalableFloatValue(const FScalableFloat ScalableFloat, const int32 Level);

	//初始化角色的属性
	UFUNCTION(BlueprintCallable, Category="RPGAbilitySystemLibrary|CharacterClassDefaults", meta=(DefaultToSelf = "WorldContextObject"))
	static void InitializeDefaultAttributes(const UObject* WorldContextObject, ECharacterClass CharacterClass, float Level, UAbilitySystemComponent* ASC);

	//初始化角色的技能
	UFUNCTION(BlueprintCallable, Category="RPGAbilitySystemLibrary|CharacterClassDefaults", meta=(DefaultToSelf = "WorldContextObject"))
	static void GiveStartupAbilities(const UObject* WorldContextObject, UAbilitySystemComponent* ASC, ECharacterClass CharacterClass);

	//获取角色配置数据
	UFUNCTION(BlueprintCallable, Category="RPGAbilitySystemLibrary|CharacterClassDefaults", meta=(DefaultToSelf = "WorldContextObject"))
	static UCharacterClassInfo* GetCharacterClassInfo(const UObject* WorldContextObject);

	//获取玩家角色技能配置数据
	UFUNCTION(BlueprintCallable, Category="RPGAbilitySystemLibrary|CharacterClassDefaults", meta=(DefaultToSelf = "WorldContextObject"))
	static UAbilityInfo* GetAbilityInfo(const UObject* WorldContextObject);

	//获取根据敌人类型和等级获取敌人产生的经验
	UFUNCTION(BlueprintCallable, Category="RPGAbilitySystemLibrary|CharacterClassDefaults", meta=(DefaultToSelf = "WorldContextObject"))
	static int32 GetXPRewardForClassAndLevel(const UObject* WorldContextObject, ECharacterClass CharacterClass, int32 CharacterLevel);

	//获取当前GE是否触发格挡
	UFUNCTION(BlueprintPure, Category="RPGAbilitySystemLibrary|GameplayEffects")
	static bool IsBlockedHit(const FGameplayEffectContextHandle& EffectContextHandle);

	//获取当前GE是否触发暴击
	UFUNCTION(BlueprintPure, Category="RPGAbilitySystemLibrary|GameplayEffects")
	static bool IsCriticalHit(const FGameplayEffectContextHandle& EffectContextHandle);

	//获取当前GE是否成功应用负面效果
	UFUNCTION(BlueprintPure, Category="RPGAbilitySystemLibrary|GameplayEffects")
	static bool IsSuccessfulDeBuff(const FGameplayEffectContextHandle& EffectContextHandle);

	//获取当前GE负面效果伤害
	UFUNCTION(BlueprintPure, Category="RPGAbilitySystemLibrary|GameplayEffects")
	static float GetDeBuffDamage(const FGameplayEffectContextHandle& EffectContextHandle);

	//获取当前GE负面效果持续时间
	UFUNCTION(BlueprintPure, Category="RPGAbilitySystemLibrary|GameplayEffects")
	static float GetDeBuffDuration(const FGameplayEffectContextHandle& EffectContextHandle);

	//获取当前GE负面效果触发间隔
	UFUNCTION(BlueprintPure, Category="RPGAbilitySystemLibrary|GameplayEffects")
	static float GetDeBuffFrequency(const FGameplayEffectContextHandle& EffectContextHandle);

	//获取当前GE负面效果伤害类型
	UFUNCTION(BlueprintPure, Category="RPGAbilitySystemLibrary|GameplayEffects")
	static FGameplayTag GetDeBuffDamageType(const FGameplayEffectContextHandle& EffectContextHandle);

	//获取当前GE死亡冲击的方向和力度
	UFUNCTION(BlueprintPure, Category="RPGAbilitySystemLibrary|GameplayEffects")
	static FVector GetDeathImpulse(const FGameplayEffectContextHandle& EffectContextHandle);

	//获取当前GE攻击击退的方向和力度
	UFUNCTION(BlueprintPure, Category="RPGAbilitySystemLibrary|GameplayEffects")
	static FVector GetKnockbackForce(const FGameplayEffectContextHandle& EffectContextHandle);

	//设置GE是否触发格挡
	UFUNCTION(BlueprintCallable, Category="RPGAbilitySystemLibrary|GameplayEffects")
	static void SetIsBlockHit(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, bool bInIsBlockedHit);

	//设置GE是否触发暴击
	UFUNCTION(BlueprintCallable, Category="RPGAbilitySystemLibrary|GameplayEffects")
	static void SetIsCriticalHit(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, bool bInIsCriticalHit);

	//设置GE是否应用负面效果
	UFUNCTION(BlueprintCallable, Category="RPGAbilitySystemLibrary|GameplayEffects")
	static void SetIsSuccessfulDeBuff(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, bool bInIsSuccessfulDeBuff);

	//设置GE负面效果相关数值 负面效果伤害类型 负面效果伤害 负面效果持续时间 负面效果触发间隔时间
	UFUNCTION(BlueprintCallable, Category="RPGAbilitySystemLibrary|GameplayEffects")
	static void SetDeBuff(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, FGameplayTag& InDamageType, float InDamage, float InDuration, float InFrequency);
	
	//设置GE死亡冲击的方向和力度
	UFUNCTION(BlueprintCallable, Category="RPGAbilitySystemLibrary|GameplayEffects")
	static void SetDeathImpulse(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, const FVector& InDeathImpulse);
	
	//设置GE攻击击退的方向和力度
	UFUNCTION(BlueprintCallable, Category="RPGAbilitySystemLibrary|GameplayEffects")
	static void SetKnockbackForce(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, const FVector& InKnockbackForce);
	
	//通过技能生成的负面配置项应用技能负面效果
	UFUNCTION(BlueprintCallable, Category="RPGAbilitySystemLibrary|GameplayEffects")
	static FGameplayEffectContextHandle ApplyDamageEffect(const FDamageEffectParams& DamageEffectParams);

	//获取到攻击位置半径内的所有动态Actor
	UFUNCTION(BlueprintCallable, Category="RPGAbilitySystemLibrary|GameplayMechanics", meta=(DefaultToSelf = "WorldContextObject"))
	static void GetLivePlayersWithinRadius(const UObject* WorldContextObject, TArray<AActor*>& OutOverlappingActors, const TArray<AActor*>& ActorsToIgnore, float Radius, const FVector& SphereOrigin);

	//获取双方是否为敌对势力
	UFUNCTION(BlueprintPure, Category="RPGAbilitySystemLibrary|GameplayMechanics")
	static bool IsNotFriend(const AActor* FirstActor, const AActor* SecondActor);
};
