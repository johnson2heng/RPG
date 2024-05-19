// 版权归暮志未晚所有。


#include "AbilitySystem/RPGAbilitySystemBlueprintLibrary.h"

#include "RPGAbilityTypes.h"
#include "Game/RPGGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Player/RPGPlayerState.h"
#include "UI/HUD/RPGHUD.h"
#include "UI/WidgetController/RPGWidgetController.h"

UOverlayWidgetController* URPGAbilitySystemBlueprintLibrary::GetOverlayWidgetController(const UObject* WorldContextObject)
{
	//获取到playerController， 需要传入一个世界空间上下文的对象，用于得到对应世界中的PC列表，0为本地使用的PC
	if(APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContextObject, 0))
	{
		//从PC获取到HUD，我们就可以从HUD获得对应的Controller
		if(ARPGHUD* HUD = Cast<ARPGHUD>(PC->GetHUD()))
		{
			ARPGPlayerState* PS = PC->GetPlayerState<ARPGPlayerState>();
			UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
			UAttributeSet* AS = PS->GetAttributeSet();
			const FWidgetControllerParams WidgetControllerParams(PC, PS, ASC, AS);
			return HUD->GetOverlayWidgetController(WidgetControllerParams);
		}
	}
	return nullptr;
}

UAttributeMenuWidgetController* URPGAbilitySystemBlueprintLibrary::GetAttributeMenuWidgetController(const UObject* WorldContextObject)
{
	//获取到playerController， 需要传入一个世界空间上下文的对象，用于得到对应世界中的PC列表，0为本地使用的PC
	if(APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContextObject, 0))
	{
		//从PC获取到HUD，我们就可以从HUD获得对应的Controller
		if(ARPGHUD* HUD = Cast<ARPGHUD>(PC->GetHUD()))
		{
			ARPGPlayerState* PS = PC->GetPlayerState<ARPGPlayerState>();
			UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
			UAttributeSet* AS = PS->GetAttributeSet();
			const FWidgetControllerParams WidgetControllerParams(PC, PS, ASC, AS);
			return HUD->GetAttributeMenuWidgetController(WidgetControllerParams);
		}
	}
	return nullptr;
}

void URPGAbilitySystemBlueprintLibrary::InitializeDefaultAttributes(const UObject* WorldContextObject, ECharacterClass CharacterClass, float Level, UAbilitySystemComponent* ASC)
{
	const AActor* AvatarActor = ASC->GetAvatarActor();

	//从实例获取到关卡角色的配置
	UCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);

	//获取到默认的基础角色数据
	const FCharacterClassDefaultInfo ClassDefaultInfo = CharacterClassInfo->GetClassDefaultInfo(CharacterClass);

	//应用基础属性
	FGameplayEffectContextHandle PrimaryContextHandle = ASC->MakeEffectContext();
	PrimaryContextHandle.AddSourceObject(AvatarActor);
	const FGameplayEffectSpecHandle PrimarySpecHandle = ASC->MakeOutgoingSpec(ClassDefaultInfo.PrimaryAttributes, Level, PrimaryContextHandle);
	ASC->ApplyGameplayEffectSpecToSelf(*PrimarySpecHandle.Data.Get());

	//设置次级属性
	FGameplayEffectContextHandle SecondaryContextHandle = ASC->MakeEffectContext();
	SecondaryContextHandle.AddSourceObject(AvatarActor);
	const FGameplayEffectSpecHandle SecondarySpecHandle = ASC->MakeOutgoingSpec(CharacterClassInfo->SecondaryAttributes, Level, SecondaryContextHandle);
	ASC->ApplyGameplayEffectSpecToSelf(*SecondarySpecHandle.Data.Get());

	//填充血量和蓝量
	FGameplayEffectContextHandle VitalContextHandle = ASC->MakeEffectContext();
	VitalContextHandle.AddSourceObject(AvatarActor);
	const FGameplayEffectSpecHandle VitalSpecHandle = ASC->MakeOutgoingSpec(CharacterClassInfo->VitalAttributes, Level, VitalContextHandle);
	ASC->ApplyGameplayEffectSpecToSelf(*VitalSpecHandle.Data.Get());
}

void URPGAbilitySystemBlueprintLibrary::GiveStartupAbilities(const UObject* WorldContextObject, UAbilitySystemComponent* ASC)
{
	//从实例获取到关卡角色的配置
	UCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);

	//遍历角色拥有的技能数组
	for(const TSubclassOf<UGameplayAbility> AbilityClass : CharacterClassInfo->CommonAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1); //创建技能实例
		ASC->GiveAbility(AbilitySpec); //只应用不激活
	}
}

UCharacterClassInfo* URPGAbilitySystemBlueprintLibrary::GetCharacterClassInfo(const UObject* WorldContextObject)
{
	//获取到当前关卡的GameMode实例
	const ARPGGameMode* GameMode = Cast<ARPGGameMode>(UGameplayStatics::GetGameMode(WorldContextObject));
	if(GameMode == nullptr) return nullptr;

	//返回关卡的角色的配置
	return  GameMode->CharacterClassInfo;
}

bool URPGAbilitySystemBlueprintLibrary::IsBlockedHit(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if(const FRPGGameplayEffectContext* RPGEffectContext = static_cast<const FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return RPGEffectContext->IsBlockedHit();
	}
	return false;
}

bool URPGAbilitySystemBlueprintLibrary::IsCriticalHit(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if(const FRPGGameplayEffectContext* RPGEffectContext = static_cast<const FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return RPGEffectContext->IsCriticalHit();
	}
	return false;
}

void URPGAbilitySystemBlueprintLibrary::SetIsBlockHit(FGameplayEffectContextHandle& EffectContextHandle,
	bool bInIsBlockedHit)
{
	FRPGGameplayEffectContext* RPGEffectContext = static_cast<FRPGGameplayEffectContext*>(EffectContextHandle.Get());
	RPGEffectContext->SetIsBlockedHit(bInIsBlockedHit);
}

void URPGAbilitySystemBlueprintLibrary::SetIsCriticalHit(FGameplayEffectContextHandle& EffectContextHandle,
	bool bInIsCriticalHit)
{
	FRPGGameplayEffectContext* RPGEffectContext = static_cast<FRPGGameplayEffectContext*>(EffectContextHandle.Get());
	RPGEffectContext->SetIsCriticalHit(bInIsCriticalHit);
}
