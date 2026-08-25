// Copyright SumeruAI. All Rights Reserved.

#include "SumeruATFSubsystem.h"
#include "AudioToFace.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "SumeruATFLog.h"

USumeruATFSubsystem* USumeruATFSubsystem::GetATFSubsystem(const UObject* WorldContextObject)
{
	return Get(WorldContextObject);
}

USumeruATFSubsystem* USumeruATFSubsystem::Get(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}
	UWorld* World = WorldContextObject->GetWorld();
	return World ? World->GetSubsystem<USumeruATFSubsystem>() : nullptr;
}

AAudioToFaceManager* USumeruATFSubsystem::GetManager() const
{
	if (AAudioToFaceManager* Manager = CachedManager.Get())
	{
		return Manager;
	}

	if (UWorld* World = GetWorld())
	{
		for (TActorIterator<AAudioToFaceManager> It(World); It; ++It)
		{
			CachedManager = *It;
			return *It;
		}
	}

	if (AAudioToFaceManager* Fallback = AAudioToFaceManager::Instance())
	{
		CachedManager = Fallback;
		return Fallback;
	}

	return nullptr;
}

AAudioToFaceManager* USumeruATFSubsystem::EnsureManager()
{
	if (AAudioToFaceManager* Existing = GetManager())
	{
		return Existing;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogSumeruATF, Warning, TEXT("EnsureManager: world is null"));
		return nullptr;
	}

	// Allow spawn during PIE / game; skip pure editor preview worlds without play.
	const bool bCanSpawn =
		World->IsGameWorld()
		|| World->WorldType == EWorldType::PIE
		|| World->HasBegunPlay();
	if (!bCanSpawn)
	{
		UE_LOG(LogSumeruATF, Warning,
			TEXT("EnsureManager: world type=%d not ready to spawn manager"),
			static_cast<int32>(World->WorldType));
		return nullptr;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Params.ObjectFlags |= RF_Transient;

	AAudioToFaceManager* Spawned = World->SpawnActor<AAudioToFaceManager>(
		AAudioToFaceManager::StaticClass(),
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		Params);

	if (!Spawned)
	{
		UE_LOG(LogSumeruATF, Error, TEXT("EnsureManager: SpawnActor<AAudioToFaceManager> failed"));
		return nullptr;
	}

	CachedManager = Spawned;
	UE_LOG(LogSumeruATF, Log,
		TEXT("EnsureManager: spawned '%s' (optional: place AudioToFaceManager_BP in level)"),
		*Spawned->GetName());
	return Spawned;
}

void USumeruATFSubsystem::SetManager(AAudioToFaceManager* InManager)
{
	CachedManager = InManager;
}

void USumeruATFSubsystem::Deinitialize()
{
	CachedManager.Reset();
	Super::Deinitialize();
}
