// Copyright 2026 Mugen3D. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

struct FToolMenuSection;

class FMugen3DMugenMorphEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	void RegisterMenus();
	static void OpenApiSettings();
	static void CreateAtfDemoLevel();
	static void SetupAtfOnSelection();
	static void PopulateBlueprintAssetMenu(FToolMenuSection& InSection);
};
