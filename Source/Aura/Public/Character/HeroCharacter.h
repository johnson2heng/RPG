// 版权归暮志未晚所有。

#pragma once

#include "CoreMinimal.h"
#include "Character/CharacterBase.h"
#include "HeroCharacter.generated.h"

/**
 * 使用ALT+O生成当前头
 * 玩家角色基类
 */
UCLASS()
class AURA_API AHeroCharacter : public ACharacterBase
{
	GENERATED_BODY()

public:
	AHeroCharacter();
	virtual void PossessedBy(AController* NewController) override; //服务端初始化ASC
	virtual void OnRep_PlayerState() override; //客户端初始化ASC

private:
	void InitAbilityActorInfo();
	
};
