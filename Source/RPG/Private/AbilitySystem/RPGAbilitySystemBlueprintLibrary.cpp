// 版权归暮志未晚所有。


#include "AbilitySystem/RPGAbilitySystemBlueprintLibrary.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "RPGAbilityTypes.h"
#include "RPGGameplayTags.h"
#include "Game/RPGGameMode.h"
#include "Interaction/CombatInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Player/RPGPlayerState.h"
#include "UI/HUD/RPGHUD.h"
#include "UI/WidgetController/RPGWidgetController.h"

bool URPGAbilitySystemBlueprintLibrary::MakeWidgetControllerParams(const UObject* WorldContextObject, FWidgetControllerParams& OutWcParams, ARPGHUD*& OutRPGHUD)
{
	//获取到playerController， 需要传入一个世界空间上下文的对象，用于得到对应世界中的PC列表，0为本地使用的PC
	if(APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContextObject, 0))
	{
		//从PC获取到HUD，我们就可以从HUD获得对应的Controller
		if(ARPGHUD* HUD = Cast<ARPGHUD>(PC->GetHUD()))
		{
			OutRPGHUD = HUD; //修改指针的引用
			ARPGPlayerState* PS = PC->GetPlayerState<ARPGPlayerState>();
			//设置参数
			OutWcParams.PlayerController = PC;
			OutWcParams.PlayerState = PS;
			OutWcParams.AbilitySystemComponent = PS->GetAbilitySystemComponent();
			OutWcParams.AttributeSet = PS->GetAttributeSet();
			return true;
		}
	}
	return false;
}

UOverlayWidgetController* URPGAbilitySystemBlueprintLibrary::GetOverlayWidgetController(const UObject* WorldContextObject)
{
	FWidgetControllerParams WCParams;
	ARPGHUD* HUD = nullptr;
	if(MakeWidgetControllerParams(WorldContextObject, WCParams, HUD))
	{
		return HUD->GetOverlayWidgetController(WCParams);
	}
	return nullptr;
}

UAttributeMenuWidgetController* URPGAbilitySystemBlueprintLibrary::GetAttributeMenuWidgetController(const UObject* WorldContextObject)
{
	FWidgetControllerParams WCParams;
	ARPGHUD* HUD = nullptr;
	if(MakeWidgetControllerParams(WorldContextObject, WCParams, HUD))
	{
		return HUD->GetAttributeMenuWidgetController(WCParams);
	}
	return nullptr;
}

USpellMenuWidgetController* URPGAbilitySystemBlueprintLibrary::GetSpellMenuWidgetController(const UObject* WorldContextObject)
{
	FWidgetControllerParams WCParams;
	ARPGHUD* HUD = nullptr;
	if(MakeWidgetControllerParams(WorldContextObject, WCParams, HUD))
	{
		return HUD->GetSpellMenuWidgetController(WCParams);
	}
	return nullptr;
}

float URPGAbilitySystemBlueprintLibrary::GetScalableFloatValue(const FScalableFloat ScalableFloat, const int32 Level)
{
	return ScalableFloat.GetValueAtLevel(Level);
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

void URPGAbilitySystemBlueprintLibrary::GiveStartupAbilities(const UObject* WorldContextObject, UAbilitySystemComponent* ASC, ECharacterClass CharacterClass)
{
	//从实例获取到关卡角色的配置
	UCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);
	if(CharacterClassInfo == nullptr) return;

	//从战斗接口获取到角色的等级
	int32 CharacterLevel = 1;
	if(ASC->GetAvatarActor()->Implements<UCombatInterface>())
	{
		CharacterLevel = ICombatInterface::Execute_GetPlayerLevel(ASC->GetAvatarActor());
	}

	//应用角色拥有的技能数组
	for(const TSubclassOf<UGameplayAbility> AbilityClass : CharacterClassInfo->CommonAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, CharacterLevel); //创建技能实例
		ASC->GiveAbility(AbilitySpec); //只应用不激活
	}

	//获取到默认的基础角色数据
	const FCharacterClassDefaultInfo ClassDefaultInfo = CharacterClassInfo->GetClassDefaultInfo(CharacterClass);

	//应用职业技能数组
	for(const TSubclassOf<UGameplayAbility> AbilityClass : ClassDefaultInfo.StartupAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, CharacterLevel); //创建技能实例
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

