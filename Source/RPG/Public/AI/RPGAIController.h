// 版权归暮志未晚所有。

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "RPGAIController.generated.h"

class UBehaviorTreeComponent;
/**
 * 
 */
UCLASS()
class RPG_API ARPGAIController : public AAIController
{
	GENERATED_BODY()

public:
	ARPGAIController();

protected:

	UPROPERTY()
	TObjectPtr<UBehaviorTreeComponent> BehaviorTreeComponent;
};
