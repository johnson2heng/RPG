// 版权归暮志未晚所有。


#include "MyAssetManager.h"
#include "MyGameplayTags.h"

UMyAssetManager& UMyAssetManager::Get()
{
	check(GEngine);

	UMyAssetManager* MyAssetManager = Cast<UMyAssetManager>(GEngine->AssetManager);
	return *MyAssetManager;
}

void UMyAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();

	FMyGameplayTags::InitializeNativeGameplayTags();
}
