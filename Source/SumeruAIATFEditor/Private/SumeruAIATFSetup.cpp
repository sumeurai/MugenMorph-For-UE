// Copyright 2026 SumeruAI. All Rights Reserved.

#include "SumeruAIATFSetup.h"

#include "AnimGraphNode_Base.h"
#include "AnimGraphNode_Root.h"
#include "AnimGraphNode_SumeruATFFaceSync.h"
#include "Animation/AnimBlueprint.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimNodeBase.h"
#include "Animation/Skeleton.h"
#include "AssetToolsModule.h"
#include "FileHelpers.h"
#include "IAssetTools.h"
#include "AssetRegistry/AssetData.h"
#include "ATFApiClient.h"
#include "ATFApiSampleActor.h"
#include "ATFComponent.h"
#include "ATF_Type.h"
#include "AudioToFace.h"
#include "Components/SkeletalMeshComponent.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphPin.h"
#include "EdGraphSchema_K2.h"
#include "Editor.h"
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "ScopedTransaction.h"
#include "Engine/Selection.h"

#define LOCTEXT_NAMESPACE "SumeruAIATFSetup"

namespace
{
	USCS_Node* FindScsNodeOfClass(UBlueprint* Blueprint, const UClass* Class)
	{
		if (!Blueprint || !Blueprint->SimpleConstructionScript || !Class)
		{
			return nullptr;
		}
		for (USCS_Node* Node : Blueprint->SimpleConstructionScript->GetAllNodes())
		{
			if (Node && Node->ComponentClass && Node->ComponentClass->IsChildOf(Class))
			{
				return Node;
			}
		}
		return nullptr;
	}

	USCS_Node* FindFaceMeshScsNode(UBlueprint* Blueprint)
	{
		if (!Blueprint || !Blueprint->SimpleConstructionScript)
		{
			return nullptr;
		}

		USCS_Node* Best = nullptr;
		int32 BestScore = MIN_int32;
		for (USCS_Node* Node : Blueprint->SimpleConstructionScript->GetAllNodes())
		{
			if (!Node || !Node->ComponentClass || !Node->ComponentClass->IsChildOf(USkeletalMeshComponent::StaticClass()))
			{
				continue;
			}

			int32 Score = 0;
			const FString Name = Node->GetVariableName().ToString();
			if (Name.Equals(TEXT("Face"), ESearchCase::IgnoreCase))
			{
				Score += 1200;
			}
			else if (Name.Contains(TEXT("Face"), ESearchCase::IgnoreCase)
				&& !Name.Contains(TEXT("Facewear"), ESearchCase::IgnoreCase))
			{
				Score += 600;
			}
			if (Name.Contains(TEXT("Hair"), ESearchCase::IgnoreCase)
				|| Name.Contains(TEXT("Groom"), ESearchCase::IgnoreCase)
				|| Name.Contains(TEXT("Lash"), ESearchCase::IgnoreCase))
			{
				Score -= 600;
			}
			if (USkeletalMeshComponent* Mesh = Cast<USkeletalMeshComponent>(Node->ComponentTemplate))
			{
				if (USkeletalMesh* Asset = Mesh->GetSkeletalMeshAsset())
				{
					Score += Asset->GetMorphTargets().Num() * 2;
					Score += UATFComponent::CountARKitMorphMatches(Asset) * 20;
				}
			}
			if (Score > BestScore)
			{
				BestScore = Score;
				Best = Node;
			}
		}
		return Best;
	}

	USCS_Node* AddScsComponent(UBlueprint* Blueprint, UClass* Class, const FName& Name)
	{
		if (!Blueprint || !Blueprint->SimpleConstructionScript || !Class)
		{
			return nullptr;
		}
		USCS_Node* Node = Blueprint->SimpleConstructionScript->CreateNode(Class, Name);
		if (Node)
		{
			Blueprint->SimpleConstructionScript->AddNode(Node);
		}
		return Node;
	}
}

bool FSumeruAIATFSetup::IsSkippableActor(const AActor* Actor)
{
	return !Actor
		|| Actor->IsA(AATFApiSampleActor::StaticClass())
		|| Actor->IsA(AAudioToFaceManager::StaticClass())
		|| Actor->IsA(APlayerStart::StaticClass());
}

USkeletalMeshComponent* FSumeruAIATFSetup::ResolveFaceMesh(AActor* Actor)
{
	return UATFComponent::FindFaceSkeletalMesh(Actor);
}

