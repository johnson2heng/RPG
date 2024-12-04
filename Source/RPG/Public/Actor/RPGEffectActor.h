// 版权归暮志未晚所有。

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"
#include "RPGEffectActor.generated.h"

struct FActiveGameplayEffectHandle;
class UAbilitySystemComponent;
class UGameplayEffect;

//效果应用状态枚举
UENUM(BlueprintType) 
enum class EEffectApplicationPolicy
{
	//在于此Actor碰撞体重叠时应用对应GE
	ApplyOnOverlap,
	//在于此Actor结束重叠时应用GE
	ApplyOnEndOverlap,
	//不应用此类型的GE
	DoNotApply
};

//效果移除的状态枚举
UENUM(BlueprintType) 
enum class EEffectRemovalPolicy
{
	//将在于此Actor结束重叠时移动GE
	RemoveOnEndOverlap,
	//不移出GE
	DoNotRemove
};

/**
 * 在场景中可放置的影响角色属性的物件基类
 */
UCLASS()
class RPG_API ARPGEffectActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ARPGEffectActor();

protected:
	// 游戏开始或生成对象时回调
	virtual void BeginPlay() override;

	//给与目标添加GameplayEffect效果
	UFUNCTION(BlueprintCallable) 
	void ApplyEffectToTarget(AActor* TargetActor, TSubclassOf<UGameplayEffect> GameplayEffectClass);

	//在重叠开始时处理效果的添加删除逻辑
	UFUNCTION(BlueprintCallable) 
	void OnOverlap(AActor* TargetActor);

	//在重叠结束时处理效果的添加删除逻辑
	UFUNCTION(BlueprintCallable) 
	void OnEndOverlap(AActor* TargetActor);

	//Instant和Duration的GE在应用后，此物体是否需要被销毁
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Apply Effects")
	bool bDestroyOnEffectApplication = true;

	//敌人是否能够拾取此物体
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Apply Effects")
	bool bApplyEffectsToEnemies = false;

	//生成瞬时完成的GameplayEffect的类
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Apply Effects")
	TSubclassOf<UGameplayEffect> InstantGameplayEffectClass; 

	//瞬时GE应用的阶段，DoNotApply为不应用
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Apply Effects")
	EEffectApplicationPolicy InstantEffectApplicationPolicy = EEffectApplicationPolicy::DoNotApply;

	//生成具有一定持续时间的GameplayEffect的类
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Apply Effects")
	TSubclassOf<UGameplayEffect> DurationGameplayEffectClass; 

	//持续时长GE的应用阶段，DoNotApply为不应用
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Apply Effects")
	EEffectApplicationPolicy DurationEffectApplicationPolicy = EEffectApplicationPolicy::DoNotApply;

	//生成无限时间的GameplayEffect的类
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Apply Effects")
	TSubclassOf<UGameplayEffect> InfinityGameplayEffectClass; 

	//无限时间GE的应用阶段，DoNotApply为不应用
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Apply Effects")
	EEffectApplicationPolicy InfinityEffectApplicationPolicy = EEffectApplicationPolicy::DoNotApply;

	//无限时间GE的删除阶段，RemoveOnEndOverlap将在不重叠时，清除角色身上的GE，DoNotRemove将不会删除
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Apply Effects")
	EEffectRemovalPolicy InfinityEffectRemovalPolicy = EEffectRemovalPolicy::RemoveOnEndOverlap;

	//用于存储当前已经激活的GameplayEffect的句柄的map
	TMap<FActiveGameplayEffectHandle, UAbilitySystemComponent*> ActiveEffectHandles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Apply Effects")
	float ActorLevel = 1.f;
};
