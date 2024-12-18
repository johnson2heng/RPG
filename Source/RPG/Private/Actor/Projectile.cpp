// 版权归暮志未晚所有。


#include "Actor/Projectile.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "AbilitySystem/RPGAbilitySystemLibrary.h"
#include "RPG/RPG.h"
#include "Components/AudioComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AProjectile::AProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true; //服务器负责计算并更新Actor的状态，然后通过网络将这些更新复制到所有连接的客户端上。

	//初始化碰撞体
	Sphere = CreateDefaultSubobject<USphereComponent>("Sphere");
	SetRootComponent(Sphere); //设置其为根节点，
	Sphere->SetCollisionObjectType(ECC_PROJECTILE); //设置发射物的碰撞类型
	Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly); //设置其只用作查询使用
	Sphere->SetCollisionResponseToChannels(ECR_Ignore); //设置其忽略所有碰撞检测
	Sphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap); //设置其与世界动态物体产生重叠事件
	Sphere->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap); //设置其与世界静态物体产生重叠事件
	Sphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap); //设置其与Pawn类型物体产生重叠事件

	//创建发射组件
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovement");
	ProjectileMovement->InitialSpeed = 550.f; //设置初始速度
	ProjectileMovement->MaxSpeed = 550.f; //设置最大速度
	ProjectileMovement->ProjectileGravityScale = 0.f; //设置重力影响因子，0为不受影响
}

// Called when the game starts or when spawned
void AProjectile::BeginPlay()
{
	Super::BeginPlay();

	//设置此物体的存在时间
	SetLifeSpan(LifeSpan);
	
	Sphere->OnComponentBeginOverlap.AddDynamic(this, &AProjectile::OnSphereOverlap);

	//创建一个音效，并附加到根组件上面，在技能移动时，声音也会跟随移动
	LoopingSoundComponent = UGameplayStatics::SpawnSoundAttached(LoopingSound, GetRootComponent());
}

void AProjectile::Destroyed()
{
	//如果没有权威性，并且bHit没有修改为true，证明当前没有触发Overlap事件，在销毁前播放击中特效
	if(!bHit) PlayImpact();
	
	Super::Destroyed();
}

void AProjectile::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(GetInstigator() == OtherActor) return; //判断和碰撞体接触的目标是否为自身

	//如果不是敌人，将不执行后续
	if(!URPGAbilitySystemLibrary::IsNotFriend(GetInstigator(), OtherActor)) return;
	
	//播放击中特效 当前服务器端未调用销毁时，直接播放
	if(!bHit) PlayImpact();

	//在重叠后，销毁自身
	if(HasAuthority()) //是否为服务器，如果服务器端，将执行应用GE，并通过GE的自动同步到客户端
	{
		//为目标应用GE
		if(UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor))
		{
			//死亡冲击的力度和方向
			DamageEffectParams.DeathImpulse = GetActorForwardVector() * DamageEffectParams.DeathImpulseMagnitude;

			//判断攻击是否触发击退
			if(const bool bKnockback = FMath::RandRange(1, 100) < DamageEffectParams.KnockbackChance)
			{
				//将技能攻击的朝向向上偏转，基于向右的方向向量
				const FVector KnockbackDirection = GetActorForwardVector().RotateAngleAxis(-45.f, GetActorRightVector()); 
				DamageEffectParams.KnockbackForce = KnockbackDirection * DamageEffectParams.KnockbackForceMagnitude;
			}
			
			//通过配置项应用给目标ASC
			DamageEffectParams.TargetAbilitySystemComponent = TargetASC;
			URPGAbilitySystemLibrary::ApplyDamageEffect(DamageEffectParams);
		}
		
		Destroy();
	}
	else
	{
		//如果对actor没有权威性，将bHit设置为true，证明当前已经播放了击中特效
		bHit = true;
	}
}

void AProjectile::PlayImpact()
{
	//播放声效
	UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation(), FRotator::ZeroRotator);
	//播放粒子特效
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ImpactEffect, GetActorLocation());
	//将音乐停止后会自动销毁
	if(LoopingSoundComponent)
	{
		//循环组件暂停并销毁
		LoopingSoundComponent->Stop();
		LoopingSoundComponent->DestroyComponent();
	}
	
	bHit = true;
}