void FSumeruAIATFSetup::ApplyAtfDefaults(UATFComponent* Atf, USkeletalMeshComponent* FaceMesh)
{
	if (!Atf)
	{
		return;
	}
	Atf->Modify();
	Atf->bAutoInitOnBeginPlay = true;
	Atf->bReplaceAnimInstance = false;
	Atf->CurMotionType = MotionType::ARKit;
	if (FaceMesh)
	{
		Atf->FaceCom = FaceMesh;
	}
}

void FSumeruAIATFSetup::WireSampleActors(UWorld* World, USkeletalMeshComponent* FaceMesh, int32 FaceId)
{
	if (!World || !FaceMesh)
	{
		return;
	}
	for (TActorIterator<AATFApiSampleActor> It(World); It; ++It)
	{
		AATFApiSampleActor* Sample = *It;
		if (!Sample)
		{
			continue;
		}
		Sample->Modify();
		Sample->FaceMesh = FaceMesh;
		Sample->FaceId = FaceId;
	}
}

bool FSumeruAIATFSetup::AddInstanceComponent(AActor* Actor, UActorComponent* Component)
{
	if (!Actor || !Component)
	{
		return false;
	}
	Actor->Modify();
	Component->CreationMethod = EComponentCreationMethod::Instance;
	Actor->AddInstanceComponent(Component);
	Component->RegisterComponent();
	return true;
}

namespace
{
	UEdGraph* FindAnimGraph(UAnimBlueprint* AnimBP)
	{
		if (!AnimBP)
		{
			return nullptr;
		}

		TArray<UEdGraph*> Graphs;
		AnimBP->GetAllGraphs(Graphs);
		for (UEdGraph* Graph : Graphs)
		{
			if (Graph && Graph->GetName().Equals(TEXT("AnimGraph"), ESearchCase::IgnoreCase))
			{
				return Graph;
			}
		}
		return nullptr;
	}

	bool GraphHasFaceSync(const UEdGraph* Graph)
	{
		if (!Graph)
		{
			return false;
		}
		for (const UEdGraphNode* Node : Graph->Nodes)
		{
			if (Cast<UAnimGraphNode_SumeruATFFaceSync>(Node))
			{
				return true;
			}
		}
		return false;
	}

	UEdGraphPin* FindPosePin(UEdGraphNode* Node, EEdGraphPinDirection Direction)
	{
		if (!Node)
		{
			return nullptr;
		}
		for (UEdGraphPin* Pin : Node->Pins)
		{
			if (!Pin || Pin->Direction != Direction || Pin->bHidden || Pin->bOrphanedPin)
			{
				continue;
			}
			if (Pin->PinType.PinCategory == UEdGraphSchema_K2::PC_Struct
				&& Pin->PinType.PinSubCategoryObject == FPoseLink::StaticStruct())
			{
				return Pin;
			}
		}
		return nullptr;
	}

	UAnimGraphNode_Base* FindInsertTarget(UEdGraph* Graph, bool& bOutIsRigLogic)
	{
		bOutIsRigLogic = false;
		if (!Graph)
		{
			return nullptr;
		}

		UAnimGraphNode_Base* RigLogic = nullptr;
		UAnimGraphNode_Base* Root = nullptr;
		for (UEdGraphNode* Node : Graph->Nodes)
		{
			if (!Node)
			{
				continue;
			}
			if (Node->GetClass()->GetName().Contains(TEXT("RigLogic")))
			{
				if (UAnimGraphNode_Base* AsAnim = Cast<UAnimGraphNode_Base>(Node))
				{
					RigLogic = AsAnim;
				}
			}
			if (UAnimGraphNode_Root* AsRoot = Cast<UAnimGraphNode_Root>(Node))
			{
				Root = AsRoot;
			}
		}

		if (RigLogic)
		{
			bOutIsRigLogic = true;
			return RigLogic;
		}
		return Root;
	}

	UAnimBlueprint* AnimBPFromClass(UClass* AnimClass)
	{
		return AnimClass ? Cast<UAnimBlueprint>(AnimClass->ClassGeneratedBy) : nullptr;
	}

