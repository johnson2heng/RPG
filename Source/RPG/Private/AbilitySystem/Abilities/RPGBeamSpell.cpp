// 版权归暮志未晚所有。


#include "AbilitySystem/Abilities/RPGBeamSpell.h"

#include "AbilitySystem/RPGAbilitySystemLibrary.h"
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

				//如果命中结果绑定了战斗接口，在死亡时，将触发对应的函数
				if(ICombatInterface* CombatInterface = Cast<ICombatInterface>(MouseHitActor))
				{
					//判断是否已经绑定了委托回调
					if(!CombatInterface->GetOnDeathDelegate().IsAlreadyBound(this, &URPGBeamSpell::PrimaryTargetDied))
					{
						//未绑定，则绑定死亡回调
						CombatInterface->GetOnDeathDelegate().AddDynamic(this, &URPGBeamSpell::PrimaryTargetDied);
					}
				}
			}
		}
	}
}

void URPGBeamSpell::StoreAdditionalTargets(TArray<AActor*>& OutAdditionalTargets)
{
	TArray<AActor*> ActorsToIgnore; //遍历时忽略的数组
	ActorsToIgnore.Add(GetAvatarActorFromActorInfo()); //忽略自身
	ActorsToIgnore.Add(MouseHitActor); //忽略鼠标命中的的敌人
	
	TArray<AActor*> OverlappingActors; //存放遍历结果的数组

	//通过封装的函数获取到技能范围内散射的目标
	URPGAbilitySystemLibrary::GetLivePlayersWithinRadius(
		GetAvatarActorFromActorInfo(),
		OverlappingActors,
		ActorsToIgnore,
		850.f,
		MouseHitActor->GetActorLocation());

	int32 NumAdditionalTargets = FMath::Min(GetAbilityLevel() - 1, MaxNumShockTargets);
	// int32 NumAdditionalTargets = 5;

	//通过自定义函数来获取最近的几个目标
	URPGAbilitySystemLibrary::GetClosestTargets(NumAdditionalTargets, OverlappingActors, OutAdditionalTargets, MouseHitActor->GetActorLocation());

	//绑定死亡回调
	for(AActor* TargetActor : OutAdditionalTargets)
	{
		if(ICombatInterface* CombatInterface = Cast<ICombatInterface>(TargetActor))
		{
			//判断是否已经绑定了委托回调
			if(!CombatInterface->GetOnDeathDelegate().IsAlreadyBound(this, &URPGBeamSpell::AdditionalTargetDied))
			{
				//未绑定，则绑定死亡回调
				CombatInterface->GetOnDeathDelegate().AddDynamic(this, &URPGBeamSpell::AdditionalTargetDied);
			}
		}
	}
}

void URPGBeamSpell::OnEndAbility(TArray<AActor*> AdditionalTargets)
{
	//取消主要目标的死亡回调
	if(ICombatInterface* CombatInterface = Cast<ICombatInterface>(MouseHitActor))
	{
		CombatInterface->GetOnDeathDelegate().RemoveDynamic(this, &URPGBeamSpell::PrimaryTargetDied);
	}

	//取消额外目标的死亡回调
	for(AActor* TargetActor : AdditionalTargets)
	{
		if(ICombatInterface* CombatInterface = Cast<ICombatInterface>(TargetActor))
		{
			CombatInterface->GetOnDeathDelegate().RemoveDynamic(this, &URPGBeamSpell::AdditionalTargetDied);
		}
	}
}
