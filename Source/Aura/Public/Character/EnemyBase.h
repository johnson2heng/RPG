// 版权归暮志未晚所有。

#pragma once

#include "CoreMinimal.h"
#include "Character/CharacterBase.h"
#include "Interaction/EnemyInterface.h"
#include "UI/WidgetController/OverlayWidgetController.h"
#include "EnemyBase.generated.h"

class UWidgetComponent;
/**
 * 敌人角色的基类
 */
UCLASS()
class AURA_API AEnemyBase : public ACharacterBase, public IEnemyInterface
{
	GENERATED_BODY()
	
public:
	AEnemyBase();

	/* IEnemyInterface敌人接口 */
	virtual void HighlightActor() override; //高亮
	virtual void UnHighlightActor() override; //取消高亮
	/* IEnemyInterface敌人接口 结束 */

	/* ICombatInterface战斗接口 */
	virtual int32 GetPlayerLevel() override;
	/* ICombatInterface战斗接口 结束 */

	/* 委托 */
	UPROPERTY(BlueprintAssignable)
	FOnAttributeChangedSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable)
	FOnAttributeChangedSignature OnMaxHealthChanged;
	/* 委托 结束 */

protected:
	virtual void BeginPlay() override;
	virtual  void InitAbilityActorInfo() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character Class Defaults")
	int32 Level = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Widget")
	TObjectPtr<UWidgetComponent> HealthBar;
};
