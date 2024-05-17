// 版权归暮志未晚所有。

#pragma once

#include "CoreMinimal.h"
#include "RPGDamageGameplayAbility.h"
#include "ProjectileSpell.generated.h"

class AProjectile;
/**
 * 
 */
UCLASS()
class AURA_API UProjectileSpell : public URPGDamageGameplayAbility
{
	GENERATED_BODY()

protected:

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UFUNCTION(BlueprintCallable, Category="Projectile")
	void SpawnProjectile(const FVector& ProjectileTargetLocation);
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AProjectile> ProjectileClass;
};
