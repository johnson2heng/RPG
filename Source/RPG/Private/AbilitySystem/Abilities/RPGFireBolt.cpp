// 版权归暮志未晚所有。


#include "AbilitySystem/Abilities/RPGFireBolt.h"

#include "RPGGameplayTags.h"
#include "AbilitySystem/RPGAbilitySystemLibrary.h"
#include "Actor/Projectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"

FString URPGFireBolt::GetDescription(const int32 Level)
{
	return GetDescriptionAtLevel(Level, L"火球术");
}

FString URPGFireBolt::GetNextLevelDescription(const int32 Level)
{
	return GetDescriptionAtLevel(Level, L"下一等级");
}

FString URPGFireBolt::GetDescriptionAtLevel(const int32 Level, const FString& Title)
{
	const int32 Damage = GetDamageByDamageType(Level, FRPGGameplayTags::Get().Damage_Fire);
	const float ManaCost = GetManaCost(Level);
	const float Cooldown = GetCooldown(Level);
	
	return FString::Printf(TEXT(
		// 标题
		"<Title>%s</>\n"

		// 细节
		"<Small>等级：</> <Level>%i</>\n"
		"<Small>技能冷却：</> <Cooldown>%.1f</>\n"
		"<Small>蓝量消耗：</> <ManaCost>%.1f</>\n\n"//%.1f会四舍五入到小数点后一位

		// 技能描述
		"<Default>发射 %i 颗火球，在发生撞击时产生爆炸，并造成</> <Damage>%i</> <Default>点火焰伤害，并有一定几率触发燃烧。</>"),

		// 动态修改值
		*Title,
		Level,
		Cooldown,
		-ManaCost,
		FMath::Min(Level, NumProjectiles),
		Damage);
}

void URPGFireBolt::SpawnProjectiles(const FVector& ProjectileTargetLocation, const FGameplayTag& SocketTag, const FName SocketName, const bool bOverridePitch, const float PitchOverride, AActor* HomingTarget)
{
	const bool bIsServer = GetAvatarActorFromActorInfo()->HasAuthority(); //判断此函数是否在服务器运行
	if (!bIsServer) return;

	if (GetAvatarActorFromActorInfo()->Implements<UCombatInterface>())
	{
		//限制产生火球的最大数量
		NumProjectiles = FMath::Min(MaxNumProjectiles, GetAbilityLevel());
		
		//获取释放位置
		const FVector SocketLocation = ICombatInterface::Execute_GetCombatSocketLocationByTag(GetAvatarActorFromActorInfo(), SocketTag, SocketName);
		FRotator Rotation = (ProjectileTargetLocation - SocketLocation).Rotation(); //将方向转为旋转
		if(bOverridePitch) Rotation.Pitch = PitchOverride; //覆写发射角度

		const FVector Forward = Rotation.Vector(); //获取朝向向量
		//根据函数获取到所有生成的转向
		TArray<FRotator> Rotations = URPGAbilitySystemLibrary::EvenlySpacedRotators(Forward, FVector::UpVector, ProjectileSpread, NumProjectiles);

		//遍历所有朝向，并生成火球术
		for(FRotator& Rot : Rotations)
		{
			FTransform SpawnTransform;
			SpawnTransform.SetLocation(SocketLocation);
			SpawnTransform.SetRotation(Rot.Quaternion());
				
			//SpawnActorDeferred将异步创建实例，在实例创建完成时，相应的数据已经应用到了实例身上
			AProjectile* Projectile = GetWorld()->SpawnActorDeferred<AProjectile>(
				ProjectileClass,
				SpawnTransform,
				GetOwningActorFromActorInfo(),
				CurrentActorInfo->PlayerController->GetPawn(),
				ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

			Projectile->DamageEffectParams = MakeDamageEffectParamsFromClassDefaults();

			//根据目标类型设置HomingTargetComponent，此内容必须在飞弹被生成后设置
			if(HomingTarget && HomingTarget->Implements<UCombatInterface>())
			{
				//设置攻击的位置为攻击对象的根位置
				Projectile->ProjectileMovement->HomingTargetComponent = HomingTarget->GetRootComponent();
			}
			else
			{
				//如果没有获取到攻击目标，则创建一个可销毁的并应用
				Projectile->HomingTargetSceneComponent = NewObject<USceneComponent>(USceneComponent::StaticClass());
				Projectile->HomingTargetSceneComponent->SetWorldLocation(ProjectileTargetLocation); //设置组件位置
				Projectile->ProjectileMovement->HomingTargetComponent = Projectile->HomingTargetSceneComponent;
			}
			//设置飞弹朝向目标时的加速度
			Projectile->ProjectileMovement->HomingAccelerationMagnitude = FMath::FRandRange(HomingAccelerationMin, HomingAccelerationMax);
			Projectile->ProjectileMovement->bIsHomingProjectile = bLaunchHomingProjectiles; //设置为true，飞弹将加速飞向攻击目标

			//确保变换设置被正确应用
			Projectile->FinishSpawning(SpawnTransform);

			//Debug
			//UKismetSystemLibrary::DrawDebugArrow(GetAvatarActorFromActorInfo(), SocketLocation, SocketLocation + Rot.Vector() * 100.f, 5, FLinearColor::Green, 120, 5);
		}
		
	}
}
