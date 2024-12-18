// 版权归暮志未晚所有。

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "RPGAbilitySystemComponent.generated.h"

class ULoadScreenSaveGame;
class URPGAbilitySystemComponent;
DECLARE_MULTICAST_DELEGATE_OneParam(FEffectAssetTags, const FGameplayTagContainer& /* AssetTags */) //GE应用的委托回调
DECLARE_MULTICAST_DELEGATE(FAbilityGiven) //技能初始化应用后的回调委托
DECLARE_DELEGATE_OneParam(FForEachAbility, const FGameplayAbilitySpec&); //单播委托，只能绑定一个回调
DECLARE_MULTICAST_DELEGATE_ThreeParams(FAbilityStatusChanged, const FGameplayTag& /*技能标签*/, const FGameplayTag& /*技能状态标签*/, const int32 /*技能等级*/);
DECLARE_MULTICAST_DELEGATE_FourParams(FAbilityEquipped, const FGameplayTag& /*技能标签*/, const FGameplayTag& /*技能状态标签*/, const FGameplayTag& /*输入标签*/, const FGameplayTag& /*上一个输入标签*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FDeactivatePassiveAbility, const FGameplayTag& /*技能标签*/); //中止一个技能激活的回调
DECLARE_MULTICAST_DELEGATE_TwoParams(FActivePassiveEffect, const FGameplayTag& /*被动技能标签*/, bool /*激活或取消*/); //被动技能特效监听委托，对应特效是否开启

/**
 * 技能系统组件
 */
