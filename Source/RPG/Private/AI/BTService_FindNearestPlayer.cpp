// 版权归暮志未晚所有。


#include "AI/BTService_FindNearestPlayer.h"

#include "AIController.h"
#include "BehaviorTree/BTFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"

void UBTService_FindNearestPlayer::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	//获取AIController控制的Pawn
	APawn* OwningPawn = AIOwner->GetPawn();
	//根据Pawn自身设置的FName标签来判断敌人的标签
	const FName TargetTag = OwningPawn->ActorHasTag(FName("Player")) ? FName("Enemy") : FName("Player");
	//获取到所有含有此标签的角色
	TArray<AActor*> ActorsWithTag;
	UGameplayStatics::GetAllActorsWithTag(OwningPawn, TargetTag, ActorsWithTag);
	//遍历所有的角色获取到最近的角色和距离
	float ClosestDistance = TNumericLimits<float>::Max(); //默认设置float最大值
	AActor* ClosestActor = nullptr;
	for(AActor* Actor : ActorsWithTag)
	{
		const float Distance = OwningPawn->GetDistanceTo(Actor);
		if(Distance < ClosestDistance)
		{
			ClosestDistance = Distance;
			ClosestActor = Actor;
		}
	}
	//设置黑板数据
	UBTFunctionLibrary::SetBlackboardValueAsObject(this, TargetToFollowSelector, ClosestActor);
	UBTFunctionLibrary::SetBlackboardValueAsFloat(this, DistanceToTargetSelector, ClosestDistance);
}
