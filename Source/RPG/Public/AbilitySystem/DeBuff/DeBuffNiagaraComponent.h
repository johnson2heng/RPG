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

	//用来标识粒子系统的负面标签
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag DeBuffTag; 

protected:
	virtual void BeginPlay() override; //覆写开始运行

	/**
	 * 当前的负面标签变动回调函数，如果拥有对应的负面标签，此组件激活，反之，取消激活状态
	 * @param CallbackTag 监听的变动标签
	 * @param NewCount 角色身上拥有对应标签的数量
	 */
	void DeBuffTagChanged(const FGameplayTag CallbackTag, int32 NewCount);

	/**
	 * 在角色死亡时的回调，在角色死亡，取消粒子组件激活
	 * @param DeadActor 死亡角色的回调
	 */
	UFUNCTION()
	void OnOwnerDeath(AActor* DeadActor); 
};
