// 版权归暮志未晚所有。


#include "UI/HUD/RPGHUD.h"
#include "UI/Widget/RPGUserWidget.h"
#include "UI/WidgetController/OverlayWidgetController.h"
#include "UI/WidgetController/AttributeMenuWidgetController.h"
#include "UI/WidgetController/SpellMenuWidgetController.h"

UOverlayWidgetController* ARPGHUD::GetOverlayWidgetController(const FWidgetControllerParams& WCParams)
{
	if(OverlayWidgetController == nullptr)
	{
		OverlayWidgetController = NewObject<UOverlayWidgetController>(this, OverlayWidgetControllerClass);
		OverlayWidgetController->SetWidgetControllerParams(WCParams);
		OverlayWidgetController->BindCallbacksToDependencies(); //绑定监听数值变化
	}
	return OverlayWidgetController;
}

UAttributeMenuWidgetController* ARPGHUD::GetAttributeMenuWidgetController(const FWidgetControllerParams& WCParams)
{
	if(AttributeMenuWidgetController == nullptr)
	{
		AttributeMenuWidgetController = NewObject<UAttributeMenuWidgetController>(this, AttributeMenuWidgetControllerClass);
		AttributeMenuWidgetController->SetWidgetControllerParams(WCParams);
		AttributeMenuWidgetController->BindCallbacksToDependencies(); //绑定监听数值变化
	}
	return AttributeMenuWidgetController;
}

USpellMenuWidgetController* ARPGHUD::GetSpellMenuWidgetController(const FWidgetControllerParams& WCParams)
{
	if(SpellMenuWidgetController == nullptr)
	{
		SpellMenuWidgetController = NewObject<USpellMenuWidgetController>(this, SpellMenuWidgetControllerClass);
		SpellMenuWidgetController->SetWidgetControllerParams(WCParams);
		SpellMenuWidgetController->BindCallbacksToDependencies(); //绑定监听数值变化
	}
	return SpellMenuWidgetController;
}

void ARPGHUD::InitOverlay(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS)
{
	checkf(OverlayWidgetClass, TEXT("OverlayWidgetClass 未设置，请在HUD上面设置")); //会打印相关信息到log列表
	checkf(OverlayWidgetControllerClass, TEXT("OverlayWidgetControllerClass 未设置，请在HUD上面设置"));
	
	URPGUserWidget* Widget = CreateWidget<URPGUserWidget>(GetWorld(), OverlayWidgetClass); //创建Overlay用户控件
	OverlayWidget = Cast<URPGUserWidget>(Widget);

	const FWidgetControllerParams WidgetControllerParams(PC, PS, ASC, AS); //创建参数结构体
	OverlayWidgetController = GetOverlayWidgetController(WidgetControllerParams); //获取控制器层

	//初始化所有控制器
	GetAttributeMenuWidgetController(WidgetControllerParams);
	GetSpellMenuWidgetController(WidgetControllerParams);

	OverlayWidget->SetWidgetController(OverlayWidgetController); //设置用户控件的控制器层
	OverlayWidgetController->BroadcastInitialValues(); //初始化广播的值，在所有的UI绑定后，调用一次，实现数值在UI上的初始化
	
	Widget->AddToViewport(); //添加到视口
}

