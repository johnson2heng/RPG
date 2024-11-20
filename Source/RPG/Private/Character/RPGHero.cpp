// 版权归暮志未晚所有。


#include "Character/RPGHero.h"

#include "AbilitySystemComponent.h"
#include "NiagaraComponent.h"
#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "AbilitySystem/RPGAbilitySystemLibrary.h"
#include "AbilitySystem/RPGAttributeSet.h"
#include "AbilitySystem/Data/LevelUpInfo.h"
#include "Camera/CameraComponent.h"
#include "Game/RPGGameInstance.h"
#include "Game/RPGGameMode.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Player/RPGPlayerController.h"
#include "Player/RPGPlayerState.h"
#include "UI/HUD/RPGHUD.h"

ARPGHero::ARPGHero()
{
	//设置相机
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>("CameraBoom");
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->SetUsingAbsoluteRotation(true);
	CameraBoom->bDoCollisionTest = false;

	TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>("TopDownCameraComponent");
	TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCameraComponent->bUsePawnControlRotation = false;
	
	//设置升级特效组件
	LevelUpNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>("LevelUpNiagaraComponent");
	LevelUpNiagaraComponent->SetupAttachment(GetRootComponent()); //设置附加组件
	LevelUpNiagaraComponent->bAutoActivate = false; //设置不自动激活
	
	//设置移动组件
	GetCharacterMovement()->bOrientRotationToMovement = true; //设置为true，角色将朝移动的方向旋转
	GetCharacterMovement()->RotationRate = FRotator(0.f, 400.f, 0.f); //旋转速率
	GetCharacterMovement()->bConstrainToPlane = true; //约束到平面
	GetCharacterMovement()->bSnapToPlaneAtStart = true; //设置了上面一项为true，且此项设置为true，则在开始时与地面对其

	//设置使用控制器选择Pitch，Yaw，Roll的值。
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	//设置玩家职业
	CharacterClass = ECharacterClass::Elementalist;
}

void ARPGHero::AddToXP_Implementation(int32 InXP)
{
	ARPGPlayerState* PlayerStateBase = GetPlayerState<ARPGPlayerState>();
	check(PlayerStateBase); //检测是否有效，无限会暂停游
	PlayerStateBase->AddToXP(InXP);
}

void ARPGHero::LevelUp_Implementation()
{
	MulticastLevelUpParticles(); //调用播放升级特效
}

void ARPGHero::MulticastLevelUpParticles_Implementation() const
{
	if(IsValid(LevelUpNiagaraComponent))
	{
		const FVector CameraLocation = TopDownCameraComponent->GetComponentLocation();
		const FVector NiagaraSystemLocation = LevelUpNiagaraComponent->GetComponentLocation();
		const FRotator TopCameraRotation = (CameraLocation - NiagaraSystemLocation).Rotation(); //获取相机位置和离职特效的朝向
		LevelUpNiagaraComponent->SetWorldRotation(TopCameraRotation); //设置粒子的转向
		LevelUpNiagaraComponent->Activate(true); //激活特效
	}
}

int32 ARPGHero::GetXP_Implementation() const
{
	const ARPGPlayerState* PlayerStateBase = GetPlayerState<ARPGPlayerState>();
	check(PlayerStateBase); //检测是否有效，无效会暂停游戏
	return PlayerStateBase->GetXP();
}

int32 ARPGHero::FindLevelForXP_Implementation(const int32 InXP) const
{
	const ARPGPlayerState* PlayerStateBase = GetPlayerState<ARPGPlayerState>();
	check(PlayerStateBase); //检测是否有效，无效会暂停游戏
	return PlayerStateBase->LevelUpInfo->FindLevelForXP(InXP);
}

int32 ARPGHero::GetAttributePointsReward_Implementation(const int32 Level) const
{
	const ARPGPlayerState* PlayerStateBase = GetPlayerState<ARPGPlayerState>();
	check(PlayerStateBase); //检测是否有效，无效会暂停游戏
	return PlayerStateBase->LevelUpInfo->LevelUpInformation[Level].AttributePointAward;
}

int32 ARPGHero::GetSpellPointsReward_Implementation(const int32 Level) const
{
	const ARPGPlayerState* PlayerStateBase = GetPlayerState<ARPGPlayerState>();
	check(PlayerStateBase); //检测是否有效，无效会暂停游戏
	return PlayerStateBase->LevelUpInfo->LevelUpInformation[Level].SpellPointAward;
}

