// 版权归暮志未晚所有。

#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetController/MyWidgetController.h"
#include "AttributeMenuWidgetController.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class AURA_API UAttributeMenuWidgetController : public UMyWidgetController
{
	GENERATED_BODY()

public:

	virtual void BindCallbacksToDependencies() override;
	virtual void BroadcastInitialValues() override;
};
