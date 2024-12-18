// 版权归暮志未晚所有。

#pragma once

#include "CoreMinimal.h"
#include "Checkpoints/CheckPoint.h"
#include "MapEntrance.generated.h"

/**
 * 
 */
UCLASS()
class RPG_API AMapEntrance : public ACheckPoint
{
	GENERATED_BODY()

public:
	//构造函数
	explicit AMapEntrance(const FObjectInitializer& ObjectInitializer);

	/*   Save Interface   */
	//从存档读取数据后，更新Actor
	virtual void LoadActor_Implementation() override;
	/*   End Save Interface   */

	//传送点需要传送到的目标地图
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UWorld> DestinationMap;

	//传送到目标地图的出生点标签
	UPROPERTY(EditAnywhere)
	FName DestinationPlayerStartTag;

protected:

	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

private:
	
};
