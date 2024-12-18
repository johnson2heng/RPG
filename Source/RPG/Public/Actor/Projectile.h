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

	//技能投掷物的移动组件，控制物体模拟被发射出去的运动
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	//蓝图可读写，创建时需要将接口暴露出来方便设置
	UPROPERTY(BlueprintReadWrite, meta=(ExposeOnSpawn = true)) 
	FDamageEffectParams DamageEffectParams;

	//一个场景组件，用于确定当前投掷物的攻击目标（在没有默认目标时，有默认目标直接设置目标的根组件）
	UPROPERTY()
	TObjectPtr<USceneComponent> HomingTargetSceneComponent;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Destroyed() override;

	//触发碰撞后调用事件
	UFUNCTION(BlueprintCallable) 
	virtual void PlayImpact();

	//碰撞球 meta需要在私有属性时设置。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> Sphere;

	//记录当前投掷物是否触发了碰撞
	bool bHit;

	//储存循环音效的变量，后续用于删除
	UPROPERTY()
	TObjectPtr<UAudioComponent> LoopingSoundComponent;

	/**
	 * 发射物发生碰撞后的回调函数
	 * @param OverlappedComponent 发生重叠事件的自身的碰撞体对象
	 * @param OtherActor 目标的actor对象
	 * @param OtherComp 目标的碰撞体组件
	 * @param OtherBodyIndex 目标身体的索引
	 * @param bFromSweep 是否为瞬移检测到的碰撞
	 * @param SweepResult 如果位置发生过瞬移（直接设置到某处），两个位置中间的内容会记录到此对象内
	 */
	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
private:

	//此物体的存在时间
	UPROPERTY(EditDefaultsOnly)
	float LifeSpan = 15.f;

	//击中粒子特效
	UPROPERTY(EditAnywhere)
	TObjectPtr<UNiagaraSystem> ImpactEffect;

	//击中音效
	UPROPERTY(EditAnywhere)
	TObjectPtr<USoundBase> ImpactSound;

	//移动循环音效
	UPROPERTY(EditAnywhere)
	TObjectPtr<USoundBase> LoopingSound;
};
