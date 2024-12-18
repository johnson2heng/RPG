// 版权归暮志未晚所有。

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "RPGPlayerController.generated.h"

class IHighLightInterface;
class AMagicCircle;
class UNiagaraSystem;
class UDamageTextComponent;
class USplineComponent;
class UInputConfig;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class IEnemyInterface;
class URPGAbilitySystemComponent;

//鼠标拾取目标的状态枚举
enum class ETargetingStatus : uint8
{
	//敌人
	TargetingEnemy,
	//鼠标拾取的目标非敌人
	TargetingNonEnemy,
	//无
	NotTargeting
};

/**
 * 玩家控制器
 */
UCLASS()
class RPG_API ARPGPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ARPGPlayerController();
	virtual void PlayerTick(float DeltaTime) override;

	UFUNCTION(Client, Reliable)
	void ShowDamageNumber(float DamageAmount, ACharacter* TargetCharacter, bool IsBlockedHit, bool IsCriticalHit); //在每个客户端显示伤害数值

	//显示魔法光圈 并设置光圈贴花材质
	UFUNCTION(BlueprintCallable)
	void ShowMagicCircle(UMaterialInterface* DecalMaterial);

	//隐藏魔法光圈
	UFUNCTION(BlueprintCallable)
	void HideMagicCircle() const; 
protected:
	virtual void BeginPlay() override; //游戏开始时触发
	virtual void SetupInputComponent() override; //在生成输入组件时触发

private:
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputMappingContext> CharacterContext;

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> ShiftAction;

	void ShiftPressed() { bShiftKeyDown = true; };
	void ShiftReleased() { bShiftKeyDown = false; };
	bool bShiftKeyDown = false;

	//监听键盘事件回调，用于修改玩家角色移动
	void Move(const FInputActionValue& InputActionValue);

	//鼠标位置追踪拾取
	void CursorTrace(); 
	TObjectPtr<AActor> LastActor; //上一帧拾取到的接口指针
	TObjectPtr<AActor> ThisActor; //这一帧拾取到的接口指针
	FHitResult CursorHit; //鼠标拾取结果，可以复用

	//鼠标按下事件回调
	void AbilityInputTagPressed(FGameplayTag InputTag);
	//鼠标抬起事件回调
	void AbilityInputTagReleased(FGameplayTag InputTag);
	//鼠标长按事件回调
	void AbilityInputTagHold(FGameplayTag InputTag);

	//玩家输入映射上下文配置
	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputConfig> InputConfig;

	UPROPERTY()
	TObjectPtr<URPGAbilitySystemComponent> AbilitySystemComponentBase;

	//获取ASC
	URPGAbilitySystemComponent* GetASC();

	FVector CachedDestination = FVector::ZeroVector; //存储鼠标点击的位置
	float FollowTime = 0.f; // 用于查看按住了多久
	bool bAutoRunning = false; //当前是否自动移动
	// bool bTargeting = false; //当前鼠标是否选中敌人
	ETargetingStatus TargetingStatus = ETargetingStatus::NotTargeting; //鼠标拾取

	//定义鼠标悬停多长时间内算点击事件
	UPROPERTY(EditDefaultsOnly)
	float ShortPressThreshold = 0.3f; 

	//当角色和目标距离在此半径内时，将关闭自动寻路
	UPROPERTY(EditDefaultsOnly)
	float AutoRunAcceptanceRadius = 50.f; 

	//自动寻路时生成的样条线
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USplineComponent> Spline; 

	//鼠标点击移动播放的特效
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UNiagaraSystem> ClickNiagaraSystem; 

	void AutoRun();

	//创建显示伤害数值脚本的类
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UDamageTextComponent> DamageTextComponentClass; 

	//创建奥数光圈使用的类
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AMagicCircle> MagicCircleClass;

	//存储魔法光圈的属性，不需要暴露给蓝图
	UPROPERTY()
	TObjectPtr<AMagicCircle> MagicCircle;

	void UpdateMagicCircleLocation() const; //每一帧调用，更新魔法光圈的位置
};
