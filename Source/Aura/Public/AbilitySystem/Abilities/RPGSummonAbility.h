// 版权归暮志未晚所有。

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/RPGGameplayAbility.h"
#include "RPGSummonAbility.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API URPGSummonAbility : public URPGGameplayAbility
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable)
	TArray<FVector> GetSpawnLocations(); //获取召唤物生成位置

	UFUNCTION(BlueprintPure, Category="Summoning")
	TSubclassOf<APawn> GetRandomMinionClass(); //获取随机的召唤物类

	UPROPERTY(EditDefaultsOnly, Category="Summoning")
	int32 NumMinions = 5; // 召唤的数量

	UPROPERTY(EditDefaultsOnly, Category="Summoning")
	TArray<TSubclassOf<APawn>> MinionClasses; //召唤生成的角色类

	UPROPERTY(EditDefaultsOnly, Category="Summoning")
	float MinSpawnDistance = 50.f; //召唤物距离召唤师最近的距离

	UPROPERTY(EditDefaultsOnly, Category="Summoning")
	float MaxSpawnDistance = 250.f; //召唤物距离召唤师最远的距离

	UPROPERTY(EditDefaultsOnly, Category="Summoning")
	float SpawnSpread = 90.f; //召唤物在召唤师前面的角度范围
};
