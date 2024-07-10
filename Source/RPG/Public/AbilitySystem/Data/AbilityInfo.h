// 版权归暮志未晚所有。

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "AbilityInfo.generated.h"

USTRUCT(BlueprintType)
struct FRPGAbilityInfo
{
	GENERATED_BODY()

	//技能标签
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag AbilityTag = FGameplayTag();

	//技能输入映射标签
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag InputTag = FGameplayTag();

	//技能图标
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<const UTexture2D> Icon = nullptr;

	//背景材质
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UMaterialInterface> BackgroundMaterial = nullptr;
};

/**
 * 
 */
UCLASS()
class RPG_API UAbilityInfo : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="AbilityInformation")
	TArray<FRPGAbilityInfo> AbilityInformation;

	//通过技能标签获取到技能相关数据
	FRPGAbilityInfo FindAbilityInfoForTag(const FGameplayTag& AbilityTag, const bool bLogNotFound = false) const;
};
