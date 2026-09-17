// Copyright 2026 SumeruAI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

struct FToolMenuSection;

class FSumeruAIATFEditorModule : public IModuleInterface
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
