// Copyright 2026 Mugen3D. All Rights Reserved.

#include "Mugen3DMugenMorph.h"

#define LOCTEXT_NAMESPACE "FMugen3DMugenMorph"

void FMugen3DMugenMorph::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FMugen3DMugenMorph::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FMugen3DMugenMorph, Mugen3DMugenMorph)