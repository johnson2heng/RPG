// 版权归暮志未晚所有。

#pragma once

#include "CoreMinimal.h"
#include "Character/RPGCharacterBase.h"
#include "Interaction/PlayerInterface.h"
#include "RPGHero.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UNiagaraComponent;
/**
 * 玩家角色基类
 */
UCLASS()
class RPG_API ARPGHero : public ARPGCharacterBase, public IPlayerInterface
{
	GENERATED_BODY()

public:
	ARPGHero();
	virtual void PossessedBy(AController* NewController) override; //服务端初始化ASC
	virtual void OnRep_PlayerState() override; //客户端初始化ASC

	/* IPlayerInterface战斗接口 */
	virtual void AddToXP_Implementation(int32 InXP) override;
	virtual void LevelUp_Implementation() override;
	virtual int32 GetXP_Implementation() const override;
	virtual int32 FindLevelForXP_Implementation(int32 InXP) const override;
	virtual int32 GetAttributePointsReward_Implementation(int32 Level) const override;
	virtual int32 GetSpellPointsReward_Implementation(int32 Level) const override;
	virtual void AddToPlayerLevel_Implementation(int32 InPlayerLevel) override;
	virtual void AddToAttributePoints_Implementation(int32 InAttributePoints) override;
	virtual void AddToSpellPoints_Implementation(int32 InSpellPoints) override;
	virtual int32 GetAttributePoints_Implementation() const override;
	virtual int32 GetSpellPoints_Implementation() const override;
	virtual void ShowMagicCircle_Implementation(UMaterialInterface* DecalMaterial) override;
	virtual void HideMagicCircle_Implementation() override;
	/* IPlayerInterface战斗接口 结束 */

	/* ICombatInterface战斗接口 */
	virtual int32 GetPlayerLevel_Implementation() override;
	/* ICombatInterface战斗接口 结束 */

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UNiagaraComponent> LevelUpNiagaraComponent; //升级特效组件
	
private:

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCameraComponent> TopDownCameraComponent; //相机组件

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USpringArmComponent> CameraBoom; //弹簧臂组件
	
	virtual  void InitAbilityActorInfo() override;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastLevelUpParticles() const; //在多人游戏，每个客户端上播放升级特效
};
