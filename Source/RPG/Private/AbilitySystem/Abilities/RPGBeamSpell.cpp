// 版权归暮志未晚所有。


#include "AbilitySystem/Abilities/RPGBeamSpell.h"

#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"

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

void URPGBeamSpell::StoreOwnerVariables()
{
	if(CurrentActorInfo)
	{
		OwnerPlayerController = CurrentActorInfo->PlayerController.Get();
		OwnerCharacter = Cast<ACharacter>(CurrentActorInfo->AvatarActor);
	}
}

void URPGBeamSpell::TraceFirstTarget(const FVector& BeamTargetLocation)
{
	//确保所有者继承了战斗接口
	if(OwnerCharacter && OwnerCharacter->Implements<UCombatInterface>())
	{
		//获取到武器
		if(USkeletalMeshComponent* Weapon = ICombatInterface::Execute_GetWeapon(OwnerCharacter))
		{
			TArray<AActor*> ActorsToIgnore; //当前需要忽略的对象数组
			ActorsToIgnore.Add(OwnerCharacter);// 将自身忽略掉
			
			const FVector SocketLocation = Weapon->GetSocketLocation(FName("TipSocket")); //获取技能发射位置

			FHitResult HitResult; //命中结果的纯粹对象

			//通过武器发射位置和命中位置生成一条球形线，获取第一个命中的结果
			UKismetSystemLibrary::SphereTraceSingle(
				OwnerCharacter,
				SocketLocation,
				BeamTargetLocation,
				10.f,
				TraceTypeQuery1,
				false,
				ActorsToIgnore,
				EDrawDebugTrace::None, //如果需要debug，将其设置ForDuration，如果关闭设置为None
				HitResult,
				true);

			//如果有命中的结果，修改拾取结果
			if(HitResult.bBlockingHit)
			{
				MouseHitLocation = HitResult.ImpactPoint;
				MouseHitActor = HitResult.GetActor();
			}
		}
	}
}