UCLASS()
class RPG_API URPGAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	//ASC初始化完成后的回调，在AvatarActor中设置完成ASC后调用
	void AbilityActorInfoSet(); 

	//GE应用的委托回调
	FEffectAssetTags EffectAssetTags;
	//技能初始化应用后的回调委托
	FAbilityGiven AbilityGivenDelegate;
	//角色升级后技能状态更新委托
	FAbilityStatusChanged AbilityStatusChanged;
	//技能装配更新回调
	FAbilityEquipped AbilityEquipped;
	//取消技能激活的委托
	FDeactivatePassiveAbility DeactivatePassiveAbility;
	//被动技能对应特效委托
	FActivePassiveEffect ActivatePassiveEffect; 

	//初始化应用技能后，此值将被设置为true，用于记录当前是否被初始化完成
	bool bStartupAbilitiesGiven = false;
	/**
	 * 从存档读取技能数据设置角色技能
	 * @param SaveGameData 需要读取的存档
	 */
	void AddCharacterAbilitiesFormSaveData(ULoadScreenSaveGame* SaveGameData);
	//应用主动技能
	void AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities);
	//装配并应用被动技能
	void AddCharacterPassiveAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupPassiveAbilities); 

	void AbilityInputTagPressed(const FGameplayTag& InputTag);
	void AbilityInputTagHold(const FGameplayTag& InputTag);
	void AbilityInputTagReleased(const FGameplayTag& InputTag);
	//遍历技能，并将技能广播出去
	void ForEachAbility(const FForEachAbility& Delegate); 

	//获取技能的标签
	static FGameplayTag GetAbilityTagFromSpec(const FGameplayAbilitySpec& AbilitySpec);
	//获取技能的输入标签
	static FGameplayTag GetInputTagFromSpec(const FGameplayAbilitySpec& AbilitySpec);
	//获取技能的状态标签
	static FGameplayTag GetStatusTagFromSpec(const FGameplayAbilitySpec& AbilitySpec); 

	//通过技能标签获取技能实例
	FGameplayAbilitySpec* GetSpecFromAbilityTag(const FGameplayTag& AbilityTag);
	//通过技能标签获取技能状态
	FGameplayTag GetStatusTagFromAbilityTag(const FGameplayTag& AbilityTag);
	//通过技能标签获取技能输入标签
	FGameplayTag GetInputTagFromAbilityTag(const FGameplayTag& AbilityTag);

	/**
	 * 切换技能实例的状态标签
	 * @param AbilitySpec 技能实例 
	 * @param StatusTag 需要切换的状态
	 */
	static void ToggleAbilityStatus(FGameplayAbilitySpec& AbilitySpec, const FGameplayTag& StatusTag);
	
	/**
	 * 判断技能槽位是否装配了技能
	 * @param Slot 技能槽位设置的技能输入标签 
	 * @return 如果槽位有技能，返回false,如果槽位是空的返回true
	 */
	bool SlotIsEmpty(const FGameplayTag& Slot);

	/**
	 * 判断技能设置的槽位
	 * @param Spec 需要判断的技能实例
	 * @param Slot 需要判断的槽位的输入标签
	 * @return 如果当前技能的输入标签和槽位的标签一致，返回true
	 */
	static bool AbilityHasSlot(const FGameplayAbilitySpec& Spec, const FGameplayTag& Slot);

	/**
	 * 判断技能是否装配到技能槽位上
	 * @param Spec 需要判断的技能
	 * @return 如果装配到了技能槽位，返回true
	 */
	static bool AbilityHasAnySlot(const FGameplayAbilitySpec& Spec);

	/**
	 * 通过插槽的技能输入标签获取技能实例
	 * @param Slot 插槽的输入标签
	 * @return 技能实例
	 */
	FGameplayAbilitySpec* GetSpecWithSlot(const FGameplayTag& Slot);

	/**
	 * 判断技能是否为被动技能
	 * @param Spec 技能实例
	 * @return 如果技能为被动技能，返回true
	 */
	bool IsPassiveAbility(const FGameplayAbilitySpec& Spec) const;

	/**
	 * 将技能应用到目标槽位
	 * @param Spec 目标技能
	 * @param Slot 需要应用到的槽位
	 */
	static void AssignSlotToAbility(FGameplayAbilitySpec& Spec, const FGameplayTag& Slot);

	/**
	 * 多网络被动特效委托广播，让每个客户端都可以看到特效
	 * @param AbilityTag 被动技能标签
	 * @param bActivate 激活或者关闭
	 */
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastActivatePassiveEffect(const FGameplayTag& AbilityTag, bool bActivate);

	//升级角色属性
	void UpgradeAttribute(const FGameplayTag& AttributeTag);

	//服务器升级属性函数
	UFUNCTION(Server, Reliable) 
	void ServerUpgradeAttribute(const FGameplayTag& AttributeTag); 

	//根据角色等级更新技能数据状态
	void UpdateAbilityStatuses(int32 Level); 

	//只在服务器端运行，消耗技能点函数提升技能等级
	UFUNCTION(Server, Reliable) 
	void ServerSpendSpellPoint(const FGameplayTag& AbilityTag); 

	//只在服务器端运行，降低技能返回技能点
	UFUNCTION(Server, Reliable) 
	void ServerDemotionSpellPoint(const FGameplayTag& AbilityTag); 

	//在服务器处理技能装配，传入技能标签和装配的技能标签
	UFUNCTION(Server, Reliable) 
	void ServerEquipAbility(const FGameplayTag& AbilityTag, const FGameplayTag& Slot); 

	//在客户端处理技能装配
	UFUNCTION(Client, Reliable) 
	void ClientEquipAbility(const FGameplayTag& AbilityTag, const FGameplayTag& Status, const FGameplayTag& Slot, const FGameplayTag& PreviousSlot);

	//在服务器重置所有的技能
	UFUNCTION(Server, Reliable)
	void ServerResetAllAbility(); 

	//通过标签获取技能描述
	bool GetDescriptionByAbilityTag(const FGameplayTag& AbilityTag, FString& OutDescription, FString& OutNextLevelDescription); 

	//清除技能装配插槽的技能
	static void ClearSlot(FGameplayAbilitySpec* Spec); 

	//根据输入标签，清除技能装配插槽的技能
	void ClearAbilitiesOfSlot(const FGameplayTag& Slot); 

	//判断当前技能实例是否处于目标技能装配插槽
	static bool AbilityHasSlot(FGameplayAbilitySpec* Spec, const FGameplayTag& Slot); 
	
protected:

	virtual void OnRep_ActivateAbilities() override;

	//标记Client告诉UE这个函数应该只在客户端运行，设置Reliable表示这个函数调用是可靠的，即它确保数据能够到达客户端
	UFUNCTION(Client, Reliable)
	void ClientEffectApplied(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveGameplayEffectHandle) const; //GE应用回调

	//技能状态更新后回调
	UFUNCTION(Client, Reliable)
	void ClientUpdateAbilityStatus(const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag, int32 AbilityLevel); 
};
