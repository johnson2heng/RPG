// 版权归暮志未晚所有。

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MyGameModeBase.generated.h"

class UCharacterClassInfo;
/**
 * 游戏模式基类
 */
UCLASS()
class AURA_API AMyGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, Category="Character Class Defaults")
	TObjectPtr<UCharacterClassInfo> CharacterClassInfo;
};