void ARPGHero::AddToPlayerLevel_Implementation(int32 InPlayerLevel)
{
	ARPGPlayerState* PlayerStateBase = GetPlayerState<ARPGPlayerState>();
	check(PlayerStateBase); //检测是否有效，无效会暂停游戏
	PlayerStateBase->AddToLevel(InPlayerLevel);

	if(URPGAbilitySystemComponent* RPGASC = Cast<URPGAbilitySystemComponent>(GetAbilitySystemComponent()))
	{
		RPGASC->UpdateAbilityStatuses(PlayerStateBase->GetPlayerLevel());
	}
}

void ARPGHero::AddToAttributePoints_Implementation(int32 InAttributePoints)
{
	ARPGPlayerState* PlayerStateBase = GetPlayerState<ARPGPlayerState>();
	check(PlayerStateBase); //检测是否有效，无效会暂停游戏
	PlayerStateBase->AddToAttributePoints(InAttributePoints);
}

void ARPGHero::AddToSpellPoints_Implementation(int32 InSpellPoints)
{
	ARPGPlayerState* PlayerStateBase = GetPlayerState<ARPGPlayerState>();
	check(PlayerStateBase); //检测是否有效，无效会暂停游戏
	PlayerStateBase->AddToSpellPoints(InSpellPoints);
}

int32 ARPGHero::GetAttributePoints_Implementation() const
{
	const ARPGPlayerState* PlayerStateBase = GetPlayerState<ARPGPlayerState>();
	check(PlayerStateBase); //检测是否有效，无效会暂停游戏
	return PlayerStateBase->GetAttributePoints();
}

int32 ARPGHero::GetSpellPoints_Implementation() const
{
	const ARPGPlayerState* PlayerStateBase = GetPlayerState<ARPGPlayerState>();
	check(PlayerStateBase); //检测是否有效，无效会暂停游戏
	return PlayerStateBase->GetSpellPoints();
}

void ARPGHero::ShowMagicCircle_Implementation(UMaterialInterface* DecalMaterial)
{
	if(ARPGPlayerController* RPGPlayerController = Cast<ARPGPlayerController>(GetController()))
	{
		RPGPlayerController->ShowMagicCircle(DecalMaterial);
		//关闭鼠标显示
		RPGPlayerController->bShowMouseCursor = false;
	}
}

void ARPGHero::HideMagicCircle_Implementation()
{
	if(ARPGPlayerController* RPGPlayerController = Cast<ARPGPlayerController>(GetController()))
	{
		RPGPlayerController->HideMagicCircle();
		//显示鼠标
		RPGPlayerController->bShowMouseCursor = true;
	}
}

void ARPGHero::SaveProgress_Implementation(const FName& CheckpointTag)
{
	if(const ARPGGameMode* GameMode = Cast<ARPGGameMode>(UGameplayStatics::GetGameMode(this)))
	{
		//获取存档
		ULoadScreenSaveGame* SaveGameData = GameMode->RetrieveInGameSaveData();
		if(SaveGameData == nullptr) return;

		//修改存档数据
		SaveGameData->PlayerStartTag = CheckpointTag;
		SaveGameData->ActivatedPlayerStatTags.AddUnique(CheckpointTag); //将检查点添加到已激活数组，并去重

		//修改玩家相关
		if(const ARPGPlayerState* RPGPlayerState = Cast<ARPGPlayerState>(GetPlayerState()))
		{
			SaveGameData->PlayerLevel = RPGPlayerState->GetPlayerLevel();
			SaveGameData->XP = RPGPlayerState->GetXP();
			SaveGameData->AttributePoints = RPGPlayerState->GetAttributePoints();
			SaveGameData->SpellPoints = RPGPlayerState->GetSpellPoints();
		}

		//修改主要属性
		SaveGameData->Strength = URPGAttributeSet::GetStrengthAttribute().GetNumericValue(GetAttributeSet());
		SaveGameData->Intelligence = URPGAttributeSet::GetIntelligenceAttribute().GetNumericValue(GetAttributeSet());
		SaveGameData->Resilience = URPGAttributeSet::GetResilienceAttribute().GetNumericValue(GetAttributeSet());
		SaveGameData->Vigor = URPGAttributeSet::GetVigorAttribute().GetNumericValue(GetAttributeSet());

		SaveGameData->bFirstTimeLoadIn = false; //保存完成将第一次加载属性设置为false

		//保存存档
		GameMode->SaveInGameProgressData(SaveGameData);
	}
}

