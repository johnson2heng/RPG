// 版权归暮志未晚所有。

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PlayerControllerBase.generated.h"

class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class IEnemyInterface;

/**
 * 玩家控制器
 */
UCLASS()
class AURA_API APlayerControllerBase : public APlayerController
{
	GENERATED_BODY()

public:
	APlayerControllerBase();
	virtual void PlayerTick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override; //游戏开始时触发
	virtual void SetupInputComponent() override; //在生成输入组件时触发

private:
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputMappingContext> CharacterContext;

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> MoveAction;

	void Move(const struct FInputActionValue& InputActionValue);

	void CursorTrace(); //鼠标位置追踪拾取
	TObjectPtr<IEnemyInterface> LastActor; //上一帧拾取到的接口指针
	TObjectPtr<IEnemyInterface> ThisActor; //这一帧拾取到的接口指针
};