	const TCHAR* PluginFaceAnimBPPath = TEXT("/SumeruAI/SumeruAI/MetaHumans/Animations/SumeruAI_MetaHuman_FaceAnim.SumeruAI_MetaHuman_FaceAnim");
	const TCHAR* ProjectFaceAnimBPPath = TEXT("/Game/SumeruAI/MetaHumans/Animations/SumeruAI_MetaHuman_FaceAnim.SumeruAI_MetaHuman_FaceAnim");
	const TCHAR* ProjectFaceAnimBPDir = TEXT("/Game/SumeruAI/MetaHumans/Animations");
	const TCHAR* CommonFaceAnimBPPath = TEXT("/Game/MetaHumans/Common/Face/Face_AnimBP.Face_AnimBP");

	void AssignAnimBPToFace(USkeletalMeshComponent* FaceMesh, UAnimBlueprint* AnimBP)
	{
		if (!FaceMesh || !AnimBP || !AnimBP->GeneratedClass)
		{
			return;
		}
		FaceMesh->Modify();
		FaceMesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		FaceMesh->SetAnimInstanceClass(AnimBP->GeneratedClass);
	}

	UAnimBlueprint* LoadSumeruFaceAnimBP()
	{
		if (UAnimBlueprint* PluginBP = LoadObject<UAnimBlueprint>(nullptr, PluginFaceAnimBPPath))
		{
			return PluginBP;
		}
		return LoadObject<UAnimBlueprint>(nullptr, ProjectFaceAnimBPPath);
	}

	UAnimBlueprint* FindSourceFaceAnimBP(USkeletalMeshComponent* FaceMesh)
	{
		if (UAnimBlueprint* FromFace = AnimBPFromClass(FaceMesh ? FaceMesh->GetAnimClass() : nullptr))
		{
			if (FromFace->GetName() != TEXT("SumeruAI_MetaHuman_FaceAnim"))
			{
				return FromFace;
			}
		}
		return LoadObject<UAnimBlueprint>(nullptr, CommonFaceAnimBPPath);
	}

	UAnimBlueprint* DuplicateFaceAnimBPToProject(UAnimBlueprint* Source, FString& OutMessage)
	{
		if (!Source)
		{
			OutMessage = TEXT("no source Face AnimBP to duplicate");
			return nullptr;
		}

		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		UObject* Duplicated = AssetTools.DuplicateAsset(
			TEXT("SumeruAI_MetaHuman_FaceAnim"),
			ProjectFaceAnimBPDir,
			Source);
		UAnimBlueprint* NewBP = Cast<UAnimBlueprint>(Duplicated);
		if (!NewBP)
		{
			OutMessage = TEXT("failed to create /Game/SumeruAI/MetaHumans/Animations/SumeruAI_MetaHuman_FaceAnim");
			return nullptr;
		}
		return NewBP;
	}

	bool InsertFaceSyncIntoThisAnimBP(UAnimBlueprint* AnimBP, FString& OutMessage)
	{
		if (!AnimBP)
		{
			OutMessage = TEXT("no Face AnimBP");
			return false;
		}

		UEdGraph* ChosenGraph = FindAnimGraph(AnimBP);
		if (GraphHasFaceSync(ChosenGraph))
		{
			OutMessage = FString::Printf(TEXT("ATF Face Sync already in '%s'"), *AnimBP->GetName());
			return true;
		}

		bool bTargetIsRigLogic = false;
		UAnimGraphNode_Base* Target = FindInsertTarget(ChosenGraph, bTargetIsRigLogic);
		UAnimBlueprint* Chosen = AnimBP;

		if (!ChosenGraph || !Target)
		{
			OutMessage = TEXT("could not find Face AnimGraph");
			return false;
		}

		const FString AnimBPPackageName = Chosen->GetOutermost()->GetName();
		if (AnimBPPackageName.StartsWith(TEXT("/Engine/")))
		{
			OutMessage = TEXT("Face AnimBP is an engine asset; copy it into the project first");
			return false;
		}

		Chosen->Modify();
		ChosenGraph->Modify();

		UAnimGraphNode_SumeruATFFaceSync* NewNode = NewObject<UAnimGraphNode_SumeruATFFaceSync>(ChosenGraph);
		if (!NewNode)
		{
			OutMessage = TEXT("failed to create ATF Face Sync node");
			return false;
		}
		NewNode->CreateNewGuid();
		NewNode->PostPlacedNewNode();
		NewNode->AllocateDefaultPins();
		NewNode->NodePosX = Target->NodePosX - 280;
		NewNode->NodePosY = Target->NodePosY;
		ChosenGraph->AddNode(NewNode, true, false);

		UEdGraphPin* TargetIn = FindPosePin(Target, EGPD_Input);
		UEdGraphPin* SyncIn = FindPosePin(NewNode, EGPD_Input);
		UEdGraphPin* SyncOut = FindPosePin(NewNode, EGPD_Output);
		if (!TargetIn || !SyncIn || !SyncOut)
		{
			OutMessage = TEXT("could not find pose pins on Face AnimBP nodes");
			return false;
		}

		const UEdGraphSchema* Schema = ChosenGraph->GetSchema();
		if (!Schema)
		{
			OutMessage = TEXT("Face AnimBP has no graph schema");
			return false;
		}

		const TArray<UEdGraphPin*> PreviousPins = TargetIn->LinkedTo;
		TargetIn->BreakAllPinLinks();
		for (UEdGraphPin* Previous : PreviousPins)
		{
			if (Previous)
			{
				Schema->TryCreateConnection(Previous, SyncIn);
			}
		}
		Schema->TryCreateConnection(SyncOut, TargetIn);

		FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Chosen);
		FKismetEditorUtilities::CompileBlueprint(Chosen, EBlueprintCompileOptions::SkipGarbageCollection);
		Chosen->MarkPackageDirty();

