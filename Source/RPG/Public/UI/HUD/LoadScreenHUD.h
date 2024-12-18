// 版权归暮志未晚所有。

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "LoadScreenHUD.generated.h"

class UMVVM_LoadScreen;
class ULoadScreenWidget;
/**
 * 
 */
UCLASS()
class RPG_API ALoadScreenHUD : public AHUD
{
	GENERATED_BODY()

public:

	//存档关卡用户控件类
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ULoadScreenWidget> LoadScreenWidgetClass;

	//用户控件实例
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<ULoadScreenWidget> LoadScreenWidget;

	//MVVM使用的类
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UMVVM_LoadScreen> LoadScreenViewModelClass;

	//MVVM生成的实例
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UMVVM_LoadScreen> LoadScreenViewModel;

protected:
	virtual void BeginPlay() override;
};
