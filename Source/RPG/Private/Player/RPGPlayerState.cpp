// 版权归暮志未晚所有。


#include "Player/RPGPlayerState.h"
#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "AbilitySystem/RPGAttributeSet.h"
#include "Net/UnrealNetwork.h"

ARPGPlayerState::ARPGPlayerState()
{
	AbilitySystemComponent = CreateDefaultSubobject<URPGAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true); //设置组件用于在网络上复制
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<URPGAttributeSet>("AttributeSet");
	
	NetUpdateFrequency = 100.f; //每秒和服务器更新频率，使用GAS后可以设置的高一些
}

void ARPGPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//定义在多人游戏中，需要在网络中复制的属性，当属性发生变化，修改将被发送到其它客户端和服务器
	DOREPLIFETIME(ARPGPlayerState, Level);
	DOREPLIFETIME(ARPGPlayerState, XP);
	DOREPLIFETIME(ARPGPlayerState, AttributePoints);
	DOREPLIFETIME(ARPGPlayerState, SpellPoints);
}

UAbilitySystemComponent* ARPGPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ARPGPlayerState::AddToXP(const int32 InXP)
{
	XP += InXP;
	OnXPChangedDelegate.Broadcast(XP);
}

void ARPGPlayerState::SetXP(const int32 InXP)
{
	XP = InXP;
	OnXPChangedDelegate.Broadcast(XP);
}

void ARPGPlayerState::AddToAttributePoints(const int32 InAttributePoints)
{
	AttributePoints += InAttributePoints;
	OnAttributePointsChangedDelegate.Broadcast(AttributePoints);
}

void ARPGPlayerState::SetAttributePoints(const int32 InAttributePoints)
{
	AttributePoints = InAttributePoints;
	OnAttributePointsChangedDelegate.Broadcast(AttributePoints);
}

void ARPGPlayerState::AddToSpellPoints(const int32 InSpellPoints)
{
	SpellPoints += InSpellPoints;
	OnSpellPointsChangedDelegate.Broadcast(SpellPoints);
}

void ARPGPlayerState::SetSpellPoints(const int32 InSpellPoints)
{
	SpellPoints = InSpellPoints;
	OnSpellPointsChangedDelegate.Broadcast(SpellPoints);
}

void ARPGPlayerState::AddToLevel(const int32 InLevel)
{
	Level += InLevel;
	OnLevelChangedDelegate.Broadcast(Level, true);
}

void ARPGPlayerState::SetLevel(const int32 InLevel, const bool bLevelUp)
{
	Level = InLevel;
	OnLevelChangedDelegate.Broadcast(Level, bLevelUp);
}

void ARPGPlayerState::OnRep_Level(int32 OldLevel) const
{
	OnLevelChangedDelegate.Broadcast(Level, true); //上面修改委托只会在服务器触发，在此处设置是在服务器更新到客户端本地后触发
}

void ARPGPlayerState::OnRep_XP(int32 OldXP) const
{
	OnXPChangedDelegate.Broadcast(XP); //上面修改委托只会在服务器触发，在此处设置是在服务器更新到客户端本地后触发
}

void ARPGPlayerState::OnRep_AttributePoints(int32 OldAttributePoints) const
{
	OnAttributePointsChangedDelegate.Broadcast(AttributePoints);
}

void ARPGPlayerState::OnRep_SpellPoints(int32 OldSpellPoints) const
{
	OnSpellPointsChangedDelegate.Broadcast(SpellPoints);
}
