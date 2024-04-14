// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"// 包含核心模块的最小定义
#include "UObject/ObjectMacros.h"// 包含UObject的宏定义
#include "UObject/Object.h"// 包含UObject类的定义
#include "Templates/SubclassOf.h"// 包含子类模板的定义
#include "GameplayTagContainer.h"// 包含游戏标签容器的定义
#include "GameplayEffectTypes.h" // 包含游戏效果类型的声明
#include "GameplayAbilitySpec.h"// 包含游戏技能实例的定义
#include "GameplayEffect.h"// 包含游戏效果的定义
#include "Abilities/GameplayAbilityTypes.h"// 包含游戏技能类型的声明
#include "GameplayTaskOwnerInterface.h" // 包含游戏技能任务所有者接口的定义
#include "Abilities/GameplayAbilityTargetTypes.h"// 包含游戏技能目标类型的声明
#include "GameplayAbility.generated.h" // 包含游戏技能生成的头文件

// 声明一些与游戏技能相关的类
class UAbilitySystemComponent;
class UAnimMontage;
class UGameplayAbility;
class UGameplayTask;
class UGameplayTasksComponent;

// 定义一个结构体，用于在激活技能时启用日志记录
struct FScopedCanActivateAbilityLogEnabler
{
	FScopedCanActivateAbilityLogEnabler() { ++LogEnablerCounter; }// 构造函数，增加日志记录计数器

	~FScopedCanActivateAbilityLogEnabler() { --LogEnablerCounter; }// 析构函数，减少日志记录计数器

	static bool IsLoggingEnabled() { return LogEnablerCounter > 0; }// 静态方法，判断是否启用日志记录

private:

	static int32 LogEnablerCounter; // 日志记录计数器
};

/**
 * UGameplayAbility
 *	
 *	游戏技能定义了可以被激活或触发的自定义游戏逻辑。
 *	
 *	技能系统为游戏技能提供的主要功能包括：
 *		-可用性功能：
 *			-冷却时间
 *			-成本（法力、耐力等）
 *			-等等
 *			
 *		-复制支持
 *			-客户端/服务器之间的技能激活通信
 *			-客户端预测技能激活
 *			
 *		-实例化支持
 *			-技能可以是非实例化的（仅本地）
 *			-每个所有者实例化
 *			-每次执行实例化（默认）
 *			
 *		-基础的，可扩展的：
 *			-输入绑定
 *			-'给予' 技能（可以使用）给角色
 *	
 *
 *	有关非实例化技能的示例，请参阅GameplayAbility_Montage
 *		-播放蒙太奇并在播放蒙太奇时对其目标应用GameplayEffect。
 *		-完成后，移除GameplayEffect。
 *	
 *	关于复制支持的注意事项：
 *		-非实例化技能具有有限的复制支持。
 *			-不能有状态（显然），因此没有复制属性
 *			-技能类上的复制不能实现。
 *			
 *	要支持状态或事件复制，必须实例化技能。这可以通过InstancingPolicy属性完成。
 */

// 当游戏技能结束时的通知委托定义
DECLARE_MULTICAST_DELEGATE_OneParam(FOnGameplayAbilityEnded, UGameplayAbility*);

// 当游戏技能被取消时的通知委托定义
DECLARE_MULTICAST_DELEGATE(FOnGameplayAbilityCancelled);

// 用于在状态结束时通知技能状态任务的通知委托
DECLARE_MULTICAST_DELEGATE_OneParam(FOnGameplayAbilityStateEnded, FName);

// 用于延迟执行直到我们离开临界区的委托
DECLARE_DELEGATE(FPostLockDelegate);

// 定义一个结构体，用于定义技能如何由外部事件触发
USTRUCT()
struct FAbilityTriggerData
{
	GENERATED_USTRUCT_BODY()// 自动生成USTRUCT所需的代码

	FAbilityTriggerData() 
	: TriggerSource(EGameplayAbilityTriggerSource::GameplayEvent)// 构造函数，设置触发源为游戏事件
	{}

	/** 要响应的标签 */
	UPROPERTY(EditAnywhere, Category=TriggerData, meta=(Categories="TriggerTagCategory"))
	FGameplayTag TriggerTag;

	/** 要响应的触发类型 */
	UPROPERTY(EditAnywhere, Category=TriggerData)
	TEnumAsByte<EGameplayAbilityTriggerSource::Type> TriggerSource;
};

// 定义一个类，用于定义可以由玩家或外部游戏逻辑激活的自定义游戏逻辑
UCLASS(Blueprintable)
class GAMEPLAYABILITIES_API UGameplayAbility : public UObject, public IGameplayTaskOwnerInterface// 继承自UObject和IGameplayTaskOwnerInterface接口
{
	GENERATED_UCLASS_BODY()
	
	// 友元类声明，这些类可以在反射系统中访问UGameplayAbility的私有成员
	friend class UAbilitySystemComponent;
	friend class UGameplayAbilitySet;
	friend struct FScopedTargetListLock;

public:

	// ----------------------------------------------------------------------------------------------------------------
	//
	//	The important functions:这段注释解释了UGameplayAbility类中一些重要函数的用途和行为
	//	
	//		CanActivateAbility()	- 一个常量函数，用来检查技能是否可以被激活。可以被UI等调用。
	//
	//		TryActivateAbility()	- 尝试激活技能。会调用CanActivateAbility()。输入事件可以直接调用这个函数。
	//								- 也处理每次执行时的实例化逻辑以及复制/预测调用。
	//		
	//		CallActivateAbility()	- 受保护的非虚拟函数。做一些激活前的样板工作，然后调用ActivateAbility()。
	//
	//		ActivateAbility()		- 技能所做的操作。子类需要覆盖这个函数。
	//	
	//		CommitAbility()			- 提交资源/冷却时间等。ActivateAbility()必须调用这个函数！
	//		
	//		CancelAbility()			- 从外部源中断技能。
	//
	//		EndAbility()			- 结束技能。这应该是由技能自身调用以结束自己。
	//	
	// ----------------------------------------------------------------------------------------------------------------

	// --------------------------------------
	//	Accessors 访问器
	// --------------------------------------

	/** 返回技能在执行时是如何实例化的。这限制了技能在其实现中可以执行的操作。 */
	EGameplayAbilityInstancingPolicy::Type GetInstancingPolicy() const
	{
		return InstancingPolicy;
	}

