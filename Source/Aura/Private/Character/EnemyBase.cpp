// 版权归暮志未晚所有。


#include "Character/EnemyBase.h"

#include "RPGGameplayTags.h"
#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "AbilitySystem/RPGAttributeSet.h"
#include "AbilitySystem/RPGAbilitySystemBlueprintLibrary.h"
#include "Aura/Aura.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UI/Widget/RPGUserWidget.h"

AEnemyBase::AEnemyBase()
{
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block); //设置可视为阻挡

	AbilitySystemComponent = CreateDefaultSubobject<URPGAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true); //设置组件用于在网络上复制
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	AttributeSet = CreateDefaultSubobject<URPGAttributeSet>("AttributeSet");

	HealthBar = CreateDefaultSubobject<UWidgetComponent>("HealthBar");
	HealthBar->SetupAttachment(GetRootComponent()); //将血条附件到根节点上
}

void AEnemyBase::HighlightActor()
{
	GetMesh()->SetRenderCustomDepth(true);
	GetMesh()->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);
	Weapon->SetRenderCustomDepth(true);
	Weapon->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);
}

void AEnemyBase::UnHighlightActor()
{
	GetMesh()->SetRenderCustomDepth(false);
	Weapon->SetRenderCustomDepth(false);
}

int32 AEnemyBase::GetPlayerLevel()
{
	return Level;
}

void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();

	//设置角色的初始移动速度
	GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;

	//初始化角色的ASC
	InitAbilityActorInfo();

	//初始化角色的技能
	if(HasAuthority()){
		URPGAbilitySystemBlueprintLibrary::GiveStartupAbilities(this, AbilitySystemComponent);
	}

	if(URPGUserWidget* UserWidget = Cast<URPGUserWidget>(HealthBar->GetUserWidgetObject()))
	{
		UserWidget->SetWidgetController(this);
	}
	
	if(const URPGAttributeSet* AS = Cast<URPGAttributeSet>(AttributeSet))
	{
		//监听血量变化
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AS->GetHealthAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnHealthChanged.Broadcast(Data.NewValue);
			}
		);
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AS->GetMaxHealthAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnMaxHealthChanged.Broadcast(Data.NewValue);
			}
		);

		AbilitySystemComponent->RegisterGameplayTagEvent(FRPGGameplayTags::Get().Effects_HitReact, EGameplayTagEventType::NewOrRemoved)
			.AddUObject(this, &ThisClass::HitReactTagChanged);

		//初始化血量
		OnHealthChanged.Broadcast(AS->GetHealth());
		OnMaxHealthChanged.Broadcast(AS->GetMaxHealth());
	}
}

void AEnemyBase::HitReactTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	bHitReacting = NewCount > 0;
	GetCharacterMovement()->MaxWalkSpeed = bHitReacting ? 0.f : BaseWalkSpeed;
}

void AEnemyBase::Die()
{
	SetLifeSpan(LifeSpan);
	Super::Die();
}

void AEnemyBase::InitAbilityActorInfo()
{
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	Cast<URPGAbilitySystemComponent>(AbilitySystemComponent)->AbilityActorInfoSet();

	//通过GE初始角色的属性
	if(HasAuthority())
	{
		InitializeDefaultAttributes();
	}
}

void AEnemyBase::InitializeDefaultAttributes() const
{
	URPGAbilitySystemBlueprintLibrary::InitializeDefaultAttributes(this, CharacterClass, Level, AbilitySystemComponent);
}
