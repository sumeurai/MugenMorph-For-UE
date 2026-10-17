// Copyright 2026 Mugen3D. All Rights Reserved.

#include "Mugen3DIKRuntime.h"

#define LOCTEXT_NAMESPACE "FMugen3DIKRuntime"

void FMugen3DIKRuntime::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FMugen3DIKRuntime::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FMugen3DIKRuntime, Mugen3DIKRuntime)