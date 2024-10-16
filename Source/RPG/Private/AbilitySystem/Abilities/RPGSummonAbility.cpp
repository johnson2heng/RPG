// 版权归暮志未晚所有。


#include "AbilitySystem/Abilities/RPGSummonAbility.h"

#include "Kismet/KismetSystemLibrary.h"

TArray<FVector> URPGSummonAbility::GetSpawnLocations()
{
	const FVector Forward = GetAvatarActorFromActorInfo()->GetActorForwardVector(); //获取召唤师的朝向
	const FVector Location = GetAvatarActorFromActorInfo()->GetActorLocation(); //获取召唤师的位置
	const float DeltaSpread = SpawnSpread / (NumMinions - 1.f); //将召唤的角度范围进行分段，在每段里面生成一个召唤物

	const FVector LeftOfSpread = Forward.RotateAngleAxis(-SpawnSpread / 2.f, FVector::UpVector); //获取到最左侧的角度

	TArray<FVector> SpawnLocations;
	for(int32 i = 0; i < NumMinions; i++) //遍历，在每个分段上面获取位置
	{
		const FVector Direction = LeftOfSpread.RotateAngleAxis(DeltaSpread * i, FVector::UpVector); //获取当前分段的角度
		FVector ChosenSpawnLocation = Location + Direction * FMath::FRandRange(MinSpawnDistance, MaxSpawnDistance); //随机位置，加上原始位置就是偏移的位置

		//通过射线拾取位置，保证角色贴地生成
		FHitResult Hit;
		GetWorld()->LineTraceSingleByChannel(Hit, ChosenSpawnLocation + FVector(0.f, 0.f, 400.f), ChosenSpawnLocation - FVector(0.f, 0.f, 400.f), ECC_Visibility);
		if(Hit.bBlockingHit)
		{
			ChosenSpawnLocation = Hit.ImpactPoint;
		}
		// DrawDebugSphere(GetWorld(), ChosenSpawnLocation, 15.f,12,FColor::Green,false, 3.f);
		SpawnLocations.Add(ChosenSpawnLocation);
	}
	
	// const FVector RightOfSpread = Forward.RotateAngleAxis(SpawnSpread / 2.f, FVector::UpVector); //获取到最右侧的角度
	
	// UKismetSystemLibrary::DrawDebugArrow(GetAvatarActorFromActorInfo(), Location, Location + LeftOfSpread * MaxSpawnDistance, 4.f, FLinearColor::Red, 3.f);
	// UKismetSystemLibrary::DrawDebugArrow(GetAvatarActorFromActorInfo(), Location, Location + RightOfSpread * MaxSpawnDistance, 4.f, FLinearColor::Black, 3.f);
	//
	// DrawDebugSphere(GetWorld(), Location + LeftOfSpread * MinSpawnDistance, 15.f,12,FColor::Blue,false, 3.f);
	// DrawDebugSphere(GetWorld(), Location + LeftOfSpread * MaxSpawnDistance, 15.f,12,FColor::Blue,false, 3.f);
	// DrawDebugSphere(GetWorld(), Location + RightOfSpread * MinSpawnDistance, 15.f,12,FColor::Blue,false, 3.f);
	// DrawDebugSphere(GetWorld(), Location + RightOfSpread * MaxSpawnDistance, 15.f,12,FColor::Blue,false, 3.f);

	return SpawnLocations;
}

TSubclassOf<APawn> URPGSummonAbility::GetRandomMinionClass()
{
	const int32 Selection = FMath::RandRange(0, MinionClasses.Num() - 1);
	return MinionClasses[Selection];
}
