// 版权归暮志未晚所有。


#include "AbilitySystem/RPGAbilitySystemLibrary.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "RPGAbilityTypes.h"
#include "RPGGameplayTags.h"
#include "Components/CapsuleComponent.h"
#include "Engine/DamageEvents.h"
#include "Game/RPGGameMode.h"
#include "GameFramework/Character.h"
#include "Interaction/CombatInterface.h"
#include "Interaction/PlayerInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Player/RPGPlayerState.h"
#include "UI/HUD/RPGHUD.h"
#include "UI/WidgetController/RPGWidgetController.h"

bool URPGAbilitySystemLibrary::MakeWidgetControllerParams(const UObject* WorldContextObject, FWidgetControllerParams& OutWcParams, ARPGHUD*& OutRPGHUD)
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

UOverlayWidgetController* URPGAbilitySystemLibrary::GetOverlayWidgetController(const UObject* WorldContextObject)
{
	FWidgetControllerParams WCParams;
	ARPGHUD* HUD = nullptr;
	if(MakeWidgetControllerParams(WorldContextObject, WCParams, HUD))
	{
		return HUD->GetOverlayWidgetController(WCParams);
	}
	return nullptr;
}

UAttributeMenuWidgetController* URPGAbilitySystemLibrary::GetAttributeMenuWidgetController(const UObject* WorldContextObject)
{
	FWidgetControllerParams WCParams;
	ARPGHUD* HUD = nullptr;
	if(MakeWidgetControllerParams(WorldContextObject, WCParams, HUD))
	{
		return HUD->GetAttributeMenuWidgetController(WCParams);
	}
	return nullptr;
}

USpellMenuWidgetController* URPGAbilitySystemLibrary::GetSpellMenuWidgetController(const UObject* WorldContextObject)
{
	FWidgetControllerParams WCParams;
	ARPGHUD* HUD = nullptr;
	if(MakeWidgetControllerParams(WorldContextObject, WCParams, HUD))
	{
		return HUD->GetSpellMenuWidgetController(WCParams);
	}
	return nullptr;
}

float URPGAbilitySystemLibrary::GetScalableFloatValue(const FScalableFloat& ScalableFloat, const int32& Level)
{
	return ScalableFloat.GetValueAtLevel(Level);
}

void URPGAbilitySystemLibrary::InitializeDefaultAttributes(const UObject* WorldContextObject, ECharacterClass CharacterClass, float Level, UAbilitySystemComponent* ASC)
{
	const AActor* AvatarActor = ASC->GetAvatarActor();

	//从实例获取到关卡角色的配置
	UCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);
	if(CharacterClassInfo == nullptr) return;

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

