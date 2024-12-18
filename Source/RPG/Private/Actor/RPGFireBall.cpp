// 版权归暮志未晚所有。


#include "Actor/RPGFireBall.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayCueManager.h"
#include "RPGGameplayTags.h"
#include "AbilitySystem/RPGAbilitySystemLibrary.h"
#include "Components/AudioComponent.h"
#include "Interaction/CombatInterface.h"

void ARPGFireBall::BeginPlay()
{
	Super::BeginPlay();

	StartOutgoingTimeline(); //调用开始时间线修改
}

void ARPGFireBall::PlayImpact()
{
	if(GetOwner())
	{
		//设置GameplayCue播放位置
		FGameplayCueParameters Parameters;
		Parameters.Location = GetActorLocation();
		UGameplayCueManager::ExecuteGameplayCue_NonReplicated(GetOwner(), FRPGGameplayTags::Get().GameplayCue_FireBlast, Parameters);
	}
	//将音乐停止后会自动销毁
	if(LoopingSoundComponent)
	{
		//循环组件暂停并销毁
		LoopingSoundComponent->Stop();
		LoopingSoundComponent->DestroyComponent();
	}
	
	bHit = true;
}

void ARPGFireBall::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(GetInstigator() == OtherActor) return; //判断和碰撞体接触的目标是否为自身

	//如果不是敌人，将不执行后续
	if(!URPGAbilitySystemLibrary::IsNotFriend(GetInstigator(), OtherActor)) return;

	//目标未继承战斗接口，返回
	if(!OtherActor->Implements<UCombatInterface>()) return;
	
	//在重叠后，销毁自身
	if(HasAuthority()) //是否为服务器，如果服务器端，将执行应用GE，并通过GE的自动同步到客户端
	{
		//为目标应用GE
		if(UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor))
		{
			//死亡冲击的力度和方向
			DamageEffectParams.DeathImpulse = GetActorForwardVector() * DamageEffectParams.DeathImpulseMagnitude;
			
			//通过配置项应用给目标ASC
			DamageEffectParams.TargetAbilitySystemComponent = TargetASC;
			URPGAbilitySystemLibrary::ApplyDamageEffect(DamageEffectParams);
		}
	}
}