		OutMessage = FString::Printf(
			TEXT("inserted ATF Face Sync into '%s' before %s%s"),
			*Chosen->GetName(),
			*Target->GetNodeTitle(ENodeTitleType::ListView).ToString(),
			bTargetIsRigLogic ? TEXT("") : TEXT(" (open Face AnimBP and keep this node before RigLogic)"));
		return true;
	}
}

bool FSumeruAIATFSetup::InsertFaceSyncIntoFaceAnimBP(USkeletalMeshComponent* FaceMesh, FString& OutMessage)
{
	return AssignSumeruFaceAnimBP(FaceMesh, OutMessage);
}

bool FSumeruAIATFSetup::AssignSumeruFaceAnimBP(USkeletalMeshComponent* FaceMesh, FString& OutMessage)
{
	if (!FaceMesh)
	{
		OutMessage = TEXT("no Face mesh");
		return false;
	}

	UAnimBlueprint* Dedicated = LoadSumeruFaceAnimBP();
	if (!Dedicated)
	{
		FString DuplicateError;
		Dedicated = DuplicateFaceAnimBPToProject(FindSourceFaceAnimBP(FaceMesh), DuplicateError);
		if (!Dedicated)
		{
			OutMessage = DuplicateError;
			return false;
		}
	}

	FString InsertMessage;
	if (!InsertFaceSyncIntoThisAnimBP(Dedicated, InsertMessage))
	{
		OutMessage = InsertMessage;
		return false;
	}

	if (!Dedicated->GeneratedClass)
	{
		FKismetEditorUtilities::CompileBlueprint(Dedicated, EBlueprintCompileOptions::SkipGarbageCollection);
	}
	if (!Dedicated->GeneratedClass)
	{
		OutMessage = TEXT("SumeruAI_MetaHuman_FaceAnim failed to compile");
		return false;
	}

	AssignAnimBPToFace(FaceMesh, Dedicated);

	if (UPackage* Package = Dedicated->GetOutermost())
	{
		TArray<UPackage*> Packages = { Package };
		UEditorLoadingAndSavingUtils::SavePackages(Packages, true);
	}

	OutMessage = FString::Printf(
		TEXT("assigned '%s' to Face (%s)"),
		*Dedicated->GetName(),
		*InsertMessage);
	return true;
}

