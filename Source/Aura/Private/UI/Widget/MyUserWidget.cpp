// 版权归暮志未晚所有。


#include "UI/Widget/MyUserWidget.h"

void UMyUserWidget::SetWidgetController(UObject* InWidgetController)
{
	WidgetController = InWidgetController;
	WidgetControllerSet();
}
