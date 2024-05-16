// 版权归暮志未晚所有。


#include "RPGAssetManager.h"

#include "AbilitySystemGlobals.h"
#include "RPGGameplayTags.h"

URPGAssetManager& URPGAssetManager::Get()
{
	check(GEngine);

	URPGAssetManager* MyAssetManager = Cast<URPGAssetManager>(GEngine->AssetManager);
	return *MyAssetManager;
}

void URPGAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();

	FRPGGameplayTags::InitializeNativeGameplayTags();

	//如果使用TargetData，必须开启此项
	UAbilitySystemGlobals::Get().InitGlobalData();
}
