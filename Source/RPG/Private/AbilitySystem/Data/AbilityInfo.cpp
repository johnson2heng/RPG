// 版权归暮志未晚所有。


#include "AbilitySystem/Data/AbilityInfo.h"

#include "RPG/RPGLogChannels.h"

FRPGAbilityInfo UAbilityInfo::FindAbilityInfoForTag(const FGameplayTag& AbilityTag, const bool bLogNotFound) const
{
	for(const FRPGAbilityInfo& Info : AbilityInformation)
	{
		if(Info.AbilityTag == AbilityTag)
		{
			return Info;
		}
	}

	if(bLogNotFound)
	{
		//如果获取不到数据，打印消息
		UE_LOG(LogRPG, Error, TEXT("无法通过技能标签[%s]在技能数据[%s]查找到对应的技能数据"), *AbilityTag.ToString(), *GetNameSafe(this));
	}

	return FRPGAbilityInfo();
}
