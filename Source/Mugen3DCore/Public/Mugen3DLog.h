// Copyright 2026 Mugen3D. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

enum EMugen3DLogLevel
{
	Log,
	Warning,
	Error,
};


namespace Mugen3D
{
		static bool ShowLog = true;
	
		static void Mugen3DLog(FString content, EMugen3DLogLevel Mugen3DLogLevel = EMugen3DLogLevel::Log)
		{
			if (ShowLog)
			{
				switch (Mugen3DLogLevel)
				{
				case EMugen3DLogLevel::Log:
					UE_LOG(LogTemp, Display, TEXT(" %s"), *content);
					break;
				case EMugen3DLogLevel::Warning:
					UE_LOG(LogTemp, Warning, TEXT(" %s"), *content);
					break;
				case EMugen3DLogLevel::Error:
					UE_LOG(LogTemp, Error, TEXT(" %s"), *content);
					break;
				default:
					break;
				}
			}
			
		}

		static FString IntToS(int i)
		{
			return FString::FromInt(i);
		}

		static FString BoolToS(bool b)
		{
			return b ? TEXT("true") : TEXT("false");
		}

		static FString FloatToS(float f)
		{
			return FString::SanitizeFloat(f);
		}
}