TSubclassOf<UGameplayEffect> ARPGHero::GetSecondaryAttributes_Implementation()
{
	return DefaultSecondaryAttributes;
}

TSubclassOf<UGameplayEffect> ARPGHero::GetVitalAttributes_Implementation()
{
	return DefaultVitalAttributes;
}

int32 ARPGHero::GetPlayerLevel_Implementation()
{
	const ARPGPlayerState* PlayerStateBase = GetPlayerState<ARPGPlayerState>();
	check(PlayerStateBase); //检测是否有效，无效会暂停游戏
	return PlayerStateBase->GetPlayerLevel();
}

//服务端初始化ASC
void ARPGHero::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	//设置OwnerActor的Controller
	SetOwner(NewController);

	//初始化ASC的OwnerActor和AvatarActor
	InitAbilityActorInfo();

	//加载存档数据
	LoadProgress();

	//初始化角色技能
	// AddCharacterAbilities();

	//调用ASC广播
	OnASCRegistered.Broadcast(AbilitySystemComponent);
}

//客户端初始化ASC
void ARPGHero::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	//初始化ASC的OwnerActor和AvatarActor
	InitAbilityActorInfo();

	//调用ASC广播
	OnASCRegistered.Broadcast(AbilitySystemComponent);
}

void ARPGHero::LoadProgress() const
{
	if(const ARPGGameMode* GameMode = Cast<ARPGGameMode>(UGameplayStatics::GetGameMode(this)))
	{
		//获取存档
		ULoadScreenSaveGame* SaveGameData = GameMode->RetrieveInGameSaveData();
		if(SaveGameData == nullptr) return;

		//修改玩家相关
		if(ARPGPlayerState* RPGPlayerState = Cast<ARPGPlayerState>(GetPlayerState()))
		{
			RPGPlayerState->SetLevel(SaveGameData->PlayerLevel, false);
			RPGPlayerState->SetXP(SaveGameData->XP);
			RPGPlayerState->SetAttributePoints(SaveGameData->AttributePoints);
			RPGPlayerState->SetSpellPoints(SaveGameData->SpellPoints);
		}

		//判断是否为第一次加载存档，如果第一次，属性没有相关内容
		if(SaveGameData->bFirstTimeLoadIn)
		{
			//如果第一次加载存档，使用默认GE初始化主要属性
			InitializeDefaultAttributes();

			//初始化角色技能
			AddCharacterAbilities();
		}
		else
		{
			//如果不是第一次，将通过函数库函数通过存档数据初始化角色属性
			URPGAbilitySystemLibrary::InitializeDefaultAttributesFromSaveData(this, AbilitySystemComponent, SaveGameData);

			//初始化角色技能 TODO：还未实现通过存档获取保存的技能，现在测试使用。
			AddCharacterAbilities();
		}
	}
}

void ARPGHero::InitAbilityActorInfo()
{
	ARPGPlayerState* PlayerStateBase = GetPlayerState<ARPGPlayerState>();
	check(PlayerStateBase); //检测是否有效，无效会暂停游戏
	//从playerState获取ASC和AS
	AbilitySystemComponent = PlayerStateBase->GetAbilitySystemComponent();
	AttributeSet = PlayerStateBase->GetAttributeSet();
	//初始化ASC
	AbilitySystemComponent->InitAbilityActorInfo(PlayerStateBase, this);
	//触发Actor的技能信息设置回调
	Cast<URPGAbilitySystemComponent>(AbilitySystemComponent)->AbilityActorInfoSet();

	//注册监听负面标签变动
	DeBuffRegisterChanged();

	//获取PC
	if(ARPGPlayerController* RPGPlayerController = Cast<ARPGPlayerController>(GetController()))
	{
		if(ARPGHUD* HUD = Cast<ARPGHUD>(RPGPlayerController->GetHUD()))
		{
			HUD->InitOverlay(RPGPlayerController, PlayerStateBase, AbilitySystemComponent, AttributeSet);
		}
	}

	//通过GE初始角色的属性
	// InitializeDefaultAttributes();
}
