// Copyright 2026 Mugen3D. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class AActor;
class UBlueprint;
class UMugenMorphComponent;
class USkeletalMeshComponent;

struct FMugen3DMugenMorphSetupResult
{
	bool bSuccess = false;
	FString Message;
};

class FMugen3DMugenMorphSetup
{
public:
	static FMugen3DMugenMorphSetupResult SetupSelectedActors();
	static FMugen3DMugenMorphSetupResult SetupSelectedBlueprints();
	static FMugen3DMugenMorphSetupResult SetupActor(AActor* Actor);
	static FMugen3DMugenMorphSetupResult SetupBlueprint(UBlueprint* Blueprint);

private:
	static bool IsSkippableActor(const AActor* Actor);
	static USkeletalMeshComponent* ResolveFaceMesh(AActor* Actor);
	static void ApplyAtfDefaults(UMugenMorphComponent* Atf, USkeletalMeshComponent* FaceMesh);
	static void WireSampleActors(UWorld* World, USkeletalMeshComponent* FaceMesh, int32 FaceId);
	static bool AddInstanceComponent(AActor* Actor, UActorComponent* Component);
	static bool InsertFaceSyncIntoFaceAnimBP(USkeletalMeshComponent* FaceMesh, FString& OutMessage);
	static bool AssignSumeruFaceAnimBP(USkeletalMeshComponent* FaceMesh, FString& OutMessage);
};
