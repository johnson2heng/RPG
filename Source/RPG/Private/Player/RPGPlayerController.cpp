// 版权归暮志未晚所有。


#include "Player/RPGPlayerController.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "EnhancedInputSubsystems.h"
#include "RPGGameplayTags.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "Actor/MagicCircle.h"
#include "Components/DecalComponent.h"
#include "Components/SplineComponent.h"
#include "GameFramework/Character.h"
#include "Input/RPGInputComponent.h"
#include "Interaction/EnemyInterface.h"
#include "Interaction/HighLightInterface.h"
#include "RPG/RPG.h"
#include "UI/Widget/DamageTextComponent.h"

ARPGPlayerController::ARPGPlayerController()
{
	bReplicates = true; //是否将数据传送服务器更新
	LastActor = nullptr;
	ThisActor = nullptr;

	Spline = CreateDefaultSubobject<USplineComponent>("Spline");
}

void ARPGPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
	//鼠标位置追踪是否悬停在敌人身上
	CursorTrace();
	//自动寻路
	AutoRun();
	//更新魔法光圈的位置
	UpdateMagicCircleLocation();
}

void ARPGPlayerController::ShowMagicCircle(UMaterialInterface* DecalMaterial)
{
	if(!IsValid(MagicCircle)) MagicCircle = GetWorld()->SpawnActor<AMagicCircle>(MagicCircleClass);

	//设置贴花材质
	if(DecalMaterial != nullptr)
	{
		MagicCircle->MagicCircleDecal->SetMaterial(0, DecalMaterial);
	}
}

void ARPGPlayerController::HideMagicCircle() const
{
	if(IsValid(MagicCircle)) MagicCircle->Destroy();
}

void ARPGPlayerController::ShowDamageNumber_Implementation(float DamageAmount, ACharacter* TargetCharacter, bool IsBlockedHit, bool IsCriticalHit)
{
	//确保传入的目标没被销毁并且设置了组件类
	if(IsValid(TargetCharacter) && DamageTextComponentClass)
	{
		UDamageTextComponent* DamageText = NewObject<UDamageTextComponent>(TargetCharacter, DamageTextComponentClass);
		DamageText->RegisterComponent(); //动态创建的组件需要调用注册
		DamageText->AttachToComponent(TargetCharacter->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform); //先附加到角色身上，使用角色位置
		DamageText->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform); //然后从角色身上分离，保证在一个位置播放完成动画
		DamageText->SetDamageText(DamageAmount, IsBlockedHit, IsCriticalHit); //设置显示的伤害数字
	}
}

void ARPGPlayerController::AutoRun()
{
	if(!bAutoRunning) return;
	if(APawn* ControlledPawn = GetPawn())
	{
		//找到距离样条最近的位置
		const FVector LocationOnSpline = Spline->FindLocationClosestToWorldLocation(ControlledPawn->GetActorLocation(), ESplineCoordinateSpace::World);
		//获取这个位置在样条上的方向
		const FVector Direction = Spline->FindDirectionClosestToWorldLocation(LocationOnSpline, ESplineCoordinateSpace::World);
		ControlledPawn->AddMovementInput(Direction);

		const float DistanceToDestination = (LocationOnSpline - CachedDestination).Length();
		if(DistanceToDestination <= AutoRunAcceptanceRadius)
		{
			bAutoRunning = false;
		}
	}
}

void ARPGPlayerController::UpdateMagicCircleLocation() const
{
	if(IsValid(MagicCircle))
	{
		MagicCircle->SetActorLocation(CursorHit.ImpactPoint);
	}
}