UAbilityInfo* URPGAbilitySystemBlueprintLibrary::GetAbilityInfo(const UObject* WorldContextObject)
{
	//获取到当前关卡的GameMode实例
	const ARPGGameMode* GameMode = Cast<ARPGGameMode>(UGameplayStatics::GetGameMode(WorldContextObject));
	if(GameMode == nullptr) return nullptr;

	//返回关卡的角色的配置
	return  GameMode->AbilityInfo;
}

int32 URPGAbilitySystemBlueprintLibrary::GetXPRewardForClassAndLevel(const UObject* WorldContextObject, ECharacterClass CharacterClass, int32 CharacterLevel)
{
	//从实例获取到关卡角色的配置
	UCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);
	if(CharacterClassInfo == nullptr) return 0;

	//获取到默认的基础角色数据
	const FCharacterClassDefaultInfo& ClassDefaultInfo = CharacterClassInfo->GetClassDefaultInfo(CharacterClass);

	const float XPReward = ClassDefaultInfo.XPReward.GetValueAtLevel(CharacterLevel);

	return static_cast<int32>(XPReward);
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

bool URPGAbilitySystemBlueprintLibrary::IsSuccessfulDeBuff(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if(const FRPGGameplayEffectContext* RPGEffectContext = static_cast<const FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return RPGEffectContext->IsSuccessfulDeBuff();
	}
	return false;
}

float URPGAbilitySystemBlueprintLibrary::GetDeBuffDamage(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if(const FRPGGameplayEffectContext* RPGEffectContext = static_cast<const FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return RPGEffectContext->GetDeBuffDamage();
	}
	return 0.f;
}

float URPGAbilitySystemBlueprintLibrary::GetDeBuffDuration(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if(const FRPGGameplayEffectContext* RPGEffectContext = static_cast<const FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return RPGEffectContext->GetDeBuffDuration();
	}
	return 0.f;
}

float URPGAbilitySystemBlueprintLibrary::GetDeBuffFrequency(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if(const FRPGGameplayEffectContext* RPGEffectContext = static_cast<const FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return RPGEffectContext->GetDeBuffFrequency();
	}
	return 0.f;
}

FGameplayTag URPGAbilitySystemBlueprintLibrary::GetDeBuffDamageType(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if(const FRPGGameplayEffectContext* RPGEffectContext = static_cast<const FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		//如果当前存在设置了伤害类型
		if(RPGEffectContext->GetDeBuffDamageType().IsValid())
		{
			//取消指针
			return *RPGEffectContext->GetDeBuffDamageType();
		}
	}
	return FGameplayTag();
}

FVector URPGAbilitySystemBlueprintLibrary::GetDeathImpulse(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if(const FRPGGameplayEffectContext* RPGEffectContext = static_cast<const FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return RPGEffectContext->GetDeathImpulse();
	}
	return FVector::ZeroVector;
}

void URPGAbilitySystemBlueprintLibrary::SetIsBlockHit(FGameplayEffectContextHandle& EffectContextHandle, const bool bInIsBlockedHit)
{
	FRPGGameplayEffectContext* RPGEffectContext = static_cast<FRPGGameplayEffectContext*>(EffectContextHandle.Get());
	RPGEffectContext->SetIsBlockedHit(bInIsBlockedHit);
}

void URPGAbilitySystemBlueprintLibrary::SetIsCriticalHit(FGameplayEffectContextHandle& EffectContextHandle, const bool bInIsCriticalHit)
{
	FRPGGameplayEffectContext* RPGEffectContext = static_cast<FRPGGameplayEffectContext*>(EffectContextHandle.Get());
	RPGEffectContext->SetIsCriticalHit(bInIsCriticalHit);
}

void URPGAbilitySystemBlueprintLibrary::SetIsSuccessfulDeBuff(FGameplayEffectContextHandle& EffectContextHandle, const bool bInIsSuccessfulDeBuff)
{
	FRPGGameplayEffectContext* RPGEffectContext = static_cast<FRPGGameplayEffectContext*>(EffectContextHandle.Get());
	RPGEffectContext->SetIsSuccessfulDeBuff(bInIsSuccessfulDeBuff);
}

void URPGAbilitySystemBlueprintLibrary::SetDeBuff(FGameplayEffectContextHandle& EffectContextHandle, FGameplayTag& InDamageType, const float InDamage, const float InDuration, const float InFrequency)
{
	FRPGGameplayEffectContext* RPGEffectContext = static_cast<FRPGGameplayEffectContext*>(EffectContextHandle.Get());
	//通过标签创建一个共享指针
	const TSharedPtr<FGameplayTag> DamageType = MakeShared<FGameplayTag>(InDamageType);
	RPGEffectContext->SetDeBuffDamageType(DamageType);
	RPGEffectContext->SetDeBuffDamage(InDamage);
	RPGEffectContext->SetDeBuffDuration(InDuration);
	RPGEffectContext->SetDeBuffFrequency(InFrequency);
}

