// 版权归暮志未晚所有。

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

class UNiagaraSystem;
class UProjectileMovementComponent;
class USphereComponent;

UCLASS()
class AURA_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProjectile();

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Destroyed() override;

	UFUNCTION()
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
private:

	//此物体的存在时间
	UPROPERTY(EditDefaultsOnly)
	float LifeSpan = 15.f;

	void PlayImpact() const;

	bool bHit;

	//碰撞球
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> Sphere;

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
