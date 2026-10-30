// Copyright 2026 Mugen3D. All Rights Reserved.

#include "MugenMorphSubsystem.h"
#include "MugenMorph.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "MugenMorphLog.h"

UMugenMorphSubsystem* UMugenMorphSubsystem::GetMugenMorphSubsystem(const UObject* WorldContextObject)
{
	return Get(WorldContextObject);
}

UMugenMorphSubsystem* UMugenMorphSubsystem::Get(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}
	UWorld* World = WorldContextObject->GetWorld();
	return World ? World->GetSubsystem<UMugenMorphSubsystem>() : nullptr;
}

AMugenMorphManager* UMugenMorphSubsystem::GetManager() const
{
	if (AMugenMorphManager* Manager = CachedManager.Get())
	{
		return Manager;
	}

	if (UWorld* World = GetWorld())
	{
		for (TActorIterator<AMugenMorphManager> It(World); It; ++It)
		{
			CachedManager = *It;
			return *It;
		}
	}

	if (AMugenMorphManager* Fallback = AMugenMorphManager::Instance())
	{
		CachedManager = Fallback;
		return Fallback;
	}

	return nullptr;
}

AMugenMorphManager* UMugenMorphSubsystem::EnsureManager()
{
	if (AMugenMorphManager* Existing = GetManager())
	{
		return Existing;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogMugenMorph, Warning, TEXT("EnsureManager: world is null"));
		return nullptr;
	}

	// Allow spawn during PIE / game; skip pure editor preview worlds without play.
	const bool bCanSpawn =
		World->IsGameWorld()
		|| World->WorldType == EWorldType::PIE
		|| World->HasBegunPlay();
	if (!bCanSpawn)
	{
		UE_LOG(LogMugenMorph, Warning,
			TEXT("EnsureManager: world type=%d not ready to spawn manager"),
			static_cast<int32>(World->WorldType));
		return nullptr;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Params.ObjectFlags |= RF_Transient;

	AMugenMorphManager* Spawned = World->SpawnActor<AMugenMorphManager>(
		AMugenMorphManager::StaticClass(),
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		Params);

	if (!Spawned)
	{
		UE_LOG(LogMugenMorph, Error, TEXT("EnsureManager: SpawnActor<AMugenMorphManager> failed"));
		return nullptr;
	}

	CachedManager = Spawned;
	UE_LOG(LogMugenMorph, Log,
		TEXT("EnsureManager: spawned '%s' (optional: place MugenMorphManager_BP in level)"),
		*Spawned->GetName());
	return Spawned;
}

void UMugenMorphSubsystem::SetManager(AMugenMorphManager* InManager)
{
	CachedManager = InManager;
}

void UMugenMorphSubsystem::Deinitialize()
{
	CachedManager.Reset();
	Super::Deinitialize();
}
