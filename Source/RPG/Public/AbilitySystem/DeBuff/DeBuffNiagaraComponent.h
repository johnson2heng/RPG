// 版权归暮志未晚所有。

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NiagaraComponent.h"
#include "DeBuffNiagaraComponent.generated.h"

/**
 * 
 */
UCLASS()
class RPG_API UDeBuffNiagaraComponent : public UNiagaraComponent
{
	GENERATED_BODY()

public:

	UDeBuffNiagaraComponent();
	
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag DeBuffTag; //用来标识粒子系统的标签

protected:
	virtual void BeginPlay() override; //覆写开始运行
	void DeBuffTagChanged(const FGameplayTag CallbackTag, int32 NewCount); //当前的负面标签变动回调

	UFUNCTION()
	void OnOwnerDeath(AActor* DeadActor); //在角色死亡时的回调
};
