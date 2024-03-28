// 版权归暮志未晚所有。


#include "AbilitySystem/MyAbilitySystemBlueprintLibrary.h"

#include "Kismet/GameplayStatics.h"
#include "Player/PlayerStateBase.h"
#include "UI/HUD/MyHUD.h"
#include "UI/WidgetController/MyWidgetController.h"

UOverlayWidgetController* UMyAbilitySystemBlueprintLibrary::GetOverlayWidgetController(const UObject* WorldContextObject)
{
	//获取到playerController， 需要传入一个世界空间上下文的对象，用于得到对应世界中的PC列表，0为本地使用的PC
	if(APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContextObject, 0))
	{
		//从PC获取到HUD，我们就可以从HUD获得对应的Controller
		if(AMyHUD* HUD = Cast<AMyHUD>(PC->GetHUD()))
		{
			APlayerStateBase* PS = PC->GetPlayerState<APlayerStateBase>();
			UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
			UAttributeSet* AS = PS->GetAttributeSet();
			const FWidgetControllerParams WidgetControllerParams(PC, PS, ASC, AS);
			return HUD->GetOverlayWidgetController(WidgetControllerParams);
		}
	}
	return nullptr;
}

UAttributeMenuWidgetController* UMyAbilitySystemBlueprintLibrary::GetAttributeMenuWidgetController(const UObject* WorldContextObject)
{
	//获取到playerController， 需要传入一个世界空间上下文的对象，用于得到对应世界中的PC列表，0为本地使用的PC
	if(APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContextObject, 0))
	{
		//从PC获取到HUD，我们就可以从HUD获得对应的Controller
		if(AMyHUD* HUD = Cast<AMyHUD>(PC->GetHUD()))
		{
			APlayerStateBase* PS = PC->GetPlayerState<APlayerStateBase>();
			UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
			UAttributeSet* AS = PS->GetAttributeSet();
			const FWidgetControllerParams WidgetControllerParams(PC, PS, ASC, AS);
			return HUD->GetAttributeMenuWidgetController(WidgetControllerParams);
		}
	}
	return nullptr;
}