void ARPGPlayerController::CursorTrace()
{
	//判断当前事件是否被阻挡，如果事件被阻挡，则清除相关内容
	if(GetASC() && GetASC()->HasMatchingGameplayTag(FRPGGameplayTags::Get().Player_Block_CursorTrace))
	{
		if(IsValid(ThisActor)) IHighLightInterface::Execute_UnHighlightActor(ThisActor);
		if(IsValid(LastActor)) IHighLightInterface::Execute_UnHighlightActor(LastActor);
		ThisActor = nullptr; 
		LastActor = nullptr;
		return;
	}

	//如果当前处于魔法范围指示阶段，将忽略掉场景中的角色
	const ECollisionChannel TraceChannel = IsValid(MagicCircle) ? ECC_EXCLUDEPLAYERS_CHANNEL : ECC_Visibility;
	
	GetHitResultUnderCursor(TraceChannel, false, CursorHit); //获取可视的鼠标命中结果
	if(!CursorHit.bBlockingHit) return; //如果未命中直接返回

	LastActor = ThisActor;
	//获取拾取的Actor，判断Actor是否继承高亮接口
	if(IsValid(CursorHit.GetActor()) && CursorHit.GetActor()->Implements<UHighLightInterface>())
	{
		ThisActor = CursorHit.GetActor();
	}
	else
	{
		ThisActor = nullptr;
	}

	//如果两次拾取的目标不同，将修改高亮目标
	if(ThisActor != LastActor)
	{
		if(IsValid(ThisActor)) IHighLightInterface::Execute_HighlightActor(ThisActor);
		if(IsValid(LastActor)) IHighLightInterface::Execute_UnHighlightActor(LastActor);
	}
	
}

void ARPGPlayerController::AbilityInputTagPressed(const FGameplayTag InputTag)
{
	//处理判断按下事件是否被阻挡
	if(GetASC() && GetASC()->HasMatchingGameplayTag(FRPGGameplayTags::Get().Player_Block_InputPressed))
	{
		return;
	}
	//判断鼠标左键，并处理移动相关
	if(InputTag.MatchesTagExact(FRPGGameplayTags::Get().InputTag_LMB))
	{
		//ThisActor为鼠标悬停在敌人身上才会有值
		if(IsValid(ThisActor))
		{
			if(ThisActor->Implements<UEnemyInterface>())
			{
				//继承敌人接口，目标为敌人
				TargetingStatus = ETargetingStatus::TargetingEnemy;
			}
			else
			{
				//无敌人接口，基本为场景静态物体
				TargetingStatus = ETargetingStatus::TargetingNonEnemy;
			}
		}
		else
		{
			//目标不存在，设置为无目标状态
			TargetingStatus = ETargetingStatus::NotTargeting;
		}

		bAutoRunning = false;
		FollowTime = 0.f; //重置统计的时间
	}
	//调用ASC内创建的键位按下事件
	if(GetASC()) GetASC()->AbilityInputTagPressed(InputTag);
}

void ARPGPlayerController::AbilityInputTagReleased(const FGameplayTag InputTag)
{
	//处理判断抬起事件是否被阻挡
	if(GetASC() && GetASC()->HasMatchingGameplayTag(FRPGGameplayTags::Get().Player_Block_InputReleased))
	{
		return;
	}

	//触发ASC相关事件触发
	if(GetASC()) GetASC()->AbilityInputTagReleased(InputTag);

	//如果不是鼠标左键，将不会执行下面移动相关的逻辑
	if(!InputTag.MatchesTagExact(FRPGGameplayTags::Get().InputTag_LMB)) return;

	//如果鼠标拾取状态为非敌人，并且没有按住Shift键，将进入寻路逻辑
	if(TargetingStatus != ETargetingStatus::TargetingEnemy && !bShiftKeyDown)
	{
		const APawn* ControlledPawn = GetPawn();
		//如果鼠标按下+抬起的时间小于设定时间，判定为点击事件
		if(FollowTime <= ShortPressThreshold && ControlledPawn)
		{
			//根据接口修改移动的目标位置
			if(IsValid(ThisActor))
			{
				IHighLightInterface::Execute_SetMoveToLocation(ThisActor, CachedDestination);
			}
			else
			{
				//触发点击时，播放点击特效
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ClickNiagaraSystem, CachedDestination);
			}
			
			//通过自动寻路组件，生成路线
			if(UNavigationPath* NavPath = UNavigationSystemV1::FindPathToLocationSynchronously(this, ControlledPawn->GetActorLocation(), CachedDestination))
			{
				Spline->ClearSplinePoints(); //清除样条内现有的点
				for(const FVector& PointLoc : NavPath->PathPoints)
				{
					Spline->AddSplinePoint(PointLoc, ESplineCoordinateSpace::World); //将新的位置添加到样条曲线中
					// DrawDebugSphere(GetWorld(), PointLoc, 8.f, 8, FColor::Orange, false, 5.f); //点击后debug调试
				}
				//保证路径点数组内不为0
				if(NavPath->PathPoints.Num() > 0)
				{
					//自动寻路将最终目的地设置为导航的终点，方便停止导航
					CachedDestination = NavPath->PathPoints[NavPath->PathPoints.Num() - 1];
					bAutoRunning = true; //设置当前正常自动寻路状态，将在tick中更新位置
				}
			}

		}
	}
}

