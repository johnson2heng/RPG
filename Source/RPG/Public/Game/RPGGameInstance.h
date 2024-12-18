// 版权归暮志未晚所有。

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "RPGGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class RPG_API URPGGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:

	//角色进入关卡后默认生成的PlayerStart的Tag
	UPROPERTY()
	FName PlayerStartTag = FName();

	//当前使用的或后续保存内容到的存档名称
	UPROPERTY()
	FString LoadSlotName = FString();

	//当前使用活后续保存的存档索引
	UPROPERTY()
	int32 LoadSlotIndex = 0;
};
