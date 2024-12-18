// 版权归暮志未晚所有。


#include "Character/RPGEnemy.h"

#include "RPGGameplayTags.h"
#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "AbilitySystem/RPGAttributeSet.h"
#include "AbilitySystem/RPGAbilitySystemLibrary.h"
#include "AI/RPGAIController.h"
#include "RPG/RPG.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UI/Widget/RPGUserWidget.h"

ARPGEnemy::ARPGEnemy()
{
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block); //设置可视为阻挡

	AbilitySystemComponent = CreateDefaultSubobject<URPGAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true); //设置组件用于在网络上复制
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	//设置转向跟随移动方向
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = true;

	AttributeSet = CreateDefaultSubobject<URPGAttributeSet>("AttributeSet");

	HealthBar = CreateDefaultSubobject<UWidgetComponent>("HealthBar");
	HealthBar->SetupAttachment(GetRootComponent()); //将血条附件到根节点上

	//设置自定义深度，高亮接口相关需要
	GetMesh()->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);
	GetMesh()->MarkRenderStateDirty(); //修改了材质，顶点，渲染可见性后，可以将其延后到帧结束，提交数据，下一帧更新，提升性能。
	Weapon->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);
	Weapon->MarkRenderStateDirty(); //修改了材质，顶点，渲染可见性后，可以将其延后到帧结束，提交数据，下一帧更新，提升性能。
}

void ARPGEnemy::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	//当前设置只在服务器端运行，客户端获取不到RPGAIController
	if(!HasAuthority()) return;
	//AIController是在服务器端执行的，所以需要在PossessedBy函数回调中获取服务器返回
	RPGAIController = Cast<ARPGAIController>(NewController);
	//初始化行为树上设置的黑板
	RPGAIController->GetBlackboardComponent()->InitializeBlackboard(*BehaviorTree->BlackboardAsset);
	//运行行为树
	RPGAIController->RunBehaviorTree(BehaviorTree);
	//设置黑板键的值
	RPGAIController->GetBlackboardComponent()->SetValueAsBool(FName("HitReacting"), false); //设置角色是否处于受击状态
	RPGAIController->GetBlackboardComponent()->SetValueAsBool(FName("RangedAttacker"), CharacterClass != ECharacterClass::Warrior); //设置角色是否为远程攻击
}

void ARPGEnemy::HighlightActor_Implementation()
{
	GetMesh()->SetRenderCustomDepth(true);
	Weapon->SetRenderCustomDepth(true);
}

void ARPGEnemy::UnHighlightActor_Implementation()
{
	GetMesh()->SetRenderCustomDepth(false);
	Weapon->SetRenderCustomDepth(false);
}

void ARPGEnemy::SetMoveToLocation_Implementation(FVector& OutDestination)
{
	//敌人类里，将不修改移动位置
}

AActor* ARPGEnemy::GetCombatTarget_Implementation() const
{
	return CombatTarget;
}

void ARPGEnemy::SetCombatTarget_Implementation(AActor* InCombatTarget)
{
	CombatTarget = InCombatTarget;
}

int32 ARPGEnemy::GetPlayerLevel_Implementation()
{
	return Level;
}

void ARPGEnemy::BeginPlay()
{
	Super::BeginPlay();

	//设置角色的初始移动速度
	GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;

	//初始化角色的ASC
	InitAbilityActorInfo();

	//初始化角色的技能
	if(HasAuthority()){
		URPGAbilitySystemLibrary::GiveStartupAbilities(this, AbilitySystemComponent, CharacterClass);
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

		AbilitySystemComponent->RegisterGameplayTagEvent(FRPGGameplayTags::Get().Effects_HitReact, EGameplayTagEventType::AnyCountChange)
			.AddUObject(this, &ThisClass::HitReactTagChanged);

		//初始化血量
		OnHealthChanged.Broadcast(AS->GetHealth());
		OnMaxHealthChanged.Broadcast(AS->GetMaxHealth());
	}
}

void ARPGEnemy::HitReactTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	bHitReacting = NewCount > 0;
	GetCharacterMovement()->MaxWalkSpeed = bHitReacting ? 0.f : BaseWalkSpeed;
	//设置黑板键的值
	if(RPGAIController && RPGAIController->GetBlackboardComponent())
	{
		RPGAIController->GetBlackboardComponent()->SetValueAsBool(FName("HitReacting"), bHitReacting);
	}
	// GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, FString::Printf(TEXT("Hit React bool: %i"), bHitReacting));
}

void ARPGEnemy::Die(const FVector& DeathImpulse)
{
	SetLifeSpan(LifeSpan);
	if(RPGAIController) RPGAIController->GetBlackboardComponent()->SetValueAsBool(FName("Dead"), true); //设置死亡，停止AI行为树

	SpawnLoot(); //生成战利品
	
	Super::Die(DeathImpulse);
}

void ARPGEnemy::InitAbilityActorInfo()
{
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	Cast<URPGAbilitySystemComponent>(AbilitySystemComponent)->AbilityActorInfoSet();

	//注册监听负面标签变动
	DeBuffRegisterChanged();

	//通过GE初始角色的属性
	if(HasAuthority())
	{
		InitializeDefaultAttributes();
	}

	//调用ASC广播
	OnASCRegistered.Broadcast(AbilitySystemComponent);
}

void ARPGEnemy::InitializeDefaultAttributes() const
{
	URPGAbilitySystemLibrary::InitializeDefaultAttributes(this, CharacterClass, Level, AbilitySystemComponent);
}

void ARPGEnemy::StunTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	Super::StunTagChanged(CallbackTag, NewCount);
	
	//设置黑板键的值
	if(RPGAIController && RPGAIController->GetBlackboardComponent())
	{
		RPGAIController->GetBlackboardComponent()->SetValueAsBool(FName("Stunned"), bIsStunned);
	}
}
