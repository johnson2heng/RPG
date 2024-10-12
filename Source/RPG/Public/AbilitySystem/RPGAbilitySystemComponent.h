// 版权归暮志未晚所有。

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "RPGAbilitySystemComponent.generated.h"

class URPGAbilitySystemComponent;
DECLARE_MULTICAST_DELEGATE_OneParam(FEffectAssetTags, const FGameplayTagContainer& /* AssetTags */) //GE应用的委托回调
DECLARE_MULTICAST_DELEGATE(FAbilityGiven) //技能初始化应用后的回调委托
DECLARE_DELEGATE_OneParam(FForEachAbility, const FGameplayAbilitySpec&); //单播委托，只能绑定一个回调
DECLARE_MULTICAST_DELEGATE_ThreeParams(FAbilityStatusChanged, const FGameplayTag& /*技能标签*/, const FGameplayTag& /*技能状态标签*/, const int32 /*技能等级*/);
DECLARE_MULTICAST_DELEGATE_FourParams(FAbilityEquipped, const FGameplayTag& /*技能标签*/, const FGameplayTag& /*技能状态标签*/, const FGameplayTag& /*输入标签*/, const FGameplayTag& /*上一个输入标签*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FDeactivatePassiveAbility, const FGameplayTag& /*技能标签*/); //中止一个技能激活的回调

/**
 * 技能系统组件
 */
UCLASS()
class RPG_API URPGAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	void AbilityActorInfoSet(); //ASC初始化完成后的回调，在AvatarActor中设置完成ASC后调用

	FEffectAssetTags EffectAssetTags; //GE应用的委托回调
	FAbilityGiven AbilityGivenDelegate; //技能初始化应用后的回调委托
	FAbilityStatusChanged AbilityStatusChanged; //角色升级后技能状态更新委托
	FAbilityEquipped AbilityEquipped; //技能装配更新回调
	FDeactivatePassiveAbility DeactivatePassiveAbility; //取消技能激活的委托
	
	bool bStartupAbilitiesGiven = false; //初始化应用技能后，此值将被设置为true，用于记录当前是否被初始化完成
	void AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities); //应用主动技能
	void AddCharacterPassiveAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupPassiveAbilities); //应用被动技能

	void AbilityInputTagPressed(const FGameplayTag& InputTag);
	void AbilityInputTagHold(const FGameplayTag& InputTag);
	void AbilityInputTagReleased(const FGameplayTag& InputTag);
	void ForEachAbility(const FForEachAbility& Delegate); //遍历技能，并将技能广播出去

	static FGameplayTag GetAbilityTagFromSpec(const FGameplayAbilitySpec& AbilitySpec); //获取技能的标签
	static FGameplayTag GetInputTagFromSpec(const FGameplayAbilitySpec& AbilitySpec); //获取技能的输入标签
	static FGameplayTag GetStatusTagFromSpec(const FGameplayAbilitySpec& AbilitySpec); //获取技能的状态标签

	FGameplayAbilitySpec* GetSpecFromAbilityTag(const FGameplayTag& AbilityTag); //通过技能标签获取技能实例
	FGameplayTag GetStatusTagFromAbilityTag(const FGameplayTag& AbilityTag); //通过技能标签获取技能状态
	FGameplayTag GetInputTagFromAbilityTag(const FGameplayTag& AbilityTag); //通过技能标签获取技能输入标签

	void UpgradeAttribute(const FGameplayTag& AttributeTag); //升级属性

	UFUNCTION(Server, Reliable) //服务器升级属性函数
	void ServerUpgradeAttribute(const FGameplayTag& AttributeTag); 

	void UpdateAbilityStatuses(int32 Level); //根据角色等级更新技能数据状态

	UFUNCTION(Server, Reliable) //只在服务器端运行，消耗技能点函数提升技能等级
	void ServerSpendSpellPoint(const FGameplayTag& AbilityTag); 

	UFUNCTION(Server, Reliable) //只在服务器端运行，降低技能返回技能点
	void ServerDemotionSpellPoint(const FGameplayTag& AbilityTag); 

	UFUNCTION(Server, Reliable) //在服务器处理技能装配，传入技能标签和装配的技能标签
	void ServerEquipAbility(const FGameplayTag& AbilityTag, const FGameplayTag& Slot); 

	UFUNCTION(Client, Reliable) //在客户端处理技能装配
	void ClientEquipAbility(const FGameplayTag& AbilityTag, const FGameplayTag& Status, const FGameplayTag& Slot, const FGameplayTag& PreviousSlot);

	UFUNCTION(Server, Reliable)
	void ServerResetAllAbility(); //在服务器重置所有的技能

	bool GetDescriptionByAbilityTag(const FGameplayTag& AbilityTag, FString& OutDescription, FString& OutNextLevelDescription); //通过标签获取技能描述

	void ClearSlot(FGameplayAbilitySpec* Spec); //清除技能装配插槽的技能

	void ClearAbilitiesOfSlot(const FGameplayTag& Slot); //根据输入标签，清除技能装配插槽的技能

	static bool AbilityHasSlot(FGameplayAbilitySpec* Spec, const FGameplayTag& Slot); //判断当前技能实例是否处于目标技能装配插槽
	
protected:

	virtual void OnRep_ActivateAbilities() override;

	//标记Client告诉UE这个函数应该只在客户端运行，设置Reliable表示这个函数调用是可靠的，即它确保数据能够到达客户端
	UFUNCTION(Client, Reliable)
	void ClientEffectApplied(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveGameplayEffectHandle) const; //GE应用回调

	UFUNCTION(Client, Reliable)
	void ClientUpdateAbilityStatus(const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag, int32 AbilityLevel); //技能状态更新后回调
};
