// 版权归暮志未晚所有。


#include "Input/InputConfig.h"

const UInputAction* UInputConfig::FindAbilityInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound) const
{
	for(const FInputActionStruct& Action : AbilityInputActions)
	{
		if(Action.InputAction && Action.InputTag == InputTag)
		{
			return Action.InputAction;
		}
	}

	if(bLogNotFound)
	{
		UE_LOG(LogTemp, Error, TEXT("无法从InputConfig[%s]中找到InputTag[%s]对应的技能InputAction"), *GetNameSafe(this), *InputTag.ToString());
	}

	return nullptr;
}
