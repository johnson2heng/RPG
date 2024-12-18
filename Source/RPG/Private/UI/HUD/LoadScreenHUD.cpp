// 版权归暮志未晚所有。


#include "UI/HUD/LoadScreenHUD.h"
#include "Blueprint/UserWidget.h"
#include "UI/MVVM/MVVM_LoadScreen.h"
#include "UI/Widget/LoadScreenWidget.h"

void ALoadScreenHUD::BeginPlay()
{
	Super::BeginPlay();

	//实例化MVVM
	LoadScreenViewModel = NewObject<UMVVM_LoadScreen>(this, LoadScreenViewModelClass);
	LoadScreenViewModel->SetWidgetName("WidgetName"); //测试代码
	LoadScreenViewModel->InitializeLoadSlots(); //初始化插槽使用的MVVM
 
	//创建用户控件并添加到视口
	LoadScreenWidget = CreateWidget<ULoadScreenWidget>(GetWorld(), LoadScreenWidgetClass);
	LoadScreenWidget->AddToViewport();

	APlayerController* PC = GetOwningPlayerController();
	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(LoadScreenWidget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	
	PC->SetInputMode(InputMode);
	PC->SetShowMouseCursor(true);

	//创建完成用户控件后，调用用户控件函数
	LoadScreenWidget->BlueprintInitializeWidget();

	//加载存档
	LoadScreenViewModel->LoadData();
}