	/** 返回技能如何在网络中复制状态/事件给每个人 */
	EGameplayAbilityReplicationPolicy::Type GetReplicationPolicy() const
	{
		return ReplicationPolicy;
	}

	/** 技能在网络中在哪里执行？客户端是“询问并预测”、“询问并等待”还是“不问（直接执行）”  */
	EGameplayAbilityNetExecutionPolicy::Type GetNetExecutionPolicy() const
	{
		return NetExecutionPolicy;
	}

	/** 技能应该在网络中哪里执行？提供保护，防止客户端尝试执行受限的技能。 */
	EGameplayAbilityNetSecurityPolicy::Type GetNetSecurityPolicy() const
	{
		return NetSecurityPolicy;
	}

	/** 返回与此技能关联的演员信息，缓存了指向有用对象的指针 */
	UFUNCTION(BlueprintCallable, Category=Ability)
	FGameplayAbilityActorInfo GetActorInfo() const;

	/** 返回拥有此技能的演员，该演员可能没有物理位置 */
	UFUNCTION(BlueprintCallable, Category = Ability)
	AActor* GetOwningActorFromActorInfo() const;

	/** 返回正在执行此技能的物理演员。可能为null */
	UFUNCTION(BlueprintCallable, Category = Ability)
	AActor* GetAvatarActorFromActorInfo() const;

	/** 对于技能来说，获取骨骼网格组件的便捷方法 - 对于动画技能非常有用 */
	UFUNCTION(BlueprintCallable, DisplayName = "GetSkeletalMeshComponentFromActorInfo", Category = Ability)
	USkeletalMeshComponent* GetOwningComponentFromActorInfo() const;

	/** 返回正在激活此技能的 AbilitySystemComponent */
	UFUNCTION(BlueprintCallable, Category = Ability)
	UAbilitySystemComponent* GetAbilitySystemComponentFromActorInfo() const;
    // 以下是对GetAbilitySystemComponentFromActorInfo的重载，提供了额外的检查和确保机制
	UAbilitySystemComponent* GetAbilitySystemComponentFromActorInfo_Checked() const;
	UAbilitySystemComponent* GetAbilitySystemComponentFromActorInfo_Ensured() const;

	/** 获取当前绑定到此技能的演员信息 - 只能在技能实例中调用 */
	const FGameplayAbilityActorInfo* GetCurrentActorInfo() const;

	/** 获取当前绑定到此技能的激活信息 - 只能在技能实例中调用 */
	FGameplayAbilityActivationInfo GetCurrentActivationInfo() const;

	/** 获取当前绑定到此技能的激活信息的引用 - 只能在技能实例中调用 */
	FGameplayAbilityActivationInfo& GetCurrentActivationInfoRef()
	{
        // 检查是否为实例化的技能，如果不是，则抛出错误
		checkf(IsInstantiated(), TEXT("%s: GetCurrentActivationInfoRef cannot be called on a non-instanced ability. Check the instancing policy."), *GetPathName());
		return CurrentActivationInfo;
	}

	/** 获取当前的AbilitySpecHandle - 只能在技能实例中调用 */
	FGameplayAbilitySpecHandle GetCurrentAbilitySpecHandle() const;

	/** 检索此技能的实际操作AbilitySpec。只能在技能实例中调用。 */
	FGameplayAbilitySpec* GetCurrentAbilitySpec() const;

	/** 检索授予此技能的GameplayEffect的EffectContext。只能在技能实例中调用。 */
	UFUNCTION(BlueprintCallable, Category = Ability)
	FGameplayEffectContextHandle GetGrantedByEffectContext() const;

	/**从我们的所有者和可选的TargetData生成GameplayEffectContextHandle。*/
	UFUNCTION(BlueprintCallable, Category = Ability)
	virtual FGameplayEffectContextHandle GetContextFromOwner(FGameplayAbilityTargetDataHandle OptionalTargetData) const;

	/** 根据指定的演员信息返回一个效果上下文 */
	virtual FGameplayEffectContextHandle MakeEffectContext(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo *ActorInfo) const;

	/** 便捷方法，用于技能获取传出的游戏效果规格（例如，传递给投射物，以便应用于它们击中的任何人） */
	UFUNCTION(BlueprintCallable, Category=Ability)
	FGameplayEffectSpecHandle MakeOutgoingGameplayEffectSpec(TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level=1.f) const;

	/** 上述函数的本地版本 */
	virtual FGameplayEffectSpecHandle MakeOutgoingGameplayEffectSpec(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level = 1.f) const;

	/** 将技能的标签添加到给定的GameplayEffectSpec中。这可能会根据项目被重写。 */
	virtual void ApplyAbilityTagsToGameplayEffectSpec(FGameplayEffectSpec& Spec, FGameplayAbilitySpec* AbilitySpec) const;

	/** 返回技能当前是否处于激活状态 */
	bool IsActive() const;

	/** 此技能是否由TriggerData触发（或者是通过输入/游戏代码显式触发的） */
	bool IsTriggered() const;

	/** 此技能是否在预测客户端上运行，单人游戏中这是假的 */
	bool IsPredictingClient() const;

	/** 如果这是在服务器上并且正在为非本地玩家执行，则为真，单人游戏中这是假的 */
	bool IsForRemoteClient() const;

	/** 如果拥有者演员是本地控制的，则为真，在单人游戏中为真 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = Ability, Meta = (ExpandBoolAsExecs = "ReturnValue"))
	bool IsLocallyControlled() const;

	/** 如果这是服务器或者单人游戏，则为真 */
	bool HasAuthority(const FGameplayAbilityActivationInfo* ActivationInfo) const;

