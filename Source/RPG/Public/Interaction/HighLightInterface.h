// 版权归暮志未晚所有。

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HighLightInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType)
class UHighLightInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class RPG_API IHighLightInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	
	//高亮 描边
	UFUNCTION(BlueprintNativeEvent)
	void HighlightActor();
	
	//取消高亮 描边
	UFUNCTION(BlueprintNativeEvent)
	void UnHighlightActor(); 
};
