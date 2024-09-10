// 版权归暮志未晚所有。


#include "AbilitySystem/Abilities/RPGFireBolt.h"

#include "RPGGameplayTags.h"
#include "Actor/Projectile.h"
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
		ManaCost,
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

		//根据可生成数量进行逻辑判断
		if(NumProjectiles > 1)
		{
			//获取释放位置
			const FVector SocketLocation = ICombatInterface::Execute_GetCombatSocketLocationByTag(GetAvatarActorFromActorInfo(), SocketTag, SocketName);
			FRotator Rotation = (ProjectileTargetLocation - SocketLocation).Rotation(); //将方向转为旋转
			if(bOverridePitch) Rotation.Pitch = PitchOverride; //覆写发射角度
			
			const float DeltaSpread = ProjectileSpread / NumProjectiles; //技能分的段数
			const FVector LeftOfSpread = Rotation.Vector().RotateAngleAxis(-ProjectileSpread / 2.f, FVector::UpVector); //获取到最左侧的角度
			
			for(int32 i = 0; i<NumProjectiles; i++)
			{
				const FVector Direction = LeftOfSpread.RotateAngleAxis(DeltaSpread * (i + 0.5f), FVector::UpVector); //获取当前分段的角度
				FTransform SpawnTransform;
				SpawnTransform.SetLocation(SocketLocation);
				SpawnTransform.SetRotation(Direction.Rotation().Quaternion());
				
				//SpawnActorDeferred将异步创建实例，在实例创建完成时，相应的数据已经应用到了实例身上
				AProjectile* Projectile = GetWorld()->SpawnActorDeferred<AProjectile>(
					ProjectileClass,
					SpawnTransform,
					GetOwningActorFromActorInfo(),
					Cast<APawn>(GetAvatarActorFromActorInfo()),
					ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

				Projectile->DamageEffectParams = MakeDamageEffectParamsFromClassDefaults();

				//确保变换设置被正确应用
				Projectile->FinishSpawning(SpawnTransform);
				
				UKismetSystemLibrary::DrawDebugArrow(GetAvatarActorFromActorInfo(), SocketLocation, SocketLocation + Direction * 100.f, 5, FLinearColor::Green, 120, 5);
			}
		}
		else
		{
			SpawnProjectile(ProjectileTargetLocation, SocketTag, SocketName, bOverridePitch, PitchOverride);
		}
		
	}
}