    //HasAuthority的蓝图版本
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = Ability, DisplayName = "HasAuthority", Meta = (ExpandBoolAsExecs = "ReturnValue"))
	bool K2_HasAuthority() const;

	/** 如果我们能有效的预测键并且预期会工作，则为真 */
	bool HasAuthorityOrPredictionKey(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo* ActivationInfo) const;

	/** 如果已经被实例化，则为真，对于蓝图总是为真 */
	bool IsInstantiated() const;

	/** 技能结束时的通知。通过TryActivateAbility设置。 */
	FOnGameplayAbilityEnded OnGameplayAbilityEnded;

	/** 技能以某种方式结束时的通知 */
	FGameplayAbilityEndedDelegate OnGameplayAbilityEndedWithData;

	/** 技能被取消时的通知。在OnGameplayAbilityEnded之前调用。 */
	FOnGameplayAbilityCancelled OnGameplayAbilityCancelled;

	/** 技能状态任务用来处理状态结束时的事件 */
	FOnGameplayAbilityStateEnded OnGameplayAbilityStateEnded;

	/** 当这个技能被服务器确认时的回调 */
	FGenericAbilityDelegate	OnConfirmDelegate;

	// --------------------------------------
	//	CanActivateAbility 能否激活技能
	// --------------------------------------

	/** 返回如果当前可以激活此技能，则为真。没有副作用 */
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const;

	/** 返回如果此技能现在可以被触发，则为真。没有副作用 */
	virtual bool ShouldAbilityRespondToEvent(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayEventData* Payload) const;

	/** 返回是否应该激活此技能 */
	virtual bool ShouldActivateAbility(ENetRole Role) const;

	/** 返回当前激活的冷却时间剩余的秒数。 */
	UFUNCTION(BlueprintCallable, Category = Ability)
	float GetCooldownTimeRemaining() const;

	/** 返回当前激活的冷却时间剩余的秒数。 */
	virtual float GetCooldownTimeRemaining(const FGameplayAbilityActorInfo* ActorInfo) const;

	/** 返回当前激活的冷却时间剩余的秒数和此冷却的原始持续时间。 */
	virtual void GetCooldownTimeRemainingAndDuration(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, float& TimeRemaining, float& CooldownDuration) const;

	/** 返回所有可以将此技能置入冷却的标签 */
	virtual const FGameplayTagContainer* GetCooldownTags() const;
	
	/** 如果技能的标签没有被封锁，并且没有“阻塞”标签，并且具有所有“必需”标签，则返回真。 */
	virtual bool DoesAbilitySatisfyTagRequirements(const UAbilitySystemComponent& AbilitySystemComponent, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const;

	/** 如果此技能正在阻止其他技能，则返回真 */
	virtual bool IsBlockingOtherAbilities() const;

	/** 设置技能阻塞标志是否启用或禁用。仅对实例化的技能有效 */
	UFUNCTION(BlueprintCallable, Category = Ability)
	virtual void SetShouldBlockOtherAbilities(bool bShouldBlockAbilities);

	// --------------------------------------
	//	CancelAbility 取消技能
	// --------------------------------------

	/** 销毁每次执行时实例化的技能。每个执行对应一个实例的技能应该“重置”。任何活跃的技能状态任务都会接收到'OnAbilityStateInterrupted'事件。非实例化的技能 - 我们能做什么？ */
	virtual void CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility);

	/** 从蓝图调用以自然取消技能 */
	UFUNCTION(BlueprintCallable, Category = Ability, DisplayName = "CancelAbility", meta=(ScriptName = "CancelAbility"))
	void K2_CancelAbility();

	/** 返回此技能是否可以被取消 */
	virtual bool CanBeCanceled() const;

	/** 设置技能是否应该忽略取消请求。仅对实例化的技能有效 */
	UFUNCTION(BlueprintCallable, Category=Ability)
	virtual void SetCanBeCanceled(bool bCanBeCanceled);

	// --------------------------------------
	//	CommitAbility 提交技能 的消耗和冷却
	// --------------------------------------

	/** 尝试提交技能（消耗资源等）。这是我们最后的机会来失败。覆盖ActivateAbility的子类必须自己调用这个函数！ */
	UFUNCTION(BlueprintCallable, Category = Ability, DisplayName = "CommitAbility", meta=(ScriptName = "CommitAbility"))
	virtual bool K2_CommitAbility();

	/** 仅尝试提交技能的冷却时间。如果BroadcastCommitEvent为true，它将广播提交事件，像WaitAbilityCommit这样的任务会监听这个事件。 */
	UFUNCTION(BlueprintCallable, Category = Ability, DisplayName = "CommitAbilityCooldown", meta=(ScriptName = "CommitAbilityCooldown"))
	virtual bool K2_CommitAbilityCooldown(bool BroadcastCommitEvent=false, bool ForceCooldown=false);

	/** 仅尝试提交技能的成本。如果BroadcastCommitEvent为true，它将广播提交事件，像WaitAbilityCommit这样的任务会监听这个事件。 */
	UFUNCTION(BlueprintCallable, Category = Ability, DisplayName = "CommitAbilityCost", meta=(ScriptName = "CommitAbilityCost"))
	virtual bool K2_CommitAbilityCost(bool BroadcastCommitEvent=false);

	/** 检查技能的冷却时间，但不应用它。*/
	UFUNCTION(BlueprintCallable, Category = Ability, DisplayName = "CheckAbilityCooldown", meta=(ScriptName = "CheckAbilityCooldown"))
	virtual bool K2_CheckAbilityCooldown();

	/** 检查技能的成本，但不应用它。 */
	UFUNCTION(BlueprintCallable, Category = Ability, DisplayName = "CheckAbilityCost", meta=(ScriptName = "CheckAbilityCost"))
	virtual bool K2_CheckAbilityCost();

    // 以下是一些虚拟函数，用于在提交技能时执行特定的检查和操作
	virtual bool CommitAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr);
	virtual bool CommitAbilityCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const bool ForceCooldown, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr);
	virtual bool CommitAbilityCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr);

	/** 在提交之前的最后机会失败，这通常会与CanActivateAbility相同。如果它们在CommitExecute中消耗了额外的资源，一些技能可能需要在这里进行额外的检查 */
	virtual bool CommitCheck(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr);

	/** 从蓝图调用函数CommitAbility */
	UFUNCTION(BlueprintImplementableEvent, Category = Ability, DisplayName = "CommitExecute", meta = (ScriptName = "CommitExecute"))
	void K2_CommitExecute();

	/** 原子性地执行提交（消耗资源、执行冷却时间等） */
	virtual void CommitExecute(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo);

	/** 返回用于确定冷却时间的游戏效果 */
	virtual UGameplayEffect* GetCooldownGameplayEffect() const;

	/** 返回用于应用成本的游戏效果 */
	virtual UGameplayEffect* GetCostGameplayEffect() const;

	/** 检查冷却时间。如果可以再次使用则返回真，否则返回假 */
	virtual bool CheckCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const;

	/** 将CooldownGameplayEffect应用到目标 */
	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const;

	/** 检查成本。如果可以支付技能则返回真，否则返回假 */
	virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const;

	/** 将技能的成本应用到目标 */
	virtual void ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const;

	// --------------------------------------
	//	Input 输入 InputPressed和InputReleased函数是虚函数，它们作为输入绑定的存根存在。在实际的游戏逻辑中，这些函数通常会被重写以响应玩家的输入事件
	// --------------------------------------

	/** 触发技能按下 */
	virtual void InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) {};

	/** 触发技能抬起 */
	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) {};

	/** 从AbilityTask_WaitConfirmCancel调用，用于处理输入确认
     * OnWaitingForConfirmInputBegin和OnWaitingForConfirmInputEnd函数用于处理等待确认输入的情况。这些函数可能在技能激活的确认/取消逻辑中被调用，例如在玩家需要按下按钮来确认或取消技能激活时。
     */
	virtual void OnWaitingForConfirmInputBegin() {}
	virtual void OnWaitingForConfirmInputEnd() {}

	// --------------------------------------
	//	Animation 动画
	// --------------------------------------

	/** 用于获取当前与技能关联的蒙太奇动画（如果有的话） */
	UFUNCTION(BlueprintCallable, Category = Animation)
	UAnimMontage* GetCurrentMontage() const;

	/** 设置当前的蒙太奇动画，这通常用于将蒙太奇事件与技能事件关联起来，例如在技能激活时播放特定的动画序列 */
	virtual void SetCurrentMontage(class UAnimMontage* InCurrentMontage);

	/** 设置移动同步点，但由于在Unreal Engine 5.3版本中没有实际用途，已被标记为弃用（deprecated），并计划在未来的引擎版本中移除。 */
	UE_DEPRECATED(5.3, "This serves no purpose and will be removed in future engine versions")
	virtual void SetMovementSyncPoint(FName SyncName);

	// ----------------------------------------------------------------------------------------------------------------
	//	Ability Levels and source objects 技能等级和源对象
	// ----------------------------------------------------------------------------------------------------------------
	
	/** 获取当前技能的等级 */
	UFUNCTION(BlueprintCallable, Category = Ability)
	int32 GetAbilityLevel() const;

	/** 返回非实例化技能的当前等级。你必须在这些上下文中调用这个版本！ */
	int32 GetAbilityLevel(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo) const;

	/** GetAbilityLevel的蓝图版本 */
	UFUNCTION(BlueprintCallable, Category = Ability, meta = (DisplayName = "GetAbilityLevelNonInstanced", ReturnDisplayName = "AbilityLevel"))
	int32 GetAbilityLevel_BP(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo& ActorInfo) const;

	/** 检索与此技能关联的SourceObject。只能在实例化的技能上调用。 */
	UFUNCTION(BlueprintCallable, Category = Ability)
	UObject* GetCurrentSourceObject() const;

	/** 检索与此技能关联的SourceObject。可以在非实例化的技能上调用。 */
	UObject* GetSourceObject(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo) const;

	/** GetSourceObject的蓝图版本 */
	UFUNCTION(BlueprintCallable, Category = Ability, meta = (DisplayName = "GetSourceObjectNonInstanced", ReturnDisplayName = "SourceObject"))
	UObject* GetSourceObject_BP(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo& ActorInfo) const;

	// --------------------------------------
	//	Interaction with ability system component 与ASC的交互
	// --------------------------------------

	/** ASC调用此函数以通知此技能实例远程实例已结束 */
	virtual void SetRemoteInstanceHasEnded();

	/** 通知技能AvatarActor已被替换。如果技能依赖于avatar状态，它可能需要结束自己 */
	virtual void NotifyAvatarDestroyed();

	/** 通知技能有任务正在等待玩家执行某些操作 */
	virtual void NotifyAbilityTaskWaitingOnPlayerData(class UAbilityTask* AbilityTask);

	/** 通知技能有任务正在等待玩家的avatar在世界中执行某些操作 */
	virtual void NotifyAbilityTaskWaitingOnAvatar(class UAbilityTask* AbilityTask);

	/** 当技能被赋予ASC时调用 */
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec);

	/** 当技能从ASC中移除时调用 */
	virtual void OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) {}

	/** 当avatar actor被设置/更改时调用 */
	virtual void OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec);

	/** 它接收技能规格（ability spec），并检查是否应该允许该技能规格上的复制。这不会阻止技能UObject本身的复制，而只是阻止位于UAbilitySystemComponent的ActivatableAbilities内部针对这个技能的规格复制。 */
	virtual bool ShouldReplicateAbilitySpec(const FGameplayAbilitySpec& AbilitySpec) const
	{
		return true;
	}

	/** 
	 * 使当前预测键无效。这应该用在有有效预测窗口的情况下，但服务器正在执行只有它能做的逻辑，之后执行客户端本来可以预测的动作（如果客户端能够先运行服务器独有的代码）。
     * 这个函数会立即返回，除了清除当前预测键之外没有其他副作用。
	 */ 
	UFUNCTION(BlueprintCallable, Category = Ability)
	void InvalidateClientPredictionKey() const;

	/** 移除授予此技能的GameplayEffect。只能在实例化的技能上调用 */
	UFUNCTION(BlueprintCallable, Category = Ability)
	virtual void RemoveGrantedByEffect();

	/** 向用户添加一个调试消息 */
	void AddAbilityTaskDebugMessage(UGameplayTask* AbilityTask, FString DebugMessage);

	// --------------------------------------
	//	公共变量，为了向后兼容性而暴露 
	// --------------------------------------

	/** 这个技能有这些标签 */
	UPROPERTY(EditDefaultsOnly, Category = Tags, meta=(Categories="AbilityTagCategory"))
	FGameplayTagContainer AbilityTags;

	/** 如果为真，这个技能将总是将输入按下/释放事件复制到服务器。 */
	UPROPERTY(EditDefaultsOnly, Category = Input)
	bool bReplicateInputDirectly;

	/** 当这个技能的远程实例结束时设置（但本地实例可能仍在运行或完成中） */
	UPROPERTY()
	bool RemoteInstanceEnded;

	// --------------------------------------
	//	覆写 UObject 成员函数
	// --------------------------------------	
	virtual UWorld* GetWorld() const override;
	virtual int32 GetFunctionCallspace(UFunction* Function, FFrame* Stack) override;
	virtual bool CallRemoteFunction(UFunction* Function, void* Parameters, FOutParmRec* OutParms, FFrame* Stack) override;
	virtual bool IsSupportedForNetworking() const override;