void URPGAbilitySystemLibrary::InitializeDefaultAttributesFromSaveData(const UObject* WorldContextObject, UAbilitySystemComponent* ASC, ULoadScreenSaveGame* SaveGame)
{
	AActor* AvatarActor = ASC->GetAvatarActor();
	
	const FRPGGameplayTags& GameplayTags = FRPGGameplayTags::Get();

	//从实例获取到关卡角色的配置
	const UCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);
	if(CharacterClassInfo == nullptr) return;

	//*********************************初始化主要属性*********************************

	//创建GE的上下文句柄
	FGameplayEffectContextHandle EffectContextHandle = ASC->MakeEffectContext();
	EffectContextHandle.AddSourceObject(AvatarActor);

	//根据句柄和类创建GE实例，并可以通过句柄找到GE实例
	const FGameplayEffectSpecHandle PrimaryContextHandle = ASC->MakeOutgoingSpec(CharacterClassInfo->PrimaryAttributes_SetByCaller, 1.0f, EffectContextHandle);

	//通过标签设置GE使用的配置
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(PrimaryContextHandle, GameplayTags.Attributes_Primary_Strength, SaveGame->Strength);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(PrimaryContextHandle, GameplayTags.Attributes_Primary_Intelligence, SaveGame->Intelligence);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(PrimaryContextHandle, GameplayTags.Attributes_Primary_Resilience, SaveGame->Resilience);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(PrimaryContextHandle, GameplayTags.Attributes_Primary_Vigor, SaveGame->Vigor);

	//应用GE
	ASC->ApplyGameplayEffectSpecToSelf(*PrimaryContextHandle.Data.Get());

	if(AvatarActor->Implements<UPlayerInterface>())
	{
		//*********************************设置次级属性*********************************
	
		FGameplayEffectContextHandle SecondaryContextHandle = ASC->MakeEffectContext();
		SecondaryContextHandle.AddSourceObject(AvatarActor);
		const FGameplayEffectSpecHandle SecondarySpecHandle = ASC->MakeOutgoingSpec(IPlayerInterface::Execute_GetSecondaryAttributes(AvatarActor), 1.0f, SecondaryContextHandle);
		ASC->ApplyGameplayEffectSpecToSelf(*SecondarySpecHandle.Data.Get());

		//*********************************填充血量和蓝量*********************************

		FGameplayEffectContextHandle VitalContextHandle = ASC->MakeEffectContext();
		VitalContextHandle.AddSourceObject(AvatarActor);
		const FGameplayEffectSpecHandle VitalSpecHandle = ASC->MakeOutgoingSpec(IPlayerInterface::Execute_GetVitalAttributes(AvatarActor), 1.0f, VitalContextHandle);
		ASC->ApplyGameplayEffectSpecToSelf(*VitalSpecHandle.Data.Get());
	}
}

void URPGAbilitySystemLibrary::GiveStartupAbilities(const UObject* WorldContextObject, UAbilitySystemComponent* ASC, ECharacterClass CharacterClass)
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

UCharacterClassInfo* URPGAbilitySystemLibrary::GetCharacterClassInfo(const UObject* WorldContextObject)
{
	//获取到当前关卡的GameMode实例
	const ARPGGameMode* GameMode = Cast<ARPGGameMode>(UGameplayStatics::GetGameMode(WorldContextObject));
	if(GameMode == nullptr) return nullptr;

	//返回关卡的角色的配置
	return  GameMode->CharacterClassInfo;
}

UAbilityInfo* URPGAbilitySystemLibrary::GetAbilityInfo(const UObject* WorldContextObject)
{
	//获取到当前关卡的GameMode实例
	const ARPGGameMode* GameMode = Cast<ARPGGameMode>(UGameplayStatics::GetGameMode(WorldContextObject));
	if(GameMode == nullptr) return nullptr;

	//返回关卡的角色的配置
	return  GameMode->AbilityInfo;
}

int32 URPGAbilitySystemLibrary::GetXPRewardForClassAndLevel(const UObject* WorldContextObject, ECharacterClass CharacterClass, int32 CharacterLevel)
{
	//从实例获取到关卡角色的配置
	UCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);
	if(CharacterClassInfo == nullptr) return 0;

	//获取到默认的基础角色数据
	const FCharacterClassDefaultInfo& ClassDefaultInfo = CharacterClassInfo->GetClassDefaultInfo(CharacterClass);

	const float XPReward = ClassDefaultInfo.XPReward.GetValueAtLevel(CharacterLevel);

	return static_cast<int32>(XPReward);
}

ULootTiers* URPGAbilitySystemLibrary::GetLootTiers(const UObject* WorldContextObject)
{
	//获取到当前关卡的GameMode实例
	const ARPGGameMode* GameMode = Cast<ARPGGameMode>(UGameplayStatics::GetGameMode(WorldContextObject));
	if(GameMode == nullptr) return nullptr;

	//返回敌人战利品配置，需要设置到GameMode上
	return  GameMode->LootTiers;
}

bool URPGAbilitySystemLibrary::IsBlockedHit(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if(const FRPGGameplayEffectContext* RPGEffectContext = static_cast<const FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return RPGEffectContext->IsBlockedHit();
	}
	return false;
}

