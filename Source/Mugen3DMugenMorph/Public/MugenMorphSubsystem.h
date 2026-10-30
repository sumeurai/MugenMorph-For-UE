// Copyright 2026 Mugen3D. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MugenMorphSubsystem.generated.h"

class AMugenMorphManager;

/**
 * World entry point for Mugen3D MugenMorph. Resolves (or spawns) the level's AMugenMorphManager.
 * Prefer this over AMugenMorphManager::Instance() in new code.
 */
UCLASS()
class MUGEN3DMUGENMORPH_API UMugenMorphSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	static UMugenMorphSubsystem* Get(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "MugenMorph", meta = (WorldContext = "WorldContextObject", DisplayName = "Get MugenMorph Subsystem"))
	static UMugenMorphSubsystem* GetMugenMorphSubsystem(const UObject* WorldContextObject);

	/** Cached/looked-up manager for this world (may be null). Does not spawn. */
	UFUNCTION(BlueprintPure, Category = "MugenMorph")
	AMugenMorphManager* GetManager() const;

	/**
	 * Find an existing manager, or spawn AMugenMorphManager once for this world.
	 * Prefer placing a Blueprint subclass in the level when you need BP event overrides.
	 */
	UFUNCTION(BlueprintCallable, Category = "MugenMorph")
	AMugenMorphManager* EnsureManager();

	/** Force refresh of the cached manager pointer. */
	void SetManager(AMugenMorphManager* InManager);

	virtual void Deinitialize() override;

private:
	mutable TWeakObjectPtr<AMugenMorphManager> CachedManager;
};