void ARPGPlayerController::AbilityInputTagHold(const FGameplayTag InputTag)
{
	//通过标签阻止悬停事件的触发
	if(GetASC() && GetASC()->HasMatchingGameplayTag(FRPGGameplayTags::Get().Player_Block_InputHold))
	{
		return;
	}

	//非鼠标左键直接触发ASC相关事件
	if(!InputTag.MatchesTagExact(FRPGGameplayTags::Get().InputTag_LMB))
	{
		if(GetASC()) GetASC()->AbilityInputTagHold(InputTag);
		return;
	}

	//在发现目标或按住Shift点击左键时，都进行技能释放
	if(TargetingStatus == ETargetingStatus::TargetingEnemy || bShiftKeyDown)
	{
		if(GetASC()) GetASC()->AbilityInputTagHold(InputTag);
	}
	else
	{
		FollowTime += GetWorld()->GetDeltaSeconds(); //统计悬停时间来判断是否为点击

		if(CursorHit.bBlockingHit) CachedDestination = CursorHit.ImpactPoint; //获取鼠标拾取位置

		if(APawn* ControlledPawn = GetPawn())
		{
			const FVector WorldDirection = (CachedDestination - ControlledPawn->GetActorLocation()).GetSafeNormal();
			ControlledPawn->AddMovementInput(WorldDirection);
		}
	}
}

URPGAbilitySystemComponent* ARPGPlayerController::GetASC()
{
	if(AbilitySystemComponentBase == nullptr)
	{
		AbilitySystemComponentBase = Cast<URPGAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn()));
	}

	return AbilitySystemComponentBase;
}

void ARPGPlayerController::BeginPlay()
{
	Super::BeginPlay();
	check(CharacterContext); //判断是否存在

	//从本地角色身上获取到它的子系统
	// check(Subsystem); //检查子系统是否存在 不存在会打断
	if(UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		//将自定义的操作映射上下文添加到子系统中
		Subsystem->AddMappingContext(CharacterContext, 0); //可以存在多个操作映射，根据优先级触发
	}

	bShowMouseCursor = true; //游戏中是否显示鼠标光标
	DefaultMouseCursor = EMouseCursor::Default; //鼠标光标的样式

	FInputModeGameAndUI InputModeData;
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock); //将鼠标锁定在视口内
	InputModeData.SetHideCursorDuringCapture(false); //鼠标被捕获时是否隐藏
	SetInputMode(InputModeData); //设置给控制器
}

void ARPGPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	URPGInputComponent* EnhancedInputComponent = CastChecked<URPGInputComponent>(InputComponent); //获取到增强输入组件

	//绑定移动操作回调
	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ARPGPlayerController::Move); //绑定移动事件

	//绑定Shift按键事件
	EnhancedInputComponent->BindAction(ShiftAction, ETriggerEvent::Started, this, &ThisClass::ShiftPressed);
	EnhancedInputComponent->BindAction(ShiftAction, ETriggerEvent::Completed, this, &ThisClass::ShiftReleased);

	//通过自定义输入组件绑定鼠标按下抬起悬停事件，数据配置在PlayerController里面，组件内绑定对应回调函数触发
	EnhancedInputComponent->BindAbilityAction(InputConfig, this, &ThisClass::AbilityInputTagPressed, &ThisClass::AbilityInputTagReleased, &ThisClass::AbilityInputTagHold);
}

void ARPGPlayerController::Move(const FInputActionValue& InputActionValue)
{
	//方向控制，如果阻止了按住事件，将不再执行
	if(GetASC() && GetASC()->HasMatchingGameplayTag(FRPGGameplayTags::Get().Player_Block_InputHold))
	{
		return;
	}
	
	const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>(); //获取输入操作的2维向量值
	const FRotator Rotation = GetControlRotation(); //获取控制器旋转
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f); //通过控制器的垂直朝向创建一个旋转值

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X); //获取世界坐标系下向前的值，-1到1
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y); //获取世界坐标系下向右的值，-1到1

	if(APawn* ControlledPawn = GetPawn<APawn>())
	{
		ControlledPawn->AddMovementInput(ForwardDirection, InputAxisVector.Y);
		ControlledPawn->AddMovementInput(RightDirection, InputAxisVector.X);
	}
}
