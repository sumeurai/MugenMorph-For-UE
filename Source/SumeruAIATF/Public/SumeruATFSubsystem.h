// Copyright 2026 SumeruAI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SumeruATFSubsystem.generated.h"

class AAudioToFaceManager;

/**
 * World entry point for Sumeru ATF. Resolves (or spawns) the level's AAudioToFaceManager.
 * Prefer this over AAudioToFaceManager::Instance() in new code.
 */
UCLASS()
class SUMERUAIATF_API USumeruATFSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	static USumeruATFSubsystem* Get(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "ATF", meta = (WorldContext = "WorldContextObject", DisplayName = "Get ATF Subsystem"))
	static USumeruATFSubsystem* GetATFSubsystem(const UObject* WorldContextObject);

	/** Cached/looked-up manager for this world (may be null). Does not spawn. */
	UFUNCTION(BlueprintPure, Category = "ATF")
	AAudioToFaceManager* GetManager() const;

	/**
	 * Find an existing manager, or spawn AAudioToFaceManager once for this world.
	 * Prefer placing a Blueprint subclass in the level when you need BP event overrides.
	 */
	UFUNCTION(BlueprintCallable, Category = "ATF")
	AAudioToFaceManager* EnsureManager();

	/** Force refresh of the cached manager pointer. */
	void SetManager(AAudioToFaceManager* InManager);

	virtual void Deinitialize() override;

private:
	mutable TWeakObjectPtr<AAudioToFaceManager> CachedManager;
};
