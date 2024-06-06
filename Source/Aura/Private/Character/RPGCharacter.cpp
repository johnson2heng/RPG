// 版权归暮志未晚所有。


#include "Character/RPGCharacter.h"
#include "AbilitySystemComponent.h"
#include "RPGGameplayTags.h"
#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "Aura/Aura.h"
#include "Components/CapsuleComponent.h"
#include "Player/RPGPlayerState.h"

// 设置默认值
ARPGCharacter::ARPGCharacter()
{
 	// 将这个字符设置为true时，将每帧进行更新。不需要可以关闭提高性能。
	PrimaryActorTick.bCanEverTick = false;

	//设置角色不会和相机碰撞
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetCapsuleComponent()->SetGenerateOverlapEvents(false); //设置碰撞体不可以生成重叠事件
	
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_PROJECTILE, ECR_Overlap); //设置碰撞体可以和发射物产生重叠
	GetMesh()->SetGenerateOverlapEvents(true); //设置碰撞体可以生成重叠事件
	
	Weapon = CreateDefaultSubobject<USkeletalMeshComponent>("Weapon"); //创建控件
	Weapon->SetupAttachment(GetMesh(), FName("WeaponHandSocket")); //添加到父节点
	Weapon->SetCollisionEnabled(ECollisionEnabled::NoCollision); //关闭骨骼网格体碰撞
}

UAbilitySystemComponent* ARPGCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UAnimMontage* ARPGCharacter::GetHitReactMontage_Implementation()
{
	return HitReactMontage;
}

void ARPGCharacter::Die()
{
	//将武器从角色身上分离
	Weapon->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, true));
	MulticastHandleDeath();
}

TArray<FTaggedMontage> ARPGCharacter::GetAttackMontages_Implementation()
{
	return AttackMontage;
}

FVector ARPGCharacter::GetCombatSocketLocationByStruct_Implementation(const FTaggedMontage TaggedMontage) const
{
	if(TaggedMontage.MontageTag.MatchesTagExact(FRPGGameplayTags::Get().Montage_Attack_Weapon))
	{
		return Weapon->GetSocketLocation(TaggedMontage.CombatTipSocketName);
	}
	else
	{
		return GetMesh()->GetSocketLocation(TaggedMontage.CombatTipSocketName);
	}
}

void ARPGCharacter::MulticastHandleDeath_Implementation()
{
	//开启武器物理效果
	Weapon->SetSimulatePhysics(true); //开启模拟物理效果
	Weapon->SetEnableGravity(true); //开启重力效果
	Weapon->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly); //开启物理碰撞通道

	//开启角色物理效果
	GetMesh()->SetSimulatePhysics(true); //开启模拟物理效果
	GetMesh()->SetEnableGravity(true); //开启重力效果
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly); //开启物理碰撞通道
	GetMesh()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block); //开启角色与静态物体产生碰撞

	//关闭角色碰撞体碰撞通道，避免其对武器和角色模拟物理效果产生影响
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//设置角色溶解
	Dissolve();

	//设置死亡状态
	bDead = true;
}

void ARPGCharacter::BeginPlay()
{
	Super::BeginPlay();
}

FVector ARPGCharacter::GetCombatSocketLocation_Implementation() const
{
	return Weapon->GetSocketLocation(WeaponTipSocketName);
}

bool ARPGCharacter::IsDead_Implementation() const
{
	return bDead;
}

AActor* ARPGCharacter::GetAvatar_Implementation()
{
	return this;
}

void ARPGCharacter::InitAbilityActorInfo()
{
}

void ARPGCharacter::ApplyEffectToSelf(TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level) const
{
	check(IsValid(GetAbilitySystemComponent()));
	check(GameplayEffectClass);
	
	FGameplayEffectContextHandle ContextHandle = GetAbilitySystemComponent()->MakeEffectContext();
	ContextHandle.AddSourceObject(this);
	const FGameplayEffectSpecHandle SpecHandle = GetAbilitySystemComponent()->MakeOutgoingSpec(GameplayEffectClass, Level, ContextHandle);
	GetAbilitySystemComponent()->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), GetAbilitySystemComponent());
}

void ARPGCharacter::InitializeDefaultAttributes() const
{
	ApplyEffectToSelf(DefaultPrimaryAttributes, 1.f);
	ApplyEffectToSelf(DefaultSecondaryAttributes, 1.f);
	ApplyEffectToSelf(DefaultVitalAttributes, 1.f);
}

void ARPGCharacter::AddCharacterAbilities() const
{
	URPGAbilitySystemComponent* ASC = CastChecked<URPGAbilitySystemComponent>(GetAbilitySystemComponent());
	if(!HasAuthority()) return; //查询是否拥有网络权限，应用技能需要添加给服务器

	ASC->AddCharacterAbilities(StartupAbilities);
}

void ARPGCharacter::Dissolve()
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




