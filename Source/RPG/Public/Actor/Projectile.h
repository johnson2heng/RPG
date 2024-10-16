// 版权归暮志未晚所有。

#pragma once

#include "CoreMinimal.h"
#include "RPGAbilityTypes.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

class UNiagaraSystem;
class UProjectileMovementComponent;
class USphereComponent;

UCLASS()
class RPG_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProjectile();

	UPROPERTY(VisibleAnywhere) //技能投掷物的移动组件，控制物体模拟被发射出去的运动
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(BlueprintReadWrite, meta=(ExposeOnSpawn = true)) //蓝图可读写，创建时需要将接口暴露出来方便设置
	FDamageEffectParams DamageEffectParams;
	
	UPROPERTY() //一个场景组件，用于确定当前投掷物的攻击目标（在没有默认目标时，有默认目标直接设置目标的根组件）
	TObjectPtr<USceneComponent> HomingTargetSceneComponent;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Destroyed() override;

	//碰撞球 meta需要在私有属性时设置。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> Sphere;

	UFUNCTION()
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
private:

	//此物体的存在时间
	UPROPERTY(EditDefaultsOnly)
	float LifeSpan = 15.f;

	void PlayImpact();

	bool bHit;

	//击中粒子特效
	UPROPERTY(EditAnywhere)
	TObjectPtr<UNiagaraSystem> ImpactEffect;

	//击中音效
	UPROPERTY(EditAnywhere)
	TObjectPtr<USoundBase> ImpactSound;

	//移动循环音效
	UPROPERTY(EditAnywhere)
	TObjectPtr<USoundBase> LoopingSound;

	//储存循环音效的变量，后续用于删除
	UPROPERTY()
	TObjectPtr<UAudioComponent> LoopingSoundComponent;
};
