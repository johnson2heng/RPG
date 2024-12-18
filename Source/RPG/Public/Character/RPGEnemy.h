// 版权归暮志未晚所有。

#pragma once

#include "CoreMinimal.h"
#include "Character/RPGCharacterBase.h"
#include "Interaction/EnemyInterface.h"
#include "AbilitySystem/Data/CharacterClassInfo.h"
#include "Interaction/HighLightInterface.h"
#include "UI/WidgetController/OverlayWidgetController.h"
#include "RPGEnemy.generated.h"

class ARPGAIController;
class UBehaviorTree;
class UWidgetComponent;
/**
 * 敌人角色的基类
 */
UCLASS()
class RPG_API ARPGEnemy : public ARPGCharacterBase, public IEnemyInterface, public IHighLightInterface
{
	GENERATED_BODY()
	
public:
	ARPGEnemy();

	//当角色被某个控制器控制后，将触发此函数，只在服务器端执行
	virtual void PossessedBy(AController* NewController) override;

	/****************************** IEnemyInterface敌人接口 ******************************/
	virtual AActor* GetCombatTarget_Implementation() const override;
	virtual void SetCombatTarget_Implementation(AActor* InCombatTarget) override;
	/****************************** IEnemyInterface敌人接口 结束 ******************************/

	/****************************** IHighLightInterface 敌人接口 ******************************/
	virtual void HighlightActor_Implementation() override; //高亮 描边
	virtual void UnHighlightActor_Implementation() override; //取消高亮 描边
	/**
	 * 修改玩家角色移动的目标
	 * @param OutDestination 以引用的方式修改外部目标引用值 
	 */
	virtual void SetMoveToLocation_Implementation(FVector& OutDestination) override;
	/****************************** IHighLightInterface 敌人接口 结束 ******************************/

	/****************************** ICombatInterface战斗接口 ******************************/
	virtual int32 GetPlayerLevel_Implementation() override;
	virtual void Die(const FVector& DeathImpulse) override;
	/****************************** ICombatInterface战斗接口 结束 ******************************/

	/* 委托 */
	UPROPERTY(BlueprintAssignable)
	FOnAttributeChangedSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable)
	FOnAttributeChangedSignature OnMaxHealthChanged;

	void HitReactTagChanged(const FGameplayTag CallbackTag, int32 NewCount);
	/* 委托 结束 */

	//当前是否处于被攻击状态
	UPROPERTY(BlueprintReadOnly, Category="Combat")
	bool bHitReacting = false;  

	//设置死亡后的存在时间
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat")
	float LifeSpan = 5.f; 

	//敌人需要攻击的目标
	UPROPERTY(BlueprintReadWrite, Category="Combat")
	TObjectPtr<AActor> CombatTarget;

	//设置敌人等级
	void SetLevel(const int32 InLevel) { Level = InLevel; }
protected:
	virtual void BeginPlay() override;
	virtual  void InitAbilityActorInfo() override;
	virtual void InitializeDefaultAttributes() const override;
	virtual void StunTagChanged(const FGameplayTag CallbackTag, int32 NewCount) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character Class Defaults")
	int32 Level = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Widget")
	TObjectPtr<UWidgetComponent> HealthBar;

	UPROPERTY(EditAnywhere, Category="AI")
	TObjectPtr<UBehaviorTree> BehaviorTree;

	UPROPERTY()
	TObjectPtr<ARPGAIController> RPGAIController;

	//生成战利品
	UFUNCTION(BlueprintImplementableEvent)
	void SpawnLoot();
};
