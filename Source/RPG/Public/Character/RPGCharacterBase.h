// 版权归暮志未晚所有。

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "Interaction/CombatInterface.h"
#include "RPGCharacterBase.generated.h"

class UPassiveNiagaraComponent;
class UDeBuffNiagaraComponent;
class UGameplayAbility;
class UGameplayEffect;
class UAbilitySystemComponent;
class UAttributeSet;

/**
 * 所有角色的基类
 */
UCLASS(Abstract)
class RPG_API ARPGCharacterBase : public ACharacter, public IAbilitySystemInterface, public ICombatInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ARPGCharacterBase();

	virtual void Tick(float DeltaSeconds) override;

	//设置生命周期内需要复制的属性
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/**
	 * 覆写 应用伤害给自身
	 * @see https://www.unrealengine.com/blog/damage-in-ue4
	 * @param DamageAmount		要施加的伤害数值
	 * @param DamageEvent		描述伤害细节的结构体，支持不同类型的伤害，如普通伤害、点伤害（FPointDamageEvent）、范围伤害（FRadialDamageEvent）等。
	 * @param EventInstigator	负责造成伤害的 Controller，通常是玩家或 AI 的控制器。
	 * @param DamageCauser		直接造成伤害的 Actor，例如爆炸物、子弹或掉落的石头。
	 * @return					返回实际应用的伤害值。这允许目标修改或减少伤害，然后将最终的值返回。
	 */
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	//覆盖虚函数获取asc
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//获取as
	UAttributeSet* GetAttributeSet() const { return AttributeSet; } 

	/* ICombatInterface战斗接口 */
	virtual UAnimMontage* GetHitReactMontage_Implementation() override;
	virtual FVector GetCombatSocketLocation_Implementation() const override;
	virtual FVector GetCombatSocketLocationByTag_Implementation(const FGameplayTag SocketTag, const FName SocketName) const override;
	virtual FVector GetCombatSocketLocationByStruct_Implementation(const FTaggedMontage TaggedMontage) const override;
	virtual bool IsDead_Implementation() const override;
	virtual AActor* GetAvatar_Implementation() override;
	virtual void Die(const FVector& DeathImpulse) override;
	virtual TArray<FTaggedMontage> GetAttackMontages_Implementation() override;
	virtual UNiagaraSystem* GetBloodEffect_Implementation() override;
	virtual FTaggedMontage GetTaggedMontageByTag_Implementation(const FGameplayTag& MontageTag) override;
	virtual int32 GetMinionCount_Implementation() override;
	virtual void IncrementMinionCount_Implementation(const int32 Amount) override;
	virtual ECharacterClass GetCharacterClass_Implementation() override;
	virtual FOnASCRegistered& GetOnASCRegisteredDelegate() override; //获取ASC注册成功委托
	virtual FOnDeath& GetOnDeathDelegate() override; //角色死亡委托
	virtual USkeletalMeshComponent* GetWeapon_Implementation() override;
	virtual void SetIsBeingShocked_Implementation(bool bInShock) override;
	virtual bool IsBeingShocked_Implementation() const override;
	virtual FOnDamageSignature& GetOnDamageDelegate() override;
	/* ICombatInterface战斗接口 结束 */

	FOnASCRegistered OnASCRegistered; //ASC注册成功委托
	FOnDeath OnDeathDelegate; //角色死亡后触发的死亡委托
	FOnDamageSignature OnDamageDelegate; //传入伤害后得到结果后的委托

	UFUNCTION(NetMulticast, Reliable)
	virtual void MulticastHandleDeath(const FVector& DeathImpulse);

	UPROPERTY(EditAnywhere, Category="Combat")
	TArray<FTaggedMontage> AttackMontage;

	//当前角色是否处于眩晕状态
	UPROPERTY(Replicated, BlueprintReadOnly)
	bool bIsStunned = false;

	//当前角色是否处于持续攻击状态
	UPROPERTY(Replicated, BlueprintReadOnly)
	bool IsBeingShocked = false;

	//设置当前敌人的职业类型
	void SetCharacterClass(const ECharacterClass InClass) { CharacterClass = InClass; }
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//注册用于监听负面标签变动的函数
	void DeBuffRegisterChanged();

	//眩晕标签变动后的回调
	virtual void StunTagChanged(const FGameplayTag CallbackTag, int32 NewCount);

	//当前角色死亡状态
	bool bDead = false; 

	//当前角色的最大移动速度
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat")
	float BaseWalkSpeed = 600.f;

	//角色的武器组件
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<USkeletalMeshComponent> Weapon;

	//设置技能释放的位置
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	FName WeaponTipSocketName; 

	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;

	//初始化角色技能组件函数，在子类的begin play中调用
	virtual void InitAbilityActorInfo();

	//主要属性GE
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Attributes")
	TSubclassOf<UGameplayEffect> DefaultPrimaryAttributes;

	//次级属性GE
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Attributes")
	TSubclassOf<UGameplayEffect> DefaultSecondaryAttributes;

	//额外的属性GE
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Attributes")
	TSubclassOf<UGameplayEffect> DefaultVitalAttributes;

	void ApplyEffectToSelf(TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level) const;

	//初始化角色的属性
	virtual void InitializeDefaultAttributes() const;

	//初始化角色的技能
	void AddCharacterAbilities() const;

	/******************************************* 溶解效果材质 *******************************************/

	//溶解效果
	void Dissolve(); 

	UFUNCTION(BlueprintImplementableEvent)
	void StartDissolveTimeline(const TArray<UMaterialInstanceDynamic*>& DynamicMaterialInstance);

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UMaterialInstance> DissolveMaterialInstance;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UMaterialInstance> WeaponDissolveMaterialInstance;

	//受伤特效
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat")
	UNiagaraSystem* BloodEffect; 

	//死亡音效
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat")
	USoundBase* DeathSound; 

	/* 召唤仆从 */

	//仆从数量
	UPROPERTY(BlueprintReadOnly, Category="Combat")
	int32 MinionCount = 0; 

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character Class Defaults")
	ECharacterClass CharacterClass = ECharacterClass::Warrior;

	//火焰负面效果表现组件
	UPROPERTY(VisibleAnywhere) 
	TObjectPtr<UDeBuffNiagaraComponent> BurnDeBuffComponent;

	//眩晕负面效果表现组件
	UPROPERTY(VisibleAnywhere) 
	TObjectPtr<UDeBuffNiagaraComponent> StunDeBuffComponent;
	
private:

	//角色初始技能设置
	UPROPERTY(EditAnywhere, Category="Attributes")
	TArray<TSubclassOf<UGameplayAbility>> StartupAbilities; 

	//角色初始被动技能设置
	UPROPERTY(EditAnywhere, Category="Attributes")
	TArray<TSubclassOf<UGameplayAbility>> StartupPassiveAbilities; 

	UPROPERTY(EditAnywhere, Category="Combat")
	TObjectPtr<UAnimMontage> HitReactMontage;

	//光环被动技能特效组件
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UPassiveNiagaraComponent> HaloOfProtectionNiagaraComponent;

	//回血被动技能特效组件
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UPassiveNiagaraComponent> LifeSiphonNiagaraComponent;

	//回蓝被动技能特效组件
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UPassiveNiagaraComponent> ManaSiphonNiagaraComponent;

	//被动技能挂载的组件
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> EffectAttachComponent;
};
