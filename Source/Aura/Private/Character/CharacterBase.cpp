// 版权归暮志未晚所有。


#include "Character/CharacterBase.h"

// 设置默认值
ACharacterBase::ACharacterBase()
{
 	// 将这个字符设置为true时，将每帧进行更新。不需要可以关闭提高性能。
	PrimaryActorTick.bCanEverTick = false;

	Weapon = CreateDefaultSubobject<USkeletalMeshComponent>("Weapon"); //创建控件
	Weapon->SetupAttachment(GetMesh(), FName("WeaponHandSocket")); //添加到父节点
	Weapon->SetCollisionEnabled(ECollisionEnabled::NoCollision); //关闭骨骼网格体碰撞

}

// 在生成时或游戏开始时将被调用
void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
}