#if UE_WITH_IRIS
	/** Register all replication fragments */
	virtual void RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags) override;
#endif // UE_WITH_IRIS

	// --------------------------------------
	//	IGameplayTaskOwnerInterface
	// --------------------------------------	
	virtual UGameplayTasksComponent* GetGameplayTasksComponent(const UGameplayTask& Task) const override;
	virtual AActor* GetGameplayTaskOwner(const UGameplayTask* Task) const override;
	virtual AActor* GetGameplayTaskAvatar(const UGameplayTask* Task) const override;
	virtual void OnGameplayTaskInitialized(UGameplayTask& Task) override;
	virtual void OnGameplayTaskActivated(UGameplayTask& Task) override;
	virtual void OnGameplayTaskDeactivated(UGameplayTask& Task) override;

protected:

	// --------------------------------------
	//	ShouldAbilityRespondToEvent
	// --------------------------------------

	/** 如果这个技能现在可以被激活，则返回true。没有副作用，蓝图可调用 */
	UFUNCTION(BlueprintImplementableEvent, Category = Ability, DisplayName = "ShouldAbilityRespondToEvent", meta=(ScriptName = "ShouldAbilityRespondToEvent"))
	bool K2_ShouldAbilityRespondToEvent(FGameplayAbilityActorInfo ActorInfo, FGameplayEventData Payload) const;

    /** 一个标志，表示是否在蓝图（Blueprint）中实现了 ShouldAbilityRespondToEvent 事件 */  
	bool bHasBlueprintShouldAbilityRespondToEvent;

	/** 发送一个游戏事件，并创建一个预测窗口 */
	UFUNCTION(BlueprintCallable, Category = Ability)
	virtual void SendGameplayEvent(FGameplayTag EventTag, FGameplayEventData Payload);

	// --------------------------------------
	//	CanActivate
	// --------------------------------------
	
	/** 如果当前可以激活这个技能，则返回true。这个函数没有副作用。 */
	UFUNCTION(BlueprintImplementableEvent, Category = Ability, DisplayName="CanActivateAbility", meta=(ScriptName="CanActivateAbility"))
	bool K2_CanActivateAbility(FGameplayAbilityActorInfo ActorInfo, const FGameplayAbilitySpecHandle Handle, FGameplayTagContainer& RelevantTags) const;

	//其用途可能是在类的其他部分或蓝图中追踪是否有一个蓝图实现了CanActivateAbility事件。这样，你可以根据这个标志来决定是否调用蓝图中的实现或默认的C++实现。
	bool bHasBlueprintCanUse;

	// --------------------------------------
	//	ActivateAbility 激活技能
	// --------------------------------------

	/**
	 * 定义技能主要功能的功能。
	 *  -子类可以覆盖此类的实现
	 *  -函数实现需要调用CommitAbility
	 *  -函数实现需要调用EndAbility
	 *  
	 *  在这个图中允许使用延迟/异步动作。注意，Commit和EndAbility调用要求适用于K2_ActivateAbility图。
	 *  在C++中，调用K2_ActivateAbility()可能会在没有调用CommitAbility或EndAbility的情况下返回。
	 *  但是预计这只会在有延迟/异步动作待处理时发生。当K2_ActivateAbility逻辑上完成时，我们预期Commit/End已经被调用。
	 *  
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = Ability, DisplayName = "ActivateAbility", meta=(ScriptName = "ActivateAbility"))
	void K2_ActivateAbility();

	UFUNCTION(BlueprintImplementableEvent, Category = Ability, DisplayName = "ActivateAbilityFromEvent", meta=(ScriptName = "ActivateAbilityFromEvent"))
	void K2_ActivateAbilityFromEvent(const FGameplayEventData& EventData);

	bool bHasBlueprintActivate;
	bool bHasBlueprintActivateFromEvent;

	/** 实际激活技能，不要直接调用这个函数 */
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData);

	/** 执行初始化工作，然后调用ActivateAbility */
	virtual void PreActivate(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, FOnGameplayAbilityEnded::FDelegate* OnGameplayAbilityEndedDelegate, const FGameplayEventData* TriggerEventData = nullptr);

	/** 执行PreActivate和ActivateAbility */
	void CallActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, FOnGameplayAbilityEnded::FDelegate* OnGameplayAbilityEndedDelegate = nullptr, const FGameplayEventData* TriggerEventData = nullptr);

	/** 当服务器确认其执行时，对预测技能调用 */
	virtual void ConfirmActivateSucceed();

	// -------------------------------------
	//	EndAbility 结束技能
	// -------------------------------------
	/** 从蓝图调用以强制结束技能，而不取消它。这将会复制结束技能到客户端或服务器，可以中断任务 */
	UFUNCTION(BlueprintCallable, Category = Ability, DisplayName="End Ability", meta=(ScriptName = "EndAbility"))
	virtual void K2_EndAbility();

	/** 从蓝图调用以自然结束技能。这只会在本地结束预测的技能，允许它在客户端或服务器上自然结束 */
	UFUNCTION(BlueprintCallable, Category = Ability, DisplayName = "End Ability Locally", meta = (ScriptName = "EndAbilityLocally"))
	virtual void K2_EndAbilityLocally();

	/** 蓝图事件，如果技能正常或异常结束将被调用 */
	UFUNCTION(BlueprintImplementableEvent, Category = Ability, DisplayName = "OnEndAbility", meta=(ScriptName = "OnEndAbility"))
	void K2_OnEndAbility(bool bWasCancelled);

	/** 检查技能是否可以结束 */
	bool IsEndAbilityValid(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo) const;

	/** 原生函数，如果技能正常或异常结束将被调用。如果bReplicate设置为true，尝试将结束复制到客户端/服务器 */
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled);

	// -------------------------------------
	//  Apply Gameplay effects to Self 应用游戏效果到自身
	// -------------------------------------

	/** 将一个游戏效果应用到这个技能的拥有者上 */
	UFUNCTION(BlueprintCallable, Category = Ability, DisplayName="ApplyGameplayEffectToOwner", meta=(ScriptName="ApplyGameplayEffectToOwner"))
	FActiveGameplayEffectHandle BP_ApplyGameplayEffectToOwner(TSubclassOf<UGameplayEffect> GameplayEffectClass, int32 GameplayEffectLevel = 1, int32 Stacks = 1);

	/** 不能从蓝图调用，可以安全地在CDO/非实例化技能上调用 */
	FActiveGameplayEffectHandle ApplyGameplayEffectToOwner(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const UGameplayEffect* GameplayEffect, float GameplayEffectLevel, int32 Stacks = 1) const;

	/** 将之前创建的游戏效果规格应用到这个技能的拥有者上 */
	UFUNCTION(BlueprintCallable, Category = Ability, DisplayName = "ApplyGameplayEffectSpecToOwner", meta=(ScriptName = "ApplyGameplayEffectSpecToOwner"))
	FActiveGameplayEffectHandle K2_ApplyGameplayEffectSpecToOwner(const FGameplayEffectSpecHandle EffectSpecHandle);

	FActiveGameplayEffectHandle ApplyGameplayEffectSpecToOwner(const FGameplayAbilitySpecHandle AbilityHandle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEffectSpecHandle SpecHandle) const;

	// -------------------------------------
	//  Apply Gameplay effects to Target 应用GE到目标
	// -------------------------------------

	/** 将一个游戏效果应用到目标 */
	UFUNCTION(BlueprintCallable, Category = Ability, DisplayName = "ApplyGameplayEffectToTarget", meta=(ScriptName = "ApplyGameplayEffectToTarget"))
	TArray<FActiveGameplayEffectHandle> BP_ApplyGameplayEffectToTarget(FGameplayAbilityTargetDataHandle TargetData, TSubclassOf<UGameplayEffect> GameplayEffectClass, int32 GameplayEffectLevel = 1, int32 Stacks = 1);

	/** 不能从蓝图调用，可以安全地在CDO/非实例化技能上调用 */
	TArray<FActiveGameplayEffectHandle> ApplyGameplayEffectToTarget(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayAbilityTargetDataHandle& Target, TSubclassOf<UGameplayEffect> GameplayEffectClass, float GameplayEffectLevel, int32 Stacks = 1) const;

	/** 将之前创建的游戏效果规格应用到目标 */
	UFUNCTION(BlueprintCallable, Category = Ability, DisplayName = "ApplyGameplayEffectSpecToTarget", meta=(ScriptName = "ApplyGameplayEffectSpecToTarget"))
	TArray<FActiveGameplayEffectHandle> K2_ApplyGameplayEffectSpecToTarget(const FGameplayEffectSpecHandle EffectSpecHandle, FGameplayAbilityTargetDataHandle TargetData);

	TArray<FActiveGameplayEffectHandle> ApplyGameplayEffectSpecToTarget(const FGameplayAbilitySpecHandle AbilityHandle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEffectSpecHandle SpecHandle, const FGameplayAbilityTargetDataHandle& TargetData) const;

	// -------------------------------------
	//  Remove Gameplay effects from Self 从自身移除效果
	// -------------------------------------
	
	/** 移除拥有者上与给定资产等级标签匹配的游戏效果 */
	UFUNCTION(BlueprintCallable, Category = Ability, DisplayName="RemoveGameplayEffectFromOwnerWithAssetTags", meta=(ScriptName="RemoveGameplayEffectFromOwnerWithAssetTags"))
	void BP_RemoveGameplayEffectFromOwnerWithAssetTags(FGameplayTagContainer WithAssetTags, int32 StacksToRemove = -1);

	/** 用于移除拥有者上授予的、与特定标签容器匹配的游戏效果 */
	UFUNCTION(BlueprintCallable, Category = Ability, DisplayName="RemoveGameplayEffectFromOwnerWithGrantedTags", meta=(ScriptName="RemoveGameplayEffectFromOwnerWithGrantedTags"))
	void BP_RemoveGameplayEffectFromOwnerWithGrantedTags(FGameplayTagContainer WithGrantedTags, int32 StacksToRemove = -1);

	/** 允许从蓝图中调用，用于移除与特定FActiveGameplayEffectHandle句柄匹配的游戏效果。 */
	UFUNCTION(BlueprintCallable, Category = Ability, DisplayName = "RemoveGameplayEffectFromOwnerWithHandle", meta=(ScriptName = "RemoveGameplayEffectFromOwnerWithHandle"))
	void BP_RemoveGameplayEffectFromOwnerWithHandle(FActiveGameplayEffectHandle Handle, int32 StacksToRemove = -1);

	// -------------------------------------
	//	GameplayCue 游戏提示效果
	//	技能可以在不创建GE的情况下调用GC
	// -------------------------------------

	/** 在技能拥有者上触发一个游戏提示 */
	UFUNCTION(BlueprintCallable, Category = Ability, meta=(GameplayTagFilter="GameplayCue"), DisplayName="Execute GameplayCue On Owner", meta=(ScriptName="ExecuteGameplayCue"))
	virtual void K2_ExecuteGameplayCue(FGameplayTag GameplayCueTag, FGameplayEffectContextHandle Context);

	/** 在技能拥有者上触发一个带有额外参数的游戏提示 */
	UFUNCTION(BlueprintCallable, Category = Ability, meta = (GameplayTagFilter = "GameplayCue"), DisplayName = "Execute GameplayCueWithParams On Owner", meta=(ScriptName = "ExecuteGameplayCueWithParams"))
	virtual void K2_ExecuteGameplayCueWithParams(FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters);

	/** 向技能拥有者添加一个持久的游戏提示。可选地，如果技能结束，则移除 */
	UFUNCTION(BlueprintCallable, Category = Ability, meta=(GameplayTagFilter="GameplayCue"), DisplayName="Add GameplayCue To Owner", meta=(ScriptName="AddGameplayCue"))
	virtual void K2_AddGameplayCue(FGameplayTag GameplayCueTag, FGameplayEffectContextHandle Context, bool bRemoveOnAbilityEnd = true);

	/** 向技能拥有者添加一个持久的游戏提示。可选地，如果技能结束，则移除 */
	UFUNCTION(BlueprintCallable, Category = Ability, meta = (GameplayTagFilter = "GameplayCue"), DisplayName = "Add GameplayCueWithParams To Owner", meta=(ScriptName = "AddGameplayCueWithParams"))
	virtual void K2_AddGameplayCueWithParams(FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParameter, bool bRemoveOnAbilityEnd = true);

	/** 从技能拥有者移除一个持久的游戏提示 */
	UFUNCTION(BlueprintCallable, Category = Ability, meta=(GameplayTagFilter="GameplayCue"), DisplayName="Remove GameplayCue From Owner", meta=(ScriptName="RemoveGameplayCue"))
	virtual void K2_RemoveGameplayCue(FGameplayTag GameplayCueTag);

	// -------------------------------------
	//	Protected properties 受保护的属性，这些属性用于控制技能在网络上的复制、实例化策略以及消耗和冷却类的设置。这些数值可以在蓝图中设置。
	// -------------------------------------

	/** 技能如何在网络上复制状态/事件给每个人。对于NetExecutionPolicy不需要复制。 */
	UPROPERTY(EditDefaultsOnly, Category = Advanced)
	TEnumAsByte<EGameplayAbilityReplicationPolicy::Type> ReplicationPolicy;

	/** 技能在执行时是如何实例化的。这限制了技能在其实现中可以执行的操作。 */
	UPROPERTY(EditDefaultsOnly, Category = Advanced)
	TEnumAsByte<EGameplayAbilityInstancingPolicy::Type>	InstancingPolicy;

	/** 如果设置了，技能服务器端版本可以被客户端版本取消。客户端版本总是可以被服务器取消。 */
	UPROPERTY(EditDefaultsOnly, Category = Advanced)
	bool bServerRespectsRemoteAbilityCancellation;

	/** 如果为true，并且尝试激活一个已经激活的实例化技能，结束它并重新触发它。 */
	UPROPERTY(EditDefaultsOnly, Category = Advanced)
	bool bRetriggerInstancedAbility;

	/** 这是特定于技能实例的信息。例如，它是否在预测、授权、确认等。 */
	UPROPERTY(BlueprintReadOnly, Category = Ability)
	FGameplayAbilityActivationInfo	CurrentActivationInfo;

	/** 如果是通过事件激活的技能，这是特定于此技能实例的信息 */
	UPROPERTY(BlueprintReadOnly, Category = Ability)
	FGameplayEventData CurrentEventData;

	/** 技能在网络上是如何执行的。客户端是“询问并预测”、“询问并等待”还是“不问（直接执行）”。 */
	UPROPERTY(EditDefaultsOnly, Category=Advanced)
	TEnumAsByte<EGameplayAbilityNetExecutionPolicy::Type> NetExecutionPolicy;

	/** 这个技能有哪些保护措施？客户端是否被允许请求更改技能的执行？ */
	UPROPERTY(EditDefaultsOnly, Category = Advanced)
	TEnumAsByte<EGameplayAbilityNetSecurityPolicy::Type> NetSecurityPolicy;

	/** 这个GameplayEffect代表了技能的成本（法力、耐力等）。当技能被提交时，它将被应用。 */
	UPROPERTY(EditDefaultsOnly, Category = Costs)
	TSubclassOf<class UGameplayEffect> CostGameplayEffectClass;

	/** 触发器，用于确定这个技能是否应该响应事件执行 */
	UPROPERTY(EditDefaultsOnly, Category = Triggers)
	TArray<FAbilityTriggerData> AbilityTriggers;
			
	/** 这个GameplayEffect代表了冷却时间。当技能被提交时，它将被应用，并且技能在过期前不能再次使用。 */
	UPROPERTY(EditDefaultsOnly, Category = Cooldowns)
	TSubclassOf<class UGameplayEffect> CooldownGameplayEffectClass;

	// ----------------------------------------------------------------------------------------------------------------
	//	Ability exclusion / canceling 技能 排除/取消 当前参数可以在蓝图中设置
	// ----------------------------------------------------------------------------------------------------------------

	/** 当这个技能被执行时，带有这些标签的技能将被取消。 */
	UPROPERTY(EditDefaultsOnly, Category = Tags, meta=(Categories="AbilityTagCategory"))
	FGameplayTagContainer CancelAbilitiesWithTag;

	/** 当这个技能处于激活状态时，带有这些标签的技能将被阻止 */
	UPROPERTY(EditDefaultsOnly, Category = Tags, meta=(Categories="AbilityTagCategory"))
	FGameplayTagContainer BlockAbilitiesWithTag;

	/** 当这个技能激活时，要应用到激活所有者上的标签。如果AbilitySystemGlobals中启用了ReplicateActivationOwnedTags，则这些标签会被复制 */
	UPROPERTY(EditDefaultsOnly, Category = Tags, meta=(Categories="OwnedTagsCategory"))
	FGameplayTagContainer ActivationOwnedTags;

	/** 只有当激活的演员/组件具有所有这些标签时，这个技能才能被激活 */
	UPROPERTY(EditDefaultsOnly, Category = Tags, meta=(Categories="OwnedTagsCategory"))
	FGameplayTagContainer ActivationRequiredTags;

	/** 如果激活的演员/组件具有这些标签中的任何一个，这个技能将被阻止 */
	UPROPERTY(EditDefaultsOnly, Category = Tags, meta=(Categories="OwnedTagsCategory"))
	FGameplayTagContainer ActivationBlockedTags;

	/** 只有当源演员/组件具有所有这些标签时，这个技能才能被激活 */
	UPROPERTY(EditDefaultsOnly, Category = Tags, meta=(Categories="SourceTagsCategory"))
	FGameplayTagContainer SourceRequiredTags;

	/** 如果源演员/组件具有这些标签中的任何一个，这个技能将被阻止 */
	UPROPERTY(EditDefaultsOnly, Category = Tags, meta=(Categories="SourceTagsCategory"))
	FGameplayTagContainer SourceBlockedTags;

	/** 只有当目标演员/组件具有所有这些标签时，这个技能才能被激活 */
	UPROPERTY(EditDefaultsOnly, Category = Tags, meta=(Categories="TargetTagsCategory"))
	FGameplayTagContainer TargetRequiredTags;

	/** 如果目标演员/组件具有这些标签中的任何一个，这个技能将被阻止 */
	UPROPERTY(EditDefaultsOnly, Category = Tags, meta=(Categories="TargetTagsCategory"))
	FGameplayTagContainer TargetBlockedTags;

	// ----------------------------------------------------------------------------------------------------------------
	//	Ability Tasks 技能任务
	// ----------------------------------------------------------------------------------------------------------------

	/** 查找所有当前激活的、名为InstanceName的任务，并确认它们。具体含义取决于个别任务。如果bEndTask为true，默认情况下除了结束之外不做其他操作。 */
	UFUNCTION(BlueprintCallable, Category = Ability)
	void ConfirmTaskByInstanceName(FName InstanceName, bool bEndTask);

	/** 内部函数，取消上一次我们要求取消的所有任务（按实例名称）。 */
	void EndOrCancelTasksByInstanceName();
	TArray<FName> CancelTaskInstanceNames;

	/** 将任何具有此实例名称的任务添加到下一个帧将被结束（而不是取消）的列表中。另见CancelTaskByInstanceName。 */
	UFUNCTION(BlueprintCallable, Category = Ability)
	void EndTaskByInstanceName(FName InstanceName);
	TArray<FName> EndTaskInstanceNames;

	/** 将任何具有此实例名称的任务添加到下一个帧将被取消（而不是结束）的列表中。另见EndTaskByInstanceName。 */
	UFUNCTION(BlueprintCallable, Category = Ability)
	void CancelTaskByInstanceName(FName InstanceName);

	/** 结束任何具有给定名称的活跃技能状态任务。如果名称为'None'，则结束所有活跃状态（以任意顺序）。 */
	UFUNCTION(BlueprintCallable, Category = Ability)
	void EndAbilityState(FName OptionalStateNameToEnd);

	/** 当前活跃任务的列表，不要直接修改 */
	UPROPERTY()
	TArray<TObjectPtr<UGameplayTask>>	ActiveTasks;

	/** 任务可以在它们的生命周期中发出调试消息以供调试目的。保存在技能上，以便在任务完成后仍然存在 */
	TArray<FAbilityTaskDebugMessage> TaskDebugMessages;

	// ----------------------------------------------------------------------------------------------------------------
	//	Animation 动画
	// ----------------------------------------------------------------------------------------------------------------

	/** 立即将活动蒙太奇跳转到指定的部分 */
	UFUNCTION(BlueprintCallable, Category="Ability|Animation")
	void MontageJumpToSection(FName SectionName);

	/** 设置活动蒙太奇的下一个部分名称 */
	UFUNCTION(BlueprintCallable, Category = "Ability|Animation")
	void MontageSetNextSectionName(FName FromSectionName, FName ToSectionName);

	/**
	 * 停止当前的动画蒙太奇。
	 *
	 * @param OverrideBlendTime 如果大于或等于0，将覆盖AnimMontage实例上的BlendOutTime参数
	 */
	UFUNCTION(BlueprintCallable, Category="Ability|Animation", Meta = (AdvancedDisplay = "OverrideBlendOutTime"))
	void MontageStop(float OverrideBlendOutTime = -1.0f);

	/** 此技能正在播放的活动蒙太奇 */
	UPROPERTY()
	TObjectPtr<class UAnimMontage> CurrentMontage;

	// ----------------------------------------------------------------------------------------------------------------
	//	Target Data 目标数据
	// ----------------------------------------------------------------------------------------------------------------

	/** 从拥有者avatar的位置创建一个目标位置 */
	UFUNCTION(BlueprintPure, Category = Ability)
	FGameplayAbilityTargetingLocationInfo MakeTargetLocationInfoFromOwnerActor();

	/** 从拥有者avatar的骨骼网格组件上的插座创建一个目标位置 */
	UFUNCTION(BlueprintPure, Category = Ability)
	FGameplayAbilityTargetingLocationInfo MakeTargetLocationInfoFromOwnerSkeletalMeshComponent(FName SocketName);

	// ----------------------------------------------------------------------------------------------------------------
	//	Setters for temporary execution data 临时执行数据的设置函数
	// ----------------------------------------------------------------------------------------------------------------

	/** 在由于复制而被创建后，被调用以进行初始化 */
	virtual void PostNetInit();

	/** 修改actor信息，仅在实例化的技能上是安全的 */
	virtual void SetCurrentActorInfo(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo) const;
	
	/** 修改激活信息，仅在实例化的技能上是安全的 */
	virtual void SetCurrentActivationInfo(const FGameplayAbilityActivationInfo ActivationInfo);
	
	/** 设置actor和激活信息 */
	void SetCurrentInfo(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo);
	
	/** 
	 *  这是关于使用我们的实体的共享、缓存信息
	 *	 E.g, Actor*, MovementComponent*, AnimInstance, etc.
	 *	 希望每个actor分配一次并由多个技能共享。
	 *	 实际的结构可能根据游戏需要被覆盖以包含特定于游戏的数据。
	 *	 （例如，子类可能希望将其转换为FMyGameAbilityActorInfo）
	 CurrentActorInfo是一个可变的指针，指向当前使用技能的实体的信息，例如角色、移动组件或动画实例。这个信息被缓存并共享，以减少重复的内存分配。
	 */
	mutable const FGameplayAbilityActorInfo* CurrentActorInfo;

	/** 用于实例化的技能的规格句柄 */
	mutable FGameplayAbilitySpecHandle CurrentSpecHandle;

	/** 在这个技能期间添加的游戏提示，当技能结束时将自动移除 */
	TSet<FGameplayTag> TrackedGameplayCues;

	/** 如果技能当前处于激活状态，则为真。对于每个所有者实例化的技能 */
	UPROPERTY()
	bool bIsActive;
	
	/** 如果结束技能已被调用，但尚未完成，则为真。 */
	UPROPERTY()
	bool bIsAbilityEnding = false;

	/** 如果技能当前可以被取消，则为真，否则只会通过EndAbility调用被取消。 */
	UPROPERTY()
	bool bIsCancelable;

	/** 否当前正在阻止其他技能的激活 */
	UPROPERTY()
	bool bIsBlockingOtherAbilities;

	/** 所有当前作用域锁的计数。用于管理技能的作用域锁定，防止在技能执行期间并行执行其他操作。 */
	mutable int8 ScopeLockCount;

	/** 等待作用域锁结束以便运行的所有函数的列表。包含所有等待作用域锁结束以便执行的委托函数 */
	mutable TArray<FPostLockDelegate> WaitingToExecute;

	/** 增加作用域锁计数。 */
	void IncrementListLock() const;

	/** 减少作用域锁计数。如果计数降至零，则运行等待执行的委托。 */
	void DecrementListLock() const;

public:
	/** 当这个标志被设置时，它表示在技能结束时，应该将该技能标记为待删除。 */
	void SetMarkPendingKillOnAbilityEnd(bool bInMarkPendingKillOnAbilityEnd) { bMarkPendingKillOnAbilityEnd = bInMarkPendingKillOnAbilityEnd; }

	/** 用于查询bMarkPendingKillOnAbilityEnd标志是否已经被设置。如果返回true，则表示该技能在结束时将被标记为待删除。 */
	bool IsMarkPendingKillOnAbilityEnd() const { return bMarkPendingKillOnAbilityEnd; }

protected:

	/**由AbilitySystemComponent::OnRemoveAbility方法设置。当这个方法被调用时，它将这个标志设置为true，以指示在AbilitySystemComponent::NotifyAbilityEnded事件被触发时，该技能需要被清理。 */
	UPROPERTY(BlueprintReadOnly, Category = Ability)
	bool bMarkPendingKillOnAbilityEnd;
};