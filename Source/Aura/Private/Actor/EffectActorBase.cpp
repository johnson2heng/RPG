// 版权归暮志未晚所有。


#include "Actor/EffectActorBase.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystem/AttributeSetBase.h"
#include "Components/SphereComponent.h"

AEffectActorBase::AEffectActorBase()
{
 	// 设置当前对象是否每帧调用Tick()
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>("Mesh");
	SetRootComponent(Mesh);

	Sphere = CreateDefaultSubobject<USphereComponent>("Sphere");
	Sphere->SetupAttachment(GetRootComponent());
}

void AEffectActorBase::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//TODO: 为了测试数值修改功能，启用了常量转变量功能。
	if(IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(OtherActor))
	{
		 //根据类从ASC里面获取到对应的AS实例
		const UAttributeSetBase* AttributeSet = Cast<UAttributeSetBase>(ASCInterface->GetAbilitySystemComponent()->GetAttributeSet(UAttributeSetBase::StaticClass()));
		UAttributeSetBase* MutableAttributeSet = const_cast<UAttributeSetBase*>(AttributeSet); //将常量转为变量
		MutableAttributeSet->SetHealth(AttributeSet->GetHealth() + 25.f);
		Destroy(); // 销毁自身
	}
}

void AEffectActorBase::EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
}

void AEffectActorBase::BeginPlay()
{
	Super::BeginPlay();

	Sphere->OnComponentBeginOverlap.AddDynamic(this, &AEffectActorBase::OnOverlap);
	Sphere->OnComponentEndOverlap.AddDynamic(this, &AEffectActorBase::EndOverlap);
}