bool URPGAbilitySystemLibrary::IsCriticalHit(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if(const FRPGGameplayEffectContext* RPGEffectContext = static_cast<const FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return RPGEffectContext->IsCriticalHit();
	}
	return false;
}

bool URPGAbilitySystemLibrary::IsSuccessfulDeBuff(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if(const FRPGGameplayEffectContext* RPGEffectContext = static_cast<const FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return RPGEffectContext->IsSuccessfulDeBuff();
	}
	return false;
}

float URPGAbilitySystemLibrary::GetDeBuffDamage(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if(const FRPGGameplayEffectContext* RPGEffectContext = static_cast<const FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return RPGEffectContext->GetDeBuffDamage();
	}
	return 0.f;
}

float URPGAbilitySystemLibrary::GetDeBuffDuration(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if(const FRPGGameplayEffectContext* RPGEffectContext = static_cast<const FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return RPGEffectContext->GetDeBuffDuration();
	}
	return 0.f;
}

float URPGAbilitySystemLibrary::GetDeBuffFrequency(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if(const FRPGGameplayEffectContext* RPGEffectContext = static_cast<const FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return RPGEffectContext->GetDeBuffFrequency();
	}
	return 0.f;
}

FGameplayTag URPGAbilitySystemLibrary::GetDeBuffDamageType(const FGameplayEffectContextHandle& EffectContextHandle)
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

FVector URPGAbilitySystemLibrary::GetDeathImpulse(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if(const FRPGGameplayEffectContext* RPGEffectContext = static_cast<const FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return RPGEffectContext->GetDeathImpulse();
	}
	return FVector::ZeroVector;
}

FVector URPGAbilitySystemLibrary::GetKnockbackForce(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if(const FRPGGameplayEffectContext* RPGEffectContext = static_cast<const FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return RPGEffectContext->GetKnockbackForce();
	}
	return FVector::ZeroVector;
}

bool URPGAbilitySystemLibrary::IsRadialDamage(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if(const FRPGGameplayEffectContext* RPGEffectContext = static_cast<const FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return RPGEffectContext->IsRadialDamage();
	}
	return false;
}

float URPGAbilitySystemLibrary::GetRadialDamageInnerRadius(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if(const FRPGGameplayEffectContext* RPGEffectContext = static_cast<const FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return RPGEffectContext->GetRadialDamageInnerRadius();
	}
	return 0.f;
}

float URPGAbilitySystemLibrary::GetRadialDamageOuterRadius(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if(const FRPGGameplayEffectContext* RPGEffectContext = static_cast<const FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return RPGEffectContext->GetRadialDamageOuterRadius();
	}
	return 0.f;
}

FVector URPGAbilitySystemLibrary::GetRadialDamageOrigin(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if(const FRPGGameplayEffectContext* RPGEffectContext = static_cast<const FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return RPGEffectContext->GetRadialDamageOrigin();
	}
	return FVector::ZeroVector;
}

void URPGAbilitySystemLibrary::SetIsBlockHit(FGameplayEffectContextHandle& EffectContextHandle, const bool bInIsBlockedHit)
{
	FRPGGameplayEffectContext* RPGEffectContext = static_cast<FRPGGameplayEffectContext*>(EffectContextHandle.Get());
	RPGEffectContext->SetIsBlockedHit(bInIsBlockedHit);
}

void URPGAbilitySystemLibrary::SetIsCriticalHit(FGameplayEffectContextHandle& EffectContextHandle, const bool bInIsCriticalHit)
{
	FRPGGameplayEffectContext* RPGEffectContext = static_cast<FRPGGameplayEffectContext*>(EffectContextHandle.Get());
	RPGEffectContext->SetIsCriticalHit(bInIsCriticalHit);
}

