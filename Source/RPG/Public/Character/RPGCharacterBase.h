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
	/* ICombatInterface战斗接口 结束 */

	FOnASCRegistered OnASCRegistered; //ASC注册成功委托
	FOnDeath OnDeathDelegate; //角色死亡后触发的死亡委托

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

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Attributes")
	TSubclassOf<UGameplayEffect> DefaultPrimaryAttributes;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Attributes")
	TSubclassOf<UGameplayEffect> DefaultSecondaryAttributes;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Attributes")
	TSubclassOf<UGameplayEffect> DefaultVitalAttributes;

	void ApplyEffectToSelf(TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level) const;

	virtual void InitializeDefaultAttributes() const;

	void AddCharacterAbilities() const;

	/* 溶解效果材质 */

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
