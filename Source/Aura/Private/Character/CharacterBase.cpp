// 版权归暮志未晚所有。


#include "Character/CharacterBase.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/AbilitySystemComponentBase.h"
#include "Player/PlayerStateBase.h"

// 设置默认值
ACharacterBase::ACharacterBase()
{
 	// 将这个字符设置为true时，将每帧进行更新。不需要可以关闭提高性能。
	PrimaryActorTick.bCanEverTick = false;

	Weapon = CreateDefaultSubobject<USkeletalMeshComponent>("Weapon"); //创建控件
	Weapon->SetupAttachment(GetMesh(), FName("WeaponHandSocket")); //添加到父节点
	Weapon->SetCollisionEnabled(ECollisionEnabled::NoCollision); //关闭骨骼网格体碰撞

}

UAbilitySystemComponent* ACharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();
}

void ACharacterBase::InitAbilityActorInfo()
{
}

void ACharacterBase::ApplyEffectToSelf(TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level) const
{
	check(IsValid(GetAbilitySystemComponent()));
	check(GameplayEffectClass);
	
	FGameplayEffectContextHandle ContextHandle = GetAbilitySystemComponent()->MakeEffectContext();
	ContextHandle.AddSourceObject(this);
	const FGameplayEffectSpecHandle SpecHandle = GetAbilitySystemComponent()->MakeOutgoingSpec(GameplayEffectClass, Level, ContextHandle);
	GetAbilitySystemComponent()->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), GetAbilitySystemComponent());
}

void ACharacterBase::InitializeDefaultAttributes() const
{
	ApplyEffectToSelf(DefaultPrimaryAttributes, 1.f);
	ApplyEffectToSelf(DefaultSecondaryAttributes, 1.f);
	ApplyEffectToSelf(DefaultVitalAttributes, 1.f);
}

void ACharacterBase::AddCharacterAbilities() const
{
	UAbilitySystemComponentBase* ASC = CastChecked<UAbilitySystemComponentBase>(GetAbilitySystemComponent());
	if(!HasAuthority()) return; //查询是否拥有网络权限，应用技能需要添加给服务器

	ASC->AddCharacterAbilities(StartupAbilities);
}




