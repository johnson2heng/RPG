// 版权归暮志未晚所有。


#include "Character/RPGCharacterBase.h"
#include "AbilitySystemComponent.h"
#include "RPGGameplayTags.h"
#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "AbilitySystem/DeBuff/DeBuffNiagaraComponent.h"
#include "AbilitySystem/Passive/PassiveNiagaraComponent.h"
#include "RPG/RPG.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

// 设置默认值
ARPGCharacterBase::ARPGCharacterBase()
{
 	// 将这个字符设置为true时，将每帧进行更新。不需要可以关闭提高性能。
	PrimaryActorTick.bCanEverTick = true;

	//设置角色不会和相机碰撞
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetCapsuleComponent()->SetGenerateOverlapEvents(false); //设置碰撞体不可以生成重叠事件
	
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_PROJECTILE, ECR_Overlap); //设置碰撞体可以和发射物产生重叠
	GetMesh()->SetGenerateOverlapEvents(true); //设置碰撞体可以生成重叠事件
	
	Weapon = CreateDefaultSubobject<USkeletalMeshComponent>("Weapon"); //创建控件
	Weapon->SetupAttachment(GetMesh(), FName("WeaponHandSocket")); //添加到父节点
	Weapon->SetCollisionEnabled(ECollisionEnabled::NoCollision); //关闭骨骼网格体碰撞

	//初始化火焰负面效果组件
	BurnDeBuffComponent = CreateDefaultSubobject<UDeBuffNiagaraComponent>("BurnDeBuffComponent");
	BurnDeBuffComponent->SetupAttachment(GetRootComponent());
	BurnDeBuffComponent->DeBuffTag = FRPGGameplayTags::Get().DeBuff_Burn; //设置匹配的负面标签

	//初始化眩晕负面效果组件
	StunDeBuffComponent = CreateDefaultSubobject<UDeBuffNiagaraComponent>("StunDeBuffComponent");
	StunDeBuffComponent->SetupAttachment(GetRootComponent());
	StunDeBuffComponent->DeBuffTag = FRPGGameplayTags::Get().DeBuff_Stun; //设置匹配的负面标签

	//实例化被动技能组件，并挂载
	EffectAttachComponent = CreateDefaultSubobject<USceneComponent>("EffectAttachPoint");
	EffectAttachComponent->SetupAttachment(GetRootComponent());
	HaloOfProtectionNiagaraComponent = CreateDefaultSubobject<UPassiveNiagaraComponent>("HaloOfProtectionComponent");
	HaloOfProtectionNiagaraComponent->SetupAttachment(EffectAttachComponent);
	LifeSiphonNiagaraComponent = CreateDefaultSubobject<UPassiveNiagaraComponent>("LifeSiphonComponent");
	LifeSiphonNiagaraComponent->SetupAttachment(EffectAttachComponent);
	ManaSiphonNiagaraComponent = CreateDefaultSubobject<UPassiveNiagaraComponent>("ManaSiphonComponent");
	ManaSiphonNiagaraComponent->SetupAttachment(EffectAttachComponent);
}

void ARPGCharacterBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	//防止特效跟随人物旋转，每一帧更新修改旋转为默认
	EffectAttachComponent->SetWorldRotation(FRotator::ZeroRotator);
}

void ARPGCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ARPGCharacterBase, bIsStunned); //设置眩晕属性复制
	DOREPLIFETIME(ARPGCharacterBase, IsBeingShocked); //设置眩晕属性复制
}

float ARPGCharacterBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	const float DamageTaken = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	OnDamageDelegate.Broadcast(DamageTaken);
	return DamageTaken;
}

UAbilitySystemComponent* ARPGCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UAnimMontage* ARPGCharacterBase::GetHitReactMontage_Implementation()
{
	return HitReactMontage;
}

void ARPGCharacterBase::Die(const FVector& DeathImpulse)
{
	//将武器从角色身上分离
	Weapon->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, true));
	MulticastHandleDeath(DeathImpulse);
}

void ARPGCharacterBase::MulticastHandleDeath_Implementation(const FVector& DeathImpulse)
{
	//播放死亡音效
	UGameplayStatics::PlaySoundAtLocation(this, DeathSound, GetActorLocation());
	
	//开启武器物理效果
	Weapon->SetSimulatePhysics(true); //开启模拟物理效果
	Weapon->SetEnableGravity(true); //开启重力效果
	Weapon->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly); //开启物理碰撞通道
	// Weapon->AddImpulse(DeathImpulse * 0.01f, NAME_None, true);

	//开启角色物理效果
	GetMesh()->SetSimulatePhysics(true); //开启模拟物理效果
	GetMesh()->SetEnableGravity(true); //开启重力效果
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly); //开启物理碰撞通道
	GetMesh()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block); //开启角色与静态物体产生碰撞
	GetMesh()->AddImpulse(DeathImpulse, NAME_None, true);

	//关闭角色碰撞体碰撞通道，避免其对武器和角色模拟物理效果产生影响
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//设置角色溶解
	Dissolve();

	//设置死亡状态
	bDead = true;
	
	//触发死亡委托
	OnDeathDelegate.Broadcast(this);
}

TArray<FTaggedMontage> ARPGCharacterBase::GetAttackMontages_Implementation()
{
	return AttackMontage;
}

UNiagaraSystem* ARPGCharacterBase::GetBloodEffect_Implementation()
{
	return BloodEffect;
}

