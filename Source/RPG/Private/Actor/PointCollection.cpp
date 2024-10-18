// 版权归暮志未晚所有。


#include "Actor/PointCollection.h"

#include "AbilitySystem/RPGAbilitySystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
APointCollection::APointCollection()
{
	PrimaryActorTick.bCanEverTick = false;

	Pt_0 = CreateDefaultSubobject<USceneComponent>("Pt_0");
	ImmutablePts.Add(Pt_0);
	SetRootComponent(Pt_0);
	
	CreateSceneComponent(Pt_1, "Pt_1");
	CreateSceneComponent(Pt_2, "Pt_2");
	CreateSceneComponent(Pt_3, "Pt_3");
	CreateSceneComponent(Pt_4, "Pt_4");
	CreateSceneComponent(Pt_5, "Pt_5");
	CreateSceneComponent(Pt_6, "Pt_6");
	CreateSceneComponent(Pt_7, "Pt_7");
	CreateSceneComponent(Pt_8, "Pt_8");
	CreateSceneComponent(Pt_9, "Pt_9");
	CreateSceneComponent(Pt_10, "Pt_10");
}

void APointCollection::BeginPlay()
{
	Super::BeginPlay();
	
}

void APointCollection::CreateSceneComponent(TObjectPtr<USceneComponent>& Pt, const FName Name)
{
	Pt = CreateDefaultSubobject<USceneComponent>(Name);
	ImmutablePts.Add(Pt);
	Pt->SetupAttachment(GetRootComponent());
}

void APointCollection::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

TArray<USceneComponent*> APointCollection::GetGroundPoints(const FVector& GroundLocation, int32 NumPoints, float YawOverride)
{
	checkf(ImmutablePts.Num() >= NumPoints, TEXT("访问索引超过了数组的范围"));

	TArray<USceneComponent*> ArrayCopy;

	for(USceneComponent* Pt : ImmutablePts)
	{
		if(ArrayCopy.Num() >= NumPoints) return ArrayCopy;

		if(Pt != Pt_0)
		{
			FVector ToPoint = Pt->GetComponentLocation() - Pt_0->GetComponentLocation();
			ToPoint = ToPoint.RotateAngleAxis(YawOverride, FVector::UpVector);
			Pt->SetWorldLocation(Pt_0->GetComponentLocation() + ToPoint);
		}

		const FVector RaisedLocation = FVector(Pt->GetComponentLocation().X, Pt->GetComponentLocation().Y, Pt->GetComponentLocation().Z + 500.f);
		const FVector LoweredLocation = FVector(Pt->GetComponentLocation().X, Pt->GetComponentLocation().Y, Pt->GetComponentLocation().Z - 500.f);

		FHitResult HitResult;
		TArray<AActor*> IgnoreActors;
		URPGAbilitySystemLibrary::GetLivePlayersWithinRadius(this, IgnoreActors, TArray<AActor*>(), 1500.f, GetActorLocation());

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActors(IgnoreActors);
		GetWorld()->LineTraceSingleByProfile(HitResult, RaisedLocation, LoweredLocation, FName("BlockAll"), QueryParams);

		const FVector AdjustedLocation = FVector(Pt->GetComponentLocation().X, Pt->GetComponentLocation().Y, HitResult.ImpactPoint.Z);
		Pt->SetWorldLocation(AdjustedLocation);
		Pt->SetWorldRotation(UKismetMathLibrary::MakeRotFromZ(HitResult.ImpactNormal));

		ArrayCopy.Add(Pt);
	}

	return ArrayCopy;
}

