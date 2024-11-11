// 版权归暮志未晚所有。

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LoadScreenWidget.generated.h"

/**
 * 
 */
UCLASS()
class RPG_API ULoadScreenWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	//蓝图实现初始化函数，主要用于设置MVVM
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void BlueprintInitializeWidget();
};