void URPGAbilitySystemLibrary::SetIsSuccessfulDeBuff(FGameplayEffectContextHandle& EffectContextHandle, const bool bInIsSuccessfulDeBuff)
{
	FRPGGameplayEffectContext* RPGEffectContext = static_cast<FRPGGameplayEffectContext*>(EffectContextHandle.Get());
	RPGEffectContext->SetIsSuccessfulDeBuff(bInIsSuccessfulDeBuff);
}

void URPGAbilitySystemLibrary::SetDeBuff(FGameplayEffectContextHandle& EffectContextHandle, FGameplayTag& InDamageType, const float InDamage, const float InDuration, const float InFrequency)
{
	FRPGGameplayEffectContext* RPGEffectContext = static_cast<FRPGGameplayEffectContext*>(EffectContextHandle.Get());
	//通过标签创建一个共享指针
	const TSharedPtr<FGameplayTag> DamageType = MakeShared<FGameplayTag>(InDamageType);
	RPGEffectContext->SetDeBuffDamageType(DamageType);
	RPGEffectContext->SetDeBuffDamage(InDamage);
	RPGEffectContext->SetDeBuffDuration(InDuration);
	RPGEffectContext->SetDeBuffFrequency(InFrequency);
}

void URPGAbilitySystemLibrary::SetDeathImpulse(FGameplayEffectContextHandle& EffectContextHandle, const FVector& InDeathImpulse)
{
	FRPGGameplayEffectContext* RPGEffectContext = static_cast<FRPGGameplayEffectContext*>(EffectContextHandle.Get());
	RPGEffectContext->SetDeathImpulse(InDeathImpulse);
}

void URPGAbilitySystemLibrary::SetKnockbackForce(FGameplayEffectContextHandle& EffectContextHandle, const FVector& InKnockbackForce)
{
	FRPGGameplayEffectContext* RPGEffectContext = static_cast<FRPGGameplayEffectContext*>(EffectContextHandle.Get());
	RPGEffectContext->SetKnockbackForce(InKnockbackForce);
}

void URPGAbilitySystemLibrary::SetIsRadialDamage(FGameplayEffectContextHandle& EffectContextHandle, bool bInIsRadialDamage)
{
	FRPGGameplayEffectContext* RPGEffectContext = static_cast<FRPGGameplayEffectContext*>(EffectContextHandle.Get());
	RPGEffectContext->SetIsRadialDamage(bInIsRadialDamage);
}

void URPGAbilitySystemLibrary::SetRadialDamageInnerRadius(FGameplayEffectContextHandle& EffectContextHandle, float InRadialDamageInnerRadius)
{
	FRPGGameplayEffectContext* RPGEffectContext = static_cast<FRPGGameplayEffectContext*>(EffectContextHandle.Get());
	RPGEffectContext->SetRadialDamageInnerRadius(InRadialDamageInnerRadius);
}

void URPGAbilitySystemLibrary::SetRadialDamageOuterRadius(FGameplayEffectContextHandle& EffectContextHandle, float InRadialDamageOuterRadius)
{
	FRPGGameplayEffectContext* RPGEffectContext = static_cast<FRPGGameplayEffectContext*>(EffectContextHandle.Get());
	RPGEffectContext->SetRadialDamageOuterRadius(InRadialDamageOuterRadius);
}

void URPGAbilitySystemLibrary::SetRadialDamageOrigin(FGameplayEffectContextHandle& EffectContextHandle, const FVector& InRadialDamageOrigin)
{
	FRPGGameplayEffectContext* RPGEffectContext = static_cast<FRPGGameplayEffectContext*>(EffectContextHandle.Get());
	RPGEffectContext->SetRadialDamageOrigin(InRadialDamageOrigin);
}

