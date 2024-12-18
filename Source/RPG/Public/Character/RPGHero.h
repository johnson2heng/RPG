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
	
	//当角色被某个控制器控制后，将触发此函数，只在服务端执行， 可在服务器端初始化ASC
	virtual void PossessedBy(AController* NewController) override;
	//当服务器PlayerState复制到客户端后触发此函数，客户端初始化ASC
	virtual void OnRep_PlayerState() override; 

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
	virtual void SaveProgress_Implementation(const FName& CheckpointTag) override;
	virtual TSubclassOf<UGameplayEffect> GetSecondaryAttributes_Implementation() override;
	virtual TSubclassOf<UGameplayEffect> GetVitalAttributes_Implementation() override;
	/* IPlayerInterface战斗接口 结束 */

	/* ICombatInterface战斗接口 */
	virtual int32 GetPlayerLevel_Implementation() override;
	virtual void Die(const FVector& DeathImpulse) override;
	/* ICombatInterface战斗接口 结束 */

	//角色死亡后持续时间，用于表现角色死亡
	UPROPERTY(EditDefaultsOnly)
	float DeathTime = 5.f;

	//声明一个计时器，用于角色死亡后一定时间处理后续逻辑
	FTimerHandle DeathTimer;

	//升级特效组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UNiagaraComponent> LevelUpNiagaraComponent;

	//角色加载存档保存的数值
	void LoadProgress() const;
	
private:

	//相机组件
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCameraComponent> TopDownCameraComponent; 

	//弹簧臂组件
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USpringArmComponent> CameraBoom; 

	//初始化角色技能组件函数
	virtual void InitAbilityActorInfo() override;

	//在多人游戏，每个客户端上播放升级特效
	UFUNCTION(NetMulticast, Reliable)
	void MulticastLevelUpParticles() const; 
};