FTaggedMontage ARPGCharacterBase::GetTaggedMontageByTag_Implementation(const FGameplayTag& MontageTag)
{
	for(FTaggedMontage TaggedMontage : AttackMontage)
	{
		if(TaggedMontage.MontageTag.MatchesTagExact(MontageTag))
		{
			return TaggedMontage;
		}
	}
	return FTaggedMontage();
}

int32 ARPGCharacterBase::GetMinionCount_Implementation()
{
	return MinionCount;
}

void ARPGCharacterBase::IncrementMinionCount_Implementation(const int32 Amount)
{
	MinionCount += Amount;
}

ECharacterClass ARPGCharacterBase::GetCharacterClass_Implementation()
{
	return CharacterClass;
}

FOnASCRegistered& ARPGCharacterBase::GetOnASCRegisteredDelegate()
{
	return OnASCRegistered;
}

FOnDeath& ARPGCharacterBase::GetOnDeathDelegate()
{
	return OnDeathDelegate;
}

USkeletalMeshComponent* ARPGCharacterBase::GetWeapon_Implementation()
{
	return Weapon;
}

void ARPGCharacterBase::SetIsBeingShocked_Implementation(bool bInShock)
{
	IsBeingShocked = bInShock;
}

bool ARPGCharacterBase::IsBeingShocked_Implementation() const
{
	return IsBeingShocked;
}

FOnDamageSignature& ARPGCharacterBase::GetOnDamageDelegate()
{
	return OnDamageDelegate;
}

void ARPGCharacterBase::DeBuffRegisterChanged()
{
	//监听眩晕标签变动
	AbilitySystemComponent->RegisterGameplayTagEvent(FRPGGameplayTags::Get().DeBuff_Stun, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ARPGCharacterBase::StunTagChanged);
}

void ARPGCharacterBase::StunTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	bIsStunned = NewCount > 0;
	GetCharacterMovement()->MaxWalkSpeed = bIsStunned ? 0.f : BaseWalkSpeed;
}

void ARPGCharacterBase::BeginPlay()
{
	Super::BeginPlay();
}

FVector ARPGCharacterBase::GetCombatSocketLocation_Implementation() const
{
	return Weapon->GetSocketLocation(WeaponTipSocketName);
}

FVector ARPGCharacterBase::GetCombatSocketLocationByTag_Implementation(const FGameplayTag SocketTag, const FName SocketName) const
{
	if(SocketTag.MatchesTagExact(FRPGGameplayTags::Get().CombatSocket_Weapon))
	{
		return Weapon->GetSocketLocation(SocketName);
	}
	return GetMesh()->GetSocketLocation(SocketName);
}

FVector ARPGCharacterBase::GetCombatSocketLocationByStruct_Implementation(const FTaggedMontage TaggedMontage) const
{
	return GetCombatSocketLocationByTag_Implementation(TaggedMontage.SocketTag, TaggedMontage.CombatTipSocketName);
}

bool ARPGCharacterBase::IsDead_Implementation() const
{
	return bDead;
}

AActor* ARPGCharacterBase::GetAvatar_Implementation()
{
	return this;
}

void ARPGCharacterBase::InitAbilityActorInfo()
{
}

void ARPGCharacterBase::ApplyEffectToSelf(TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level) const
{
	check(IsValid(GetAbilitySystemComponent()));
	check(GameplayEffectClass);
	
	FGameplayEffectContextHandle ContextHandle = GetAbilitySystemComponent()->MakeEffectContext();
	ContextHandle.AddSourceObject(this);
	const FGameplayEffectSpecHandle SpecHandle = GetAbilitySystemComponent()->MakeOutgoingSpec(GameplayEffectClass, Level, ContextHandle);
	GetAbilitySystemComponent()->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), GetAbilitySystemComponent());
}

void ARPGCharacterBase::InitializeDefaultAttributes() const
{
	ApplyEffectToSelf(DefaultPrimaryAttributes, 1.f);
	ApplyEffectToSelf(DefaultSecondaryAttributes, 1.f);
	ApplyEffectToSelf(DefaultVitalAttributes, 1.f);
}

void ARPGCharacterBase::AddCharacterAbilities() const
{
	URPGAbilitySystemComponent* ASC = CastChecked<URPGAbilitySystemComponent>(GetAbilitySystemComponent());
	if(!HasAuthority()) return; //查询是否拥有网络权限，应用技能需要添加给服务器

	//调用初始化主动技能和被动技能
	ASC->AddCharacterAbilities(StartupAbilities);
	ASC->AddCharacterPassiveAbilities(StartupPassiveAbilities);
}

void ARPGCharacterBase::Dissolve()
{
	TArray<UMaterialInstanceDynamic*> MatArray;
	//设置角色溶解
	if(IsValid(DissolveMaterialInstance))
	{
		UMaterialInstanceDynamic* DynamicMatInst = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);
		GetMesh()->SetMaterial(0, DynamicMatInst);
		MatArray.Add(DynamicMatInst);
	}

	//设置武器溶解
	if(IsValid(WeaponDissolveMaterialInstance))
	{
		UMaterialInstanceDynamic* DynamicMatInst = UMaterialInstanceDynamic::Create(WeaponDissolveMaterialInstance, this);
		Weapon->SetMaterial(0, DynamicMatInst);
		MatArray.Add(DynamicMatInst);
	}

	//调用时间轴渐变溶解
	StartDissolveTimeline(MatArray);
}




