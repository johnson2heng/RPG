// 版权归暮志未晚所有。


#include "Player/PlayerStateBase.h"

APlayerStateBase::APlayerStateBase()
{
	NetUpdateFrequency = 100.f; //每秒和服务器更新频率，使用GAS后可以设置的高一些
}