FSumeruAIATFSetupResult FSumeruAIATFSetup::SetupBlueprint(UBlueprint* Blueprint)
{
	FSumeruAIATFSetupResult Result;
	if (!Blueprint)
	{
		Result.Message = TEXT("No Blueprint selected.");
		return Result;
	}
	if (!Blueprint->ParentClass || !Blueprint->ParentClass->IsChildOf(AActor::StaticClass()))
	{
		Result.Message = FString::Printf(TEXT("'%s' is not an Actor Blueprint."), *Blueprint->GetName());
		return Result;
	}
	if (!Blueprint->SimpleConstructionScript)
	{
		Result.Message = FString::Printf(TEXT("'%s' has no construction script."), *Blueprint->GetName());
		return Result;
	}

	FScopedTransaction Transaction(LOCTEXT("SetupBlueprintTx", "Setup SumeruAI ATF on Blueprint"));
	Blueprint->Modify();

	TArray<FString> Changes;
	USCS_Node* AtfNode = FindScsNodeOfClass(Blueprint, UATFComponent::StaticClass());
	if (!AtfNode)
	{
		AtfNode = AddScsComponent(Blueprint, UATFComponent::StaticClass(), TEXT("ATFComponent"));
		if (AtfNode)
		{
			Changes.Add(TEXT("added ATFComponent"));
		}
	}

	USkeletalMeshComponent* FaceTemplate = nullptr;
	if (USCS_Node* FaceNode = FindFaceMeshScsNode(Blueprint))
	{
		FaceTemplate = Cast<USkeletalMeshComponent>(FaceNode->ComponentTemplate);
		Changes.Add(FString::Printf(TEXT("Face='%s'"), *FaceNode->GetVariableName().ToString()));
	}
	else if (UBlueprintGeneratedClass* Generated = Cast<UBlueprintGeneratedClass>(Blueprint->GeneratedClass))
	{
		if (AActor* Cdo = Cast<AActor>(Generated->GetDefaultObject()))
		{
			FaceTemplate = UATFComponent::FindFaceSkeletalMesh(Cdo);
			if (FaceTemplate)
			{
				Changes.Add(FString::Printf(TEXT("Face='%s' (inherited)"), *FaceTemplate->GetName()));
			}
		}
	}

	if (AtfNode)
	{
		if (UATFComponent* Atf = Cast<UATFComponent>(AtfNode->ComponentTemplate))
		{
			ApplyAtfDefaults(Atf, FaceTemplate);
			Changes.Add(TEXT("ARKit + keep Face AnimBP"));
		}
	}

	if (FaceTemplate)
	{
		FString FaceSyncMessage;
		if (InsertFaceSyncIntoFaceAnimBP(FaceTemplate, FaceSyncMessage))
		{
			Changes.Add(FaceSyncMessage);
		}
		else
		{
			Changes.Add(FString::Printf(TEXT("Face Sync skipped: %s"), *FaceSyncMessage));
		}
	}

	if (Changes.IsEmpty())
	{
		Result.Message = FString::Printf(TEXT("Could not add ATFComponent to '%s'."), *Blueprint->GetName());
		return Result;
	}

	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
	Blueprint->MarkPackageDirty();

	bool bHasPlacedInstance = false;
	if (GEditor)
	{
		if (UWorld* World = GEditor->GetEditorWorldContext().World())
		{
			for (TActorIterator<AActor> It(World); It; ++It)
			{
				if (*It && It->GetClass() && It->GetClass()->ClassGeneratedBy == Blueprint)
				{
					bHasPlacedInstance = true;
					break;
				}
			}
		}
	}

	if (bHasPlacedInstance)
	{
		Changes.Add(TEXT("skipped compiling the character Blueprint because it is placed in the level"));
	}
	else
	{
		FKismetEditorUtilities::CompileBlueprint(Blueprint, EBlueprintCompileOptions::SkipGarbageCollection);
	}

	Result.bSuccess = true;
	Result.Message = FString::Printf(
		TEXT("Setup ATF on Blueprint '%s': %s. %s"),
		*Blueprint->GetName(),
		*FString::Join(Changes, TEXT(", ")),
		bHasPlacedInstance
			? TEXT("Compile the Blueprint from its editor, or run Setup ATF on the placed actor.")
			: TEXT("Place it in the level, then PIE."));
	return Result;
}