void URPGAbilitySystemLibrary::SetIsRadialDamageEffectParams(FDamageEffectParams& DamageEffectParams, bool bIsRadial, float InnerRadius, float OutRadius, FVector Origin)
{
	DamageEffectParams.bIsRadialDamage = bIsRadial;
	DamageEffectParams.RadialDamageInnerRadius = InnerRadius;
	DamageEffectParams.RadialDamageOuterRadius = OutRadius;
	DamageEffectParams.RadialDamageOrigin = Origin;
}

void URPGAbilitySystemLibrary::SetKnockbackDirection(FDamageEffectParams& DamageEffectParams, FVector KnockbackDirection, float Magnitude)
{
	KnockbackDirection.Normalize();
	if(Magnitude == 0.f)
	{
		DamageEffectParams.KnockbackForce = KnockbackDirection * DamageEffectParams.KnockbackForceMagnitude;
	}
	else
	{
		DamageEffectParams.KnockbackForce = KnockbackDirection * Magnitude;
	}
}

void URPGAbilitySystemLibrary::SetDeathImpulseDirection(FDamageEffectParams& DamageEffectParams, FVector ImpulseDirection, float Magnitude)
{
	ImpulseDirection.Normalize();
	if(Magnitude == 0.f)
	{
		DamageEffectParams.DeathImpulse = ImpulseDirection * DamageEffectParams.DeathImpulseMagnitude;
	}
	else
	{
		DamageEffectParams.DeathImpulse = ImpulseDirection * Magnitude;
	}
}

void URPGAbilitySystemLibrary::SetEffectParamsTargetASC(FDamageEffectParams& DamageEffectParams, UAbilitySystemComponent* InASC)
{
	DamageEffectParams.TargetAbilitySystemComponent = InASC;
}

