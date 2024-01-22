// 版权归暮志未晚所有。

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MyUserWidget.generated.h"

/**
 * 用户控件基类
 */
UCLASS()
class AURA_API UMyUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable) //蓝图可调用
	void SetWidgetController(UObject* InWidgetController);

	UPROPERTY(BlueprintReadOnly) //蓝图可读取，但不能修改
	TObjectPtr<UObject> WidgetController;

protected:
	UFUNCTION(BlueprintImplementableEvent) //c++里不能定义，可以调用，蓝图中 无返回值可作为通知，有返回值还可以覆盖重写
	void WidgetControllerSet();
};
