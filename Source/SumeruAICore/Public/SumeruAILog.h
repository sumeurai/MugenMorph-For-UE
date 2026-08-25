// Copyright SumeruAI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

enum ESumeruAILogLevel
{
	Log,
	Warning,
	Error,
};


namespace SumeruAI
{
		static bool ShowLog = true;
	
		static void SumeruAILog(FString content, ESumeruAILogLevel SumeruAILogLevel = ESumeruAILogLevel::Log)
		{
			if (ShowLog)
			{
				switch (SumeruAILogLevel)
				{
				case ESumeruAILogLevel::Log:
					UE_LOG(LogTemp, Display, TEXT(" %s"), *content);
					break;
				case ESumeruAILogLevel::Warning:
					UE_LOG(LogTemp, Warning, TEXT(" %s"), *content);
					break;
				case ESumeruAILogLevel::Error:
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
