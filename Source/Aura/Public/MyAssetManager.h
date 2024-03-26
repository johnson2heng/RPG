// 版权归暮志未晚所有。

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "MyAssetManager.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API UMyAssetManager : public UAssetManager
{
	GENERATED_BODY()

public:
	static UMyAssetManager& Get();

protected:
	virtual void StartInitialLoading() override;
};