FSumeruAIATFSetupResult FSumeruAIATFSetup::SetupActor(AActor* Actor)
{
	FSumeruAIATFSetupResult Result;
	if (!Actor)
	{
		Result.Message = TEXT("No actor selected.");
		return Result;
	}
	if (IsSkippableActor(Actor))
	{
		Result.Message = FString::Printf(TEXT("Skipped '%s' (not a character)."), *Actor->GetName());
		return Result;
	}

	if (!IsValid(Actor))
	{
		Result.Message = TEXT("Selected actor is no longer valid.");
		return Result;
	}

	USkeletalMeshComponent* Face = ResolveFaceMesh(Actor);
	if (!Face)
	{
		Result.Message = FString::Printf(
			TEXT("'%s' has no skeletal mesh. Select a MetaHuman or ARKit character."),
			*Actor->GetName());
		return Result;
	}

	FScopedTransaction Transaction(LOCTEXT("SetupActorTx", "Setup SumeruAI ATF on Actor"));
	TArray<FString> Changes;

	UATFComponent* Atf = Actor->FindComponentByClass<UATFComponent>();
	if (!Atf)
	{
		Atf = NewObject<UATFComponent>(Actor, TEXT("ATFComponent"));
		if (!Atf || !AddInstanceComponent(Actor, Atf))
		{
			Result.Message = FString::Printf(TEXT("Failed to add ATFComponent to '%s'."), *Actor->GetName());
			return Result;
		}
		Changes.Add(TEXT("added ATFComponent"));
	}
	ApplyAtfDefaults(Atf, Face);
	Changes.Add(FString::Printf(TEXT("Face='%s'"), *Face->GetName()));
	Changes.Add(TEXT("ARKit + keep Face AnimBP"));

	{
		FString FaceSyncMessage;
		if (InsertFaceSyncIntoFaceAnimBP(Face, FaceSyncMessage))
		{
			Changes.Add(FaceSyncMessage);
		}
		else
		{
			Changes.Add(FString::Printf(TEXT("Face Sync skipped: %s"), *FaceSyncMessage));
		}
	}

	UWorld* World = Actor->GetWorld();
	const bool bHasSampleActor = World && static_cast<bool>(TActorIterator<AATFApiSampleActor>(World));
	if (!Actor->FindComponentByClass<UATFApiClient>() && !bHasSampleActor)
	{
		UATFApiClient* Client = NewObject<UATFApiClient>(Actor, TEXT("ATFApiClient"));
		if (AddInstanceComponent(Actor, Client))
		{
			Client->FaceId = Atf->Id;
			Changes.Add(TEXT("added ATFApiClient"));
		}
	}

	WireSampleActors(Actor->GetWorld(), Face, Atf->Id);
	Actor->MarkPackageDirty();

	Result.bSuccess = true;
	Result.Message = FString::Printf(
		TEXT("Setup ATF on '%s': %s."),
		*Actor->GetName(),
		*FString::Join(Changes, TEXT(", ")));
	return Result;
}

FSumeruAIATFSetupResult FSumeruAIATFSetup::SetupSelectedActors()
{
	FSumeruAIATFSetupResult Result;
	if (!GEditor)
	{
		Result.Message = TEXT("Editor is not available.");
		return Result;
	}

	TArray<AActor*> Actors;
	GEditor->GetSelectedActors()->GetSelectedObjects<AActor>(Actors);

	int32 SuccessCount = 0;
	TArray<FString> Messages;
	for (AActor* Actor : Actors)
	{
		if (IsSkippableActor(Actor))
		{
			continue;
		}
		const FSumeruAIATFSetupResult One = SetupActor(Actor);
		Messages.Add(One.Message);
		if (One.bSuccess)
		{
			++SuccessCount;
		}
	}

	if (Messages.IsEmpty())
	{
		Result.Message = TEXT("Select a MetaHuman or ARKit character in the level, then run Setup ATF.");
		return Result;
	}

	Result.bSuccess = SuccessCount > 0;
	Result.Message = FString::Join(Messages, TEXT("\n"));
	return Result;
}

FSumeruAIATFSetupResult FSumeruAIATFSetup::SetupSelectedBlueprints()
{
	FSumeruAIATFSetupResult Result;
	if (!FModuleManager::Get().IsModuleLoaded(TEXT("ContentBrowser")))
	{
		Result.Message = TEXT("Content Browser is not available.");
		return Result;
	}

	TArray<FAssetData> Selected;
	FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser")).Get().GetSelectedAssets(Selected);

	int32 SuccessCount = 0;
	TArray<FString> Messages;
	for (const FAssetData& Asset : Selected)
	{
		if (UBlueprint* Blueprint = Cast<UBlueprint>(Asset.GetAsset()))
		{
			const FSumeruAIATFSetupResult One = SetupBlueprint(Blueprint);
			Messages.Add(One.Message);
			if (One.bSuccess)
			{
				++SuccessCount;
			}
		}
	}

	if (Messages.IsEmpty())
	{
		Result.Message = TEXT("Select an Actor Blueprint (MetaHuman), then run Setup ATF.");
		return Result;
	}

	Result.bSuccess = SuccessCount > 0;
	Result.Message = FString::Join(Messages, TEXT("\n"));
	return Result;
}

#undef LOCTEXT_NAMESPACE
