// Copyright SumeruAI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class AActor;
class UBlueprint;
class UATFComponent;
class USkeletalMeshComponent;

struct FSumeruAIATFSetupResult
{
	bool bSuccess = false;
	FString Message;
};

class FSumeruAIATFSetup
{
public:
	static FSumeruAIATFSetupResult SetupSelectedActors();
	static FSumeruAIATFSetupResult SetupSelectedBlueprints();
	static FSumeruAIATFSetupResult SetupActor(AActor* Actor);
	static FSumeruAIATFSetupResult SetupBlueprint(UBlueprint* Blueprint);

private:
	static bool IsSkippableActor(const AActor* Actor);
	static USkeletalMeshComponent* ResolveFaceMesh(AActor* Actor);
	static void ApplyAtfDefaults(UATFComponent* Atf, USkeletalMeshComponent* FaceMesh);
	static void WireSampleActors(UWorld* World, USkeletalMeshComponent* FaceMesh, int32 FaceId);
	static bool AddInstanceComponent(AActor* Actor, UActorComponent* Component);
	static bool InsertFaceSyncIntoFaceAnimBP(USkeletalMeshComponent* FaceMesh, FString& OutMessage);
	static bool AssignSumeruFaceAnimBP(USkeletalMeshComponent* FaceMesh, FString& OutMessage);
};
