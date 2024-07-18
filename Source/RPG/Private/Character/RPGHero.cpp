// 版权归暮志未晚所有。


#include "Character/RPGHero.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Player/RPGPlayerController.h"
#include "Player/RPGPlayerState.h"
#include "UI/HUD/RPGHUD.h"

ARPGHero::ARPGHero()
{
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

//服务端初始化ASC
void ARPGHero::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	//初始化ASC的OwnerActor和AvatarActor
	InitAbilityActorInfo();

	//设置OwnerActor的Controller
	SetOwner(NewController);

	//初始化角色技能
	AddCharacterAbilities();
}

//客户端初始化ASC
void ARPGHero::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	//初始化ASC的OwnerActor和AvatarActor
	InitAbilityActorInfo();
}

void ARPGHero::AddToXP_Implementation(int32 InXP)
{
	ARPGPlayerState* PlayerStateBase = GetPlayerState<ARPGPlayerState>();
	check(PlayerStateBase); //检测是否有效，无限会暂停游
	PlayerStateBase->AddToXP(InXP);
}

void ARPGHero::LevelUp_Implementation()
{
	
}

int32 ARPGHero::GetPlayerLevel_Implementation()
{
	const ARPGPlayerState* PlayerStateBase = GetPlayerState<ARPGPlayerState>();
	check(PlayerStateBase); //检测是否有效，无限会暂停游戏
	return PlayerStateBase->GetPlayerLevel();
}

void ARPGHero::InitAbilityActorInfo()
{
	ARPGPlayerState* PlayerStateBase = GetPlayerState<ARPGPlayerState>();
	check(PlayerStateBase); //检测是否有效，无限会暂停游戏
	//从playerState获取ASC和AS
	AbilitySystemComponent = PlayerStateBase->GetAbilitySystemComponent();
	AttributeSet = PlayerStateBase->GetAttributeSet();
	//初始化ASC
	AbilitySystemComponent->InitAbilityActorInfo(PlayerStateBase, this);
	//触发Actor的技能信息设置回调
	Cast<URPGAbilitySystemComponent>(AbilitySystemComponent)->AbilityActorInfoSet(); 

	//获取PC
	if(ARPGPlayerController* PlayerControllerBase = Cast<ARPGPlayerController>(GetController()))
	{
		if(ARPGHUD* HUD = Cast<ARPGHUD>(PlayerControllerBase->GetHUD()))
		{
			HUD->InitOverlay(PlayerControllerBase, PlayerStateBase, AbilitySystemComponent, AttributeSet);
		}
	}

	//通过GE初始角色的属性
	InitializeDefaultAttributes();
	
}
