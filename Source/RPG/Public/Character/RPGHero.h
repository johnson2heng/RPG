// 版权归暮志未晚所有。

#pragma once

#include "CoreMinimal.h"
#include "Character/RPGCharacter.h"
#include "Interaction/PlayerInterface.h"
#include "RPGHero.generated.h"

/**
 * 玩家角色基类
 */
UCLASS()
class RPG_API ARPGHero : public ARPGCharacter, public IPlayerInterface
{
	GENERATED_BODY()

public:
	ARPGHero();
	virtual void PossessedBy(AController* NewController) override; //服务端初始化ASC
	virtual void OnRep_PlayerState() override; //客户端初始化ASC

	/* IPlayerInterface战斗接口 */
	virtual void AddToXP_Implementation(int32 InXP) override;
	virtual void LevelUp_Implementation() override;
	/* IPlayerInterface战斗接口 结束 */

	/* ICombatInterface战斗接口 */
	virtual int32 GetPlayerLevel_Implementation() override;
	/* ICombatInterface战斗接口 结束 */

private:
	virtual  void InitAbilityActorInfo() override;
	
};
