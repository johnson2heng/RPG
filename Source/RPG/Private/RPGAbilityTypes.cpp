#include "RPGAbilityTypes.h"

bool FRPGGameplayEffectContext::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	uint32 RepBits = 0;
	
	if (Ar.IsSaving())
	{
		if (bReplicateInstigator && Instigator.IsValid())
		{
			RepBits |= 1 << 0;
		}
		if (bReplicateEffectCauser && EffectCauser.IsValid() )
		{
			RepBits |= 1 << 1;
		}
		if (AbilityCDO.IsValid())
		{
			RepBits |= 1 << 2;
		}
		if (bReplicateSourceObject && SourceObject.IsValid())
		{
			RepBits |= 1 << 3;
		}
		if (Actors.Num() > 0)
		{
			RepBits |= 1 << 4;
		}
		if (HitResult.IsValid())
		{
			RepBits |= 1 << 5;
		}
		if (bHasWorldOrigin)
		{
			RepBits |= 1 << 6;
		}
		//自定义内容，增加暴击和格挡触发存储
		if(bIsBlockedHit)
		{
			RepBits |= 1 << 7;
		}
		if(bIsCriticalHit)
		{
			RepBits |= 1 << 8;
		}
		if(bIsSuccessfulDeBuff)
		{
			RepBits |= 1 << 9;
		}
		if(DeBuffDamage > 0.f)
		{
			RepBits |= 1 << 10;
		}
		if(DeBuffDuration > 0.f)
		{
			RepBits |= 1 << 11;
		}
		if(DeBuffFrequency > 0.f)
		{
			RepBits |= 1 << 12;
		}
		if(DamageType.IsValid())
		{
			RepBits |= 1 << 13;
		}
		if(!DeathImpulse.IsZero())
		{
			RepBits |= 1 << 14;
		}
	}

	//使用了多少长度，就将长度设置为多少
	Ar.SerializeBits(&RepBits, 15);

	if (RepBits & (1 << 0))
	{
		Ar << Instigator;
	}
	if (RepBits & (1 << 1))
	{
		Ar << EffectCauser;
	}
	if (RepBits & (1 << 2))
	{
		Ar << AbilityCDO;
	}
	if (RepBits & (1 << 3))
	{
		Ar << SourceObject;
	}
	if (RepBits & (1 << 4))
	{
		SafeNetSerializeTArray_Default<31>(Ar, Actors);
	}
	if (RepBits & (1 << 5))
	{
		if (Ar.IsLoading())
		{
			if (!HitResult.IsValid())
			{
				HitResult = TSharedPtr<FHitResult>(new FHitResult());
			}
		}
		HitResult->NetSerialize(Ar, Map, bOutSuccess);
	}
	if (RepBits & (1 << 6))
	{
		Ar << WorldOrigin;
		bHasWorldOrigin = true;
	}
	else
	{
		bHasWorldOrigin = false;
	}
	//新增对暴击格挡的序列化或反序列化处理
	if (RepBits & (1 << 7))
	{
		Ar << bIsBlockedHit;
	}
	if (RepBits & (1 << 8))
	{
		Ar << bIsCriticalHit;
	}
	if (RepBits & (1 << 9))
	{
		Ar << bIsSuccessfulDeBuff;
	}
	if (RepBits & (1 << 10))
	{
		Ar << DeBuffDamage;
	}
	if (RepBits & (1 << 11))
	{
		Ar << DeBuffDuration;
	}
	if (RepBits & (1 << 12))
	{
		Ar << DeBuffFrequency;
	}
	if (RepBits & (1 << 13))
	{
		if (Ar.IsLoading()) //判断是否在加载资源
		{
			if (!DamageType.IsValid())
			{
				DamageType = TSharedPtr<FGameplayTag>(new FGameplayTag());
			}
		}
		DamageType->NetSerialize(Ar, Map, bOutSuccess);
	}
	if (RepBits & (1 << 14))
	{
		DeathImpulse.NetSerialize(Ar, Map, bOutSuccess);
	}

	if (Ar.IsLoading())
	{
		AddInstigator(Instigator.Get(), EffectCauser.Get()); // Just to initialize InstigatorAbilitySystemComponent
	}	

	bOutSuccess = true;
	return true;
}
