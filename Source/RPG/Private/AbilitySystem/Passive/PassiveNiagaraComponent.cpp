// 版权归暮志未晚所有。


#include "AbilitySystem/Passive/PassiveNiagaraComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "RPGGameplayTags.h"
#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "Interaction/CombatInterface.h"

UPassiveNiagaraComponent::UPassiveNiagaraComponent()
{
	bAutoActivate = false;
}

void UPassiveNiagaraComponent::BeginPlay()
{
	Super::BeginPlay();

	if(URPGAbilitySystemComponent* RPGASC = Cast<URPGAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner())))
	{
		RPGASC->ActivatePassiveEffect.AddUObject(this, &UPassiveNiagaraComponent::OnPassiveActivate);
		//在初始化时，读取存档已经结束，技能也设置完成，所以我们需要读取存档并更新显示
		if(RPGASC->GetStatusTagFromAbilityTag(PassiveSpellTag) == FRPGGameplayTags::Get().Abilities_Status_Equipped)
		{
			OnPassiveActivate(PassiveSpellTag, true);
		}
	}
	else if(ICombatInterface* CombatInterface = Cast<ICombatInterface>(GetOwner()))
	{
		//AddWeakLambda 这种绑定方式的主要好处是，当绑定的对象被销毁时，委托不会保持对象的引用，从而避免悬空指针问题和内存泄漏。
		CombatInterface->GetOnASCRegisteredDelegate().AddWeakLambda(this,[this](UAbilitySystemComponent* InASC)
		{
			if(URPGAbilitySystemComponent* RPGASC = Cast<URPGAbilitySystemComponent>(InASC))
			{
				RPGASC->ActivatePassiveEffect.AddUObject(this, &UPassiveNiagaraComponent::OnPassiveActivate);
				//在初始化时，读取存档已经结束，技能也设置完成，所以我们需要读取存档并更新显示
				if(RPGASC->GetStatusTagFromAbilityTag(PassiveSpellTag) == FRPGGameplayTags::Get().Abilities_Status_Equipped)
				{
					OnPassiveActivate(PassiveSpellTag, true);
				}
			}
		});
	}
}

void UPassiveNiagaraComponent::OnPassiveActivate(const FGameplayTag& AbilityTag, bool bActivate)
{
	//判断技能标签是否一致
	if(AbilityTag.MatchesTagExact(PassiveSpellTag))
	{
		//判断是否需要激活
		if(bActivate)
		{
			//不需要重复激活
			if(!IsActive()) Activate();
		}
		else
		{
			Deactivate();
		}
	}
}