void URPGAbilitySystemBlueprintLibrary::SetDeathImpulse(FGameplayEffectContextHandle& EffectContextHandle, const FVector& bInDeathImpulse)
{
	FRPGGameplayEffectContext* RPGEffectContext = static_cast<FRPGGameplayEffectContext*>(EffectContextHandle.Get());
	RPGEffectContext->SetDeathImpulse(bInDeathImpulse);
}

FGameplayEffectContextHandle URPGAbilitySystemBlueprintLibrary::ApplyDamageEffect(const FDamageEffectParams& DamageEffectParams)
{
	const FRPGGameplayTags& GameplayTags = FRPGGameplayTags::Get();
	const AActor* SourceAvatarActor = DamageEffectParams.SourceAbilitySystemComponent->GetAvatarActor();

	//创建GE的上下文句柄
	FGameplayEffectContextHandle EffectContextHandle = DamageEffectParams.SourceAbilitySystemComponent->MakeEffectContext();
	EffectContextHandle.AddSourceObject(SourceAvatarActor);

	//设置死亡冲击
	SetDeathImpulse(EffectContextHandle, DamageEffectParams.DeathImpulse);

	//根据句柄和类创建GE实例
	const FGameplayEffectSpecHandle SpecHandle = DamageEffectParams.SourceAbilitySystemComponent->MakeOutgoingSpec(DamageEffectParams.DamageGameplayEffectClass, DamageEffectParams.AbilityLevel, EffectContextHandle);

	//通过标签设置GE使用的配置
	for(auto& Pair : DamageEffectParams.DamageTypes)
	{
		UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, Pair.Key, Pair.Value);
	}
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.DeBuff_Chance, DamageEffectParams.DeBuffChance);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, DamageEffectParams.DeBuffDamageType, DamageEffectParams.DeBuffDamage);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.DeBuff_Duration, DamageEffectParams.DeBuffDuration);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.DeBuff_Frequency, DamageEffectParams.DeBuffFrequency);

	//将GE应用给目标ASC
	DamageEffectParams.TargetAbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	return EffectContextHandle;
}

void URPGAbilitySystemBlueprintLibrary::GetLivePlayersWithinRadius(const UObject* WorldContextObject, TArray<AActor*>& OutOverlappingActors, const TArray<AActor*>& ActorsToIgnore, float Radius, const FVector& SphereOrigin)
{
	FCollisionQueryParams SphereParams; //创建一个碰撞查询的配置
	SphereParams.AddIgnoredActors(ActorsToIgnore); //添加忽略的Actor

	if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull)) //获取当前所处的场景，如果获取失败，将打印并返回Null
	{
		TArray<FOverlapResult> Overlaps;
		//获取到所有与此球体碰撞的动态物体
		World->OverlapMultiByObjectType(Overlaps, SphereOrigin, FQuat::Identity, FCollisionObjectQueryParams(FCollisionObjectQueryParams::InitType::AllDynamicObjects), FCollisionShape::MakeSphere(Radius), SphereParams);
		for(FOverlapResult& Overlap : Overlaps) //遍历所有获取到的动态Actor
		{
			//判断当前Actor是否包含战斗接口   Overlap.GetActor() 从碰撞检测结果中获取到碰撞的Actor
			const bool ImplementsCombatInterface =  Overlap.GetActor()->Implements<UCombatInterface>();
			//判断当前Actor是否存活，如果不包含战斗接口，将不会判断存活（放置的火堆也属于动态Actor，这样保证不会报错）
			if(ImplementsCombatInterface && !ICombatInterface::Execute_IsDead(Overlap.GetActor())) 
			{
				OutOverlappingActors.AddUnique(Overlap.GetActor()); //将Actor添加到返回数组，AddUnique 只有在此Actor未被添加时，才可以添加到数组
			}
		}
	}
}

bool URPGAbilitySystemBlueprintLibrary::IsNotFriend(const AActor* FirstActor, const AActor* SecondActor)
{
	if(FirstActor->ActorHasTag("Player"))
	{
		return !SecondActor->ActorHasTag("Player");
	}

	if(FirstActor->ActorHasTag("Enemy"))
	{
		return !SecondActor->ActorHasTag("Enemy");
	}

	return false;
}

