// 版权归暮志未晚所有。


#include "UI/MVVM/MVVM_LoadSlot.h"

void UMVVM_LoadSlot::InitializeSlot() const
{
	SetWidgetSwitcherIndex.Broadcast(1);
}

void UMVVM_LoadSlot::SetSlotName(const FString& InSlotName)
{
	UE_MVVM_SET_PROPERTY_VALUE(SlotName, InSlotName);
}
