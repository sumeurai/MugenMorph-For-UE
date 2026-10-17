// Copyright 2026 Mugen3D. All Rights Reserved.

#include "Mugen3DIKEditor.h"

#define LOCTEXT_NAMESPACE "FMugen3DIKEditor"

void FMugen3DIKEditor::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FMugen3DIKEditor::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FMugen3DIKEditor, Mugen3DIKEditor)