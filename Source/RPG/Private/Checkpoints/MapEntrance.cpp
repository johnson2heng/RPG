// 版权归暮志未晚所有。


#include "Checkpoints/MapEntrance.h"

#include "Components/SphereComponent.h"
#include "Game/RPGGameMode.h"
#include "Interaction/PlayerInterface.h"
#include "Kismet/GameplayStatics.h"

AMapEntrance::AMapEntrance(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	Sphere->SetupAttachment(MoveToComponent);
}

void AMapEntrance::LoadActor_Implementation()
{
	// 加载地图时，不做任何修改
}

void AMapEntrance::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//if(OtherActor->ActorHasTag("Player")) //如果只需要判断是不是玩家角色通过标签判断即可
	if(OtherActor->Implements<UPlayerInterface>())
	{
		//保存场景状态
		if(ARPGGameMode* RPGGameMode = Cast<ARPGGameMode>(UGameplayStatics::GetGameMode(this)))
		{
			//将目标关卡的路径传入，并将切换地图的相关信息存入存档
			RPGGameMode->SaveWorldState(GetWorld(), DestinationMap.ToSoftObjectPath().GetAssetName());
		}
		
		//将当前角色信息保存到存档
		IPlayerInterface::Execute_SaveProgress(OtherActor, DestinationPlayerStartTag);

		//切换地图
		UGameplayStatics::OpenLevelBySoftObjectPtr(this, DestinationMap);
	}
}
