// 版权归暮志未晚所有。


#include "Player/PlayerStateBase.h"
#include "AbilitySystem/AbilitySystemComponentBase.h"
#include "AbilitySystem/AttributeSetBase.h"
#include "Net/UnrealNetwork.h"

APlayerStateBase::APlayerStateBase()
{
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponentBase>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true); //设置组件用于在网络上复制
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<UAttributeSetBase>("AttributeSet");
	
	NetUpdateFrequency = 100.f; //每秒和服务器更新频率，使用GAS后可以设置的高一些
}

void APlayerStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APlayerStateBase, Level);
}

UAbilitySystemComponent* APlayerStateBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void APlayerStateBase::OnRep_Level(int32 OldLevel)
{
	// GAMEPLAYATTRIBUTE_REPNOTIFY(APlayerStateBase, Level, OldLevel);
}
