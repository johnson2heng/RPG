// 版权归暮志未晚所有。


#include "Checkpoints/CheckPoint.h"
#include "Components/SphereComponent.h"
#include "Game/RPGGameMode.h"
#include "Interaction/PlayerInterface.h"
#include "Kismet/GameplayStatics.h"

ACheckPoint::ACheckPoint(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{
	//关闭帧更新
	PrimaryActorTick.bCanEverTick = false;

	//创建检测点显示模型
	CheckpointMesh = CreateDefaultSubobject<UStaticMeshComponent>("CheckpointMesh");
	CheckpointMesh->SetupAttachment(GetRootComponent());
	CheckpointMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); //设置查询并产生物理
	CheckpointMesh->SetCollisionResponseToChannels(ECR_Block); //设置阻挡所有物体与其重叠
	CheckpointMesh->SetCustomDepthStencilValue(CustomDepthStencilOverride); //设置自定义深度值
	CheckpointMesh->MarkRenderStateDirty(); //修改了材质，顶点，渲染可见性后，可以将其延后到帧结束，提交数据，下一帧更新，提升性能。

	//设置球碰撞体
	Sphere = CreateDefaultSubobject<USphereComponent>("Sphere");
	Sphere->SetupAttachment(CheckpointMesh);
	Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly); //设置其只用作查询使用
	Sphere->SetCollisionResponseToChannels(ECR_Ignore); //设置其忽略所有碰撞检测
	Sphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap); //设置其与Pawn类型物体产生重叠事件

	//控制自动移动组件
	MoveToComponent = CreateDefaultSubobject<USceneComponent>("MoveToComponent");
	MoveToComponent->SetupAttachment(GetRootComponent());
}

void ACheckPoint::LoadActor_Implementation()
{
	if(bReached)
	{
		HandleGlowEffects();
	}
}

void ACheckPoint::HighlightActor_Implementation()
{
	CheckpointMesh->SetRenderCustomDepth(true);
}

void ACheckPoint::UnHighlightActor_Implementation()
{
	CheckpointMesh->SetRenderCustomDepth(false);
}

void ACheckPoint::SetMoveToLocation_Implementation(FVector& OutDestination)
{
	OutDestination = MoveToComponent->GetComponentLocation();
}

void ACheckPoint::BeginPlay()
{
	Super::BeginPlay();

	//绑定重叠事件
	if(bBindOverlapCallback)
	{
		Sphere->OnComponentBeginOverlap.AddDynamic(this, &ACheckPoint::OnSphereOverlap);	
	}

}

void ACheckPoint::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//if(OtherActor->ActorHasTag("Player")) //如果只需要判断是不是玩家角色通过标签判断即可
	if(OtherActor->Implements<UPlayerInterface>())
	{
		//如果检查点还未被激活，则进行让检查点高亮处理
		if(!bReached) HandleGlowEffects();
		
		//设置当前检查点已被玩家激活
		bReached = true;

		if(ARPGGameMode* RPGGameMode = Cast<ARPGGameMode>(UGameplayStatics::GetGameMode(this)))
		{
			//保存场景状态
			RPGGameMode->SaveWorldState(GetWorld());
		}
		
		//修改存档当的检测点
		IPlayerInterface::Execute_SaveProgress(OtherActor, PlayerStartTag);
		
	}
}

void ACheckPoint::HandleGlowEffects()
{
	//取消碰撞检查 不取消，可以每次触碰保存一次
	// Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//创建一个新材质实例，修改效果
	UMaterialInstanceDynamic* DynamicMaterialInstance = UMaterialInstanceDynamic::Create(CheckpointMesh->GetMaterial(0), this);
	CheckpointMesh->SetMaterial(0, DynamicMaterialInstance);
	CheckpointReached(DynamicMaterialInstance); //触发检查点修改材质后的回调
}
