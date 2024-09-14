// 版权归暮志未晚所有。


#include "AbilitySystem/Abilities/RPGBeamSpell.h"

void URPGBeamSpell::StoreMouseDataInfo(const FHitResult& HitResult)
{
	//判断当前是否拾取到内容
	if(HitResult.bBlockingHit)
	{
		MouseHitLocation = HitResult.ImpactPoint;
		MouseHitActor = HitResult.GetActor();
	}
	else
	{
		//取消技能
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
	}
}

void URPGBeamSpell::StoreOwnerPlayerController()
{
	if(CurrentActorInfo)
	{
		OwnerPlayerController = CurrentActorInfo->PlayerController.Get();
	}
	
}
