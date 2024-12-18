// 版权归暮志未晚所有。


#include "Actor/RPGEnemySpawnPoint.h"

#include "Character/RPGEnemy.h"

void ARPGEnemySpawnPoint::SpawnEnemy()
{
	//延迟生成Actor，并设置其尝试调整位置但固定生成
	ARPGEnemy* Enemy = GetWorld()->SpawnActorDeferred<ARPGEnemy>(
		EnemyClass,
		GetActorTransform(),
		nullptr,
		nullptr,
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
	
	Enemy->SetLevel(EnemyLevel);
	Enemy->SetCharacterClass(CharacterClass);
	Enemy->FinishSpawning(GetActorTransform());
	Enemy->SpawnDefaultController();
}