FGameplayEffectContextHandle URPGAbilitySystemLibrary::ApplyDamageEffect(const FDamageEffectParams& DamageEffectParams)
{
	const FRPGGameplayTags& GameplayTags = FRPGGameplayTags::Get();
	const AActor* SourceAvatarActor = DamageEffectParams.SourceAbilitySystemComponent->GetAvatarActor();

	//创建GE的上下文句柄
	FGameplayEffectContextHandle EffectContextHandle = DamageEffectParams.SourceAbilitySystemComponent->MakeEffectContext();
	EffectContextHandle.AddSourceObject(SourceAvatarActor);

	//设置击退相关
	SetDeathImpulse(EffectContextHandle, DamageEffectParams.DeathImpulse);
	SetKnockbackForce(EffectContextHandle, DamageEffectParams.KnockbackForce);

	//设置范围伤害相关配置
	SetIsRadialDamage(EffectContextHandle, DamageEffectParams.bIsRadialDamage);
	SetRadialDamageInnerRadius(EffectContextHandle, DamageEffectParams.RadialDamageInnerRadius);
	SetRadialDamageOuterRadius(EffectContextHandle, DamageEffectParams.RadialDamageOuterRadius);
	SetRadialDamageOrigin(EffectContextHandle, DamageEffectParams.RadialDamageOrigin);

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

void URPGAbilitySystemLibrary::GetLivePlayersWithinRadius(const UObject* WorldContextObject, TArray<AActor*>& OutOverlappingActors, const TArray<AActor*>& ActorsToIgnore, float Radius, const FVector& SphereOrigin)
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

void URPGAbilitySystemLibrary::GetClosestTargets(int32 MaxTargets, const TArray<AActor*>& Actors, TArray<AActor*>& OutClosestTargets, const FVector& Origin)
{
	//如果数量过于少，直接返回原数组
	if(Actors.Num() <= MaxTargets)
	{
		OutClosestTargets = Actors;
		return;
	}

	TArray<AActor*> ActorsToCheck = Actors; //没有引用就是复制，复制一份用于遍历
	int32 NumTargetFound = 0; //当前已经遍历出最近距离的个数

	//循环遍历，直到获得足够数量的目标时停止
	while (NumTargetFound < MaxTargets)
	{
		if(ActorsToCheck.Num() == 0) break; //如果没有可遍历内容，将跳出循环
		double ClosestDistance = TNumericLimits<double>::Max(); //记录中心于目标的位置，如果有更小的将被替换，默认是最大
		AActor* ClosestActor; //缓存当前最近距离的目标
		for(AActor* PotentialTarget : ActorsToCheck)
		{
			//获取目标和中心的距离
			const double Distance = (PotentialTarget->GetActorLocation() - Origin).Length();

			//比对当前计算的位置是否小于缓存的位置
			if(Distance < ClosestDistance)
			{
				//如果小于，将替换对应信息
				ClosestDistance = Distance;
				ClosestActor = PotentialTarget;
			}
		}
		
		ActorsToCheck.Remove(ClosestActor); //从遍历数组中删除缓存的对象
		OutClosestTargets.AddUnique(ClosestActor); //添加到返回的数组中
		++ NumTargetFound; //递增数量
	}
}

bool URPGAbilitySystemLibrary::IsNotFriend(const AActor* FirstActor, const AActor* SecondActor)
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

TArray<FRotator> URPGAbilitySystemLibrary::EvenlySpacedRotators(const FVector& Forward, const FVector& Axis, float Spread, int32 NumRotators)
{
	TArray<FRotator> Rotators;
	
	const FVector LeftOfSpread = Forward.RotateAngleAxis(-Spread / 2.f, Axis); //获取到最左侧的角度
	
	if(NumRotators > 1)
	{
		const float DeltaSpread = Spread / NumRotators; //技能分的段数

		for(int32 i=0; i<NumRotators; i++)
		{
			const FVector Direction = LeftOfSpread.RotateAngleAxis(DeltaSpread * (i + 0.5f), Axis); //获取当前分段的角度
			Rotators.Add(Direction.Rotation());
		}
	}
	else
	{
		//如果只需要一个，则将朝向放入即可
		Rotators.Add(Forward.Rotation());
	}

	return Rotators;
}

TArray<FVector> URPGAbilitySystemLibrary::EvenlyRotatedVectors(const FVector& Forward, const FVector& Axis, float Spread, int32 NumVectors)
{
	TArray<FVector> Vectors;
	
	const FVector LeftOfSpread = Forward.RotateAngleAxis(-Spread / 2.f, Axis); //获取到最左侧的角度
	
	if(NumVectors > 1)
	{
		const float DeltaSpread = Spread / NumVectors; //技能分的段数

		for(int32 i=0; i<NumVectors; i++)
		{
			const FVector Direction = LeftOfSpread.RotateAngleAxis(DeltaSpread * (i + 0.5f), Axis); //获取当前分段的角度
			Vectors.Add(Direction);
		}
	}
	else
	{
		//如果只需要一个，则将朝向放入即可
		Vectors.Add(Forward);
	}

	return Vectors;
}

/** @RETURN 如果从 Origin 发出的武器射线击中了 VictimComp 组件，则返回 True。 OutHitResult 将包含击中的具体信息。 */
static bool ComponentIsDamageableFrom(UPrimitiveComponent* VictimComp, FVector const& Origin, AActor const* IgnoredActor, const TArray<AActor*>& IgnoreActors, ECollisionChannel TraceChannel, FHitResult& OutHitResult)
{
	// 配置碰撞查询参数，忽略指定的 Actor
	FCollisionQueryParams LineParams(SCENE_QUERY_STAT(ComponentIsVisibleFrom), true, IgnoredActor);
	LineParams.AddIgnoredActors( IgnoreActors );

	// 获取组件所在世界的指针
	UWorld* const World = VictimComp->GetWorld();
	check(World);

	// 使用组件的包围盒中心作为射线终点
	FVector const TraceEnd = VictimComp->Bounds.Origin;
	FVector TraceStart = Origin;
	// 如果起点和终点重合，微调起点以避免提前退出
	if (Origin == TraceEnd)
	{
		// 微调 Z 轴
		TraceStart.Z += 0.01f;
	}

	// 只有当通道合法时才执行射线检测
	if (TraceChannel != ECollisionChannel::ECC_MAX)
	{
		bool const bHadBlockingHit = World->LineTraceSingleByChannel(OutHitResult, TraceStart, TraceEnd, TraceChannel, LineParams);
		//::DrawDebugLine(World, TraceStart, TraceEnd, FLinearColor::Red, true);

		// 如果有阻挡物，检查是否为目标组件
		if (bHadBlockingHit)
		{
			if (OutHitResult.Component == VictimComp)
			{
				// 阻挡物是目标组件，返回 true
				return true;
			}
			else
			{
				// 击中其他阻挡物，记录日志并返回 false
				UE_LOG(LogDamage, Log, TEXT("Radial Damage to %s blocked by %s (%s)"), *GetNameSafe(VictimComp), *OutHitResult.GetHitObjectHandle().GetName(), *GetNameSafe(OutHitResult.Component.Get()));
				return false;
			}
		}
	}
	else
	{
		// 如果通道无效，输出警告
		UE_LOG(LogDamage, Warning, TEXT("ECollisionChannel::ECC_MAX is not valid! No falloff is added to damage"));
	}

	// 未击中任何物体，构造一个伪造的 HitResult 假设击中组件中心
	FVector const FakeHitLoc = VictimComp->GetComponentLocation();
	FVector const FakeHitNorm = (Origin - FakeHitLoc).GetSafeNormal();		// 法线指向伤害源
	OutHitResult = FHitResult(VictimComp->GetOwner(), VictimComp, FakeHitLoc, FakeHitNorm);
	return true;
}

float URPGAbilitySystemLibrary::ApplyRadialDamageWithFalloff(AActor* TargetActor, float BaseDamage, float MinimumDamage, const FVector& Origin, float DamageInnerRadius,
	float DamageOuterRadius, float DamageFalloff, AActor* DamageCauser, AController* InstigatedByController, ECollisionChannel DamagePreventionChannel)
{
	// 判断目标角色是否死亡
	bool bIsDead = true;
	if(TargetActor->Implements<UCombatInterface>())
	{
		bIsDead = ICombatInterface::Execute_IsDead(TargetActor);
	}
	if(bIsDead)
	{
		return 0.f; //如果角色已经死亡，直接返回0
	}

	// 获取目标角色所有组件
	TArray<UActorComponent*> Components;
	TargetActor->GetComponents(Components);

	bool bIsDamageable = false; //判断攻击是能能够查看到目标
	TArray<FHitResult> HitList; //存储目标收到碰撞查询到的碰撞结果
	for (UActorComponent* Comp : Components)
	{
		UPrimitiveComponent* PrimitiveComp = Cast<UPrimitiveComponent>(Comp);
		if (PrimitiveComp && PrimitiveComp->IsCollisionEnabled())
		{
			FHitResult Hit;
			bIsDamageable = ComponentIsDamageableFrom(
				PrimitiveComp, Origin, DamageCauser, {}, DamagePreventionChannel, Hit
			);
			HitList.Add(Hit);
			if(bIsDamageable) break;
		}
	}

	//应用目标的伤害值
	float AppliedDamage = 0.f;

	if (bIsDamageable)
	{
		// 创建伤害事件
		FRadialDamageEvent DmgEvent;
		DmgEvent.DamageTypeClass = TSubclassOf<UDamageType>(UDamageType::StaticClass());
		DmgEvent.Origin = Origin;
		DmgEvent.Params = FRadialDamageParams(BaseDamage, MinimumDamage, DamageInnerRadius, DamageOuterRadius, DamageFalloff);
		DmgEvent.ComponentHits = HitList;
		
		// 应用伤害
		AppliedDamage = TargetActor->TakeDamage(BaseDamage, DmgEvent, InstigatedByController, DamageCauser);
	}

	return AppliedDamage;
}



