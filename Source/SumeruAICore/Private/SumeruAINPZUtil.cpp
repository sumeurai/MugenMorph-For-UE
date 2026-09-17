// Copyright 2026 SumeruAI. All Rights Reserved.

#include "SumeruAINPZUtil.h"
#include "SumeruAILog.h"
#include "cnpyy.h"
#include "Misc/Paths.h"

struct USumeruAINPZUtil::FNpzState
{
	cnpyy::npz_t Data;
};

namespace SumeruAINPZUtilPrivate
{
	static std::string FStringToString(const FString& InFString)
	{
		const char* CharArray = TCHAR_TO_UTF8(*InFString);
		return std::string(CharArray);
	}
}

USumeruAINPZUtil::USumeruAINPZUtil()
	: Width(0)
	, Height(0)
	, State(MakePimpl<FNpzState>())
{
}

USumeruAINPZUtil::~USumeruAINPZUtil() = default;

void USumeruAINPZUtil::LoadNPZ(const FString& FilePath)
{
	UE_LOG(LogTemp, Log, TEXT("*** Load NPZ File : %s ***"), *FilePath);
	State->Data = cnpyy::npz_load(SumeruAINPZUtilPrivate::FStringToString(FilePath));
}

TArray<float> USumeruAINPZUtil::GetVertices()
{
	TArray<float> ResultArray;
	const auto Key = SumeruAINPZUtilPrivate::FStringToString("vertices");
	if (State->Data.find(Key) != State->Data.end())
	{
		cnpyy::NpyArray Verts = State->Data[Key];
		auto Vals = Verts.as_vec<float>();
		for (size_t i = 0; i < Vals.size(); i++)
		{
			ResultArray.Add(Vals[i]);
		}
	}
	return ResultArray;
}

TArray<uint8> USumeruAINPZUtil::GetTextureData()
{
	TArray<uint8> ResultArray;
	const auto Key = SumeruAINPZUtilPrivate::FStringToString("texture");
	if (State->Data.find(Key) != State->Data.end())
	{
		cnpyy::NpyArray Verts = State->Data[Key];
		Height = Verts.shape[0];
		Width = Verts.shape[1];
		auto Vals = Verts.as_vec<uint8>();
		for (size_t i = 0; i < Vals.size(); i++)
		{
			ResultArray.Add(Vals[i]);
		}
	}
	return ResultArray;
}

FVector USumeruAINPZUtil::GetBoneData(const FString& Name)
{
	FVector Vector;
	const auto Key = SumeruAINPZUtilPrivate::FStringToString(Name);
	if (State->Data.find(Key) != State->Data.end())
	{
		cnpyy::NpyArray Verts = State->Data[Key];
		auto Vals = Verts.as_vec<float>();
		if (Vals.size() == 3)
		{
			Vector = FVector(Vals[0], -Vals[1], Vals[2]);
		}
	}
	else
	{
		Vector = FVector(0, 0, 1000);
	}
	return Vector;
}

FVector USumeruAINPZUtil::GetColor()
{
	FVector Vector;
	const auto Key = SumeruAINPZUtilPrivate::FStringToString("color_shift");
	if (State->Data.find(Key) != State->Data.end())
	{
		cnpyy::NpyArray Verts = State->Data[Key];
		auto Vals = Verts.as_vec<float>();
		if (Vals.size() == 3)
		{
			Vector = FVector(Vals[0], Vals[1], Vals[2]);
		}
	}
	else
	{
		Vector = FVector(0, 0, 0);
	}
	return Vector;
}

FVector USumeruAINPZUtil::GetCheekColor()
{
	FVector Vector;
	const auto Key = SumeruAINPZUtilPrivate::FStringToString("cheek_color");
	if (State->Data.find(Key) != State->Data.end())
	{
		cnpyy::NpyArray Verts = State->Data[Key];
		auto Vals = Verts.as_vec<float>();
		if (Vals.size() == 3)
		{
			Vector = FVector(Vals[0], Vals[1], Vals[2]);
		}
	}
	else
	{
		Vector = FVector(0, 0, 0);
	}
	return Vector;
}

FVector USumeruAINPZUtil::GetColorShift()
{
	FVector Vector;
	const auto Key = SumeruAINPZUtilPrivate::FStringToString("color_shift");
	if (State->Data.find(Key) != State->Data.end())
	{
		cnpyy::NpyArray Verts = State->Data[Key];
		auto Vals = Verts.as_vec<float>();
		if (Vals.size() == 3)
		{
			Vector = FVector(Vals[0], Vals[1], Vals[2]);
		}
	}
	else
	{
		Vector = FVector(0, 0, 0);
	}
	return Vector;
}

FVector USumeruAINPZUtil::GetHeadTranslate()
{
	FVector Vector;
	const auto Key = SumeruAINPZUtilPrivate::FStringToString("head_translate");
	if (State->Data.find(Key) != State->Data.end())
	{
		cnpyy::NpyArray Verts = State->Data[Key];
		auto Vals = Verts.as_vec<float>();
		if (Vals.size() == 3)
		{
			Vector = FVector(Vals[0], Vals[2], Vals[1]);
		}
	}
	else
	{
		Vector = FVector(0, 0, 0);
	}
	return Vector;
}

FVector USumeruAINPZUtil::GetHeadScale()
{
	FVector Vector;
	const auto Key = SumeruAINPZUtilPrivate::FStringToString("head_scale");
	if (State->Data.find(Key) != State->Data.end())
	{
		cnpyy::NpyArray Verts = State->Data[Key];
		auto Vals = Verts.as_vec<float>();
		if (Vals.size() == 3)
		{
			Vector = FVector(Vals[0], Vals[2], Vals[1]);
		}
	}
	else
	{
		Vector = FVector(1, 1, 1);
	}
	return Vector;
}

TArray<float> USumeruAINPZUtil::GetMale()
{
	TArray<float> ResultArray;
	const auto Key = SumeruAINPZUtilPrivate::FStringToString("dv_male");
	if (State->Data.find(Key) != State->Data.end())
	{
		cnpyy::NpyArray Verts = State->Data[Key];
		auto Vals = Verts.as_vec<float>();
		for (size_t i = 0; i < Vals.size(); i++)
		{
			ResultArray.Add(Vals[i]);
		}
	}
	return ResultArray;
}

TArray<float> USumeruAINPZUtil::GetMaleVertices()
{
	TArray<float> ResultArray;
	const auto Key = SumeruAINPZUtilPrivate::FStringToString("male_vertices");
	if (State->Data.find(Key) != State->Data.end())
	{
		cnpyy::NpyArray Verts = State->Data[Key];
		auto Vals = Verts.as_vec<float>();
		for (size_t i = 0; i < Vals.size(); i++)
		{
			ResultArray.Add(Vals[i]);
		}
	}
	return ResultArray;
}

FVector USumeruAINPZUtil::GetMaleScale()
{
	FVector Vector;
	const auto Key = SumeruAINPZUtilPrivate::FStringToString("scale_male");
	if (State->Data.find(Key) != State->Data.end())
	{
		cnpyy::NpyArray Verts = State->Data[Key];
		auto Vals = Verts.as_vec<float>();
		if (Vals.size() == 3)
		{
			Vector = FVector(Vals[0], Vals[2], Vals[1]);
		}
	}
	else
	{
		Vector = FVector(1, 1, 1);
	}
	return Vector;
}

float USumeruAINPZUtil::GetMaleScaleF()
{
	const auto Key = SumeruAINPZUtilPrivate::FStringToString("scale_male");
	if (State->Data.find(Key) != State->Data.end())
	{
		cnpyy::NpyArray Verts = State->Data[Key];
		auto Vals = Verts.as_vec<float>();
		return Vals[0];
	}
	return 1.0f;
}

TArray<float> USumeruAINPZUtil::GetFemale()
{
	TArray<float> ResultArray;
	const auto Key = SumeruAINPZUtilPrivate::FStringToString("dv_female");
	if (State->Data.find(Key) != State->Data.end())
	{
		cnpyy::NpyArray Verts = State->Data[Key];
		auto Vals = Verts.as_vec<float>();
		for (size_t i = 0; i < Vals.size(); i++)
		{
			ResultArray.Add(Vals[i]);
		}
	}
	return ResultArray;
}

TArray<float> USumeruAINPZUtil::GetFemaleVertices()
{
	TArray<float> ResultArray;
	const auto Key = SumeruAINPZUtilPrivate::FStringToString("female_vertices");
	if (State->Data.find(Key) != State->Data.end())
	{
		cnpyy::NpyArray Verts = State->Data[Key];
		auto Vals = Verts.as_vec<float>();
		for (size_t i = 0; i < Vals.size(); i++)
		{
			ResultArray.Add(Vals[i]);
		}
	}
	return ResultArray;
}

FVector USumeruAINPZUtil::GetFemaleScale()
{
	FVector Vector;
	const auto Key = SumeruAINPZUtilPrivate::FStringToString("scale_female");
	if (State->Data.find(Key) != State->Data.end())
	{
		cnpyy::NpyArray Verts = State->Data[Key];
		auto Vals = Verts.as_vec<float>();
		if (Vals.size() == 3)
		{
			Vector = FVector(Vals[0], Vals[2], Vals[1]);
		}
	}
	else
	{
		Vector = FVector(1, 1, 1);
	}
	return Vector;
}

float USumeruAINPZUtil::GetFemaleScaleF()
{
	const auto Key = SumeruAINPZUtilPrivate::FStringToString("scale_female");
	if (State->Data.find(Key) != State->Data.end())
	{
		cnpyy::NpyArray Verts = State->Data[Key];
		auto Vals = Verts.as_vec<float>();
		return Vals[0];
	}
	return 1.0f;
}

FAudioExpressionData USumeruAINPZUtil::GetCustomExpressionData()
{
	FAudioExpressionData AED;
	AED.Reset();

	const auto KeyFrames = SumeruAINPZUtilPrivate::FStringToString("frames");
	if (State->Data.find(KeyFrames) != State->Data.end())
	{
		cnpyy::NpyArray Verts = State->Data[KeyFrames];
		auto Vals = Verts.as_vec<float>();
		(void)Vals[0];
	}

	const auto KeyFps = SumeruAINPZUtilPrivate::FStringToString("fps");
	if (State->Data.find(KeyFps) != State->Data.end())
	{
		cnpyy::NpyArray Verts = State->Data[KeyFps];
		auto Vals = Verts.as_vec<float>();
		AED.Interval = 1.0f / static_cast<float>(Vals[0]);
		SumeruAI::SumeruAILog("fps : " + SumeruAI::IntToS(Vals[0]) + " " + SumeruAI::FloatToS(AED.Interval));
	}

	const auto KeyBlend = SumeruAINPZUtilPrivate::FStringToString("blendshapes");
	if (State->Data.find(KeyBlend) != State->Data.end())
	{
		cnpyy::NpyArray Verts = State->Data[KeyBlend];
		auto Vals = Verts.as_vec<float>();
		for (size_t i = 0; i <= Vals.size() - 61; i += 61)
		{
			FAudioExpression Temp;
			for (size_t j = i; j < i + 61; j++)
			{
				Temp.Data.Add(Vals[j]);
			}
			AED.Expressions.Add(Temp);
		}
	}

	SumeruAI::SumeruAILog("frames : " + SumeruAI::IntToS(AED.Expressions.Num()));
	return AED;
}

bool USumeruAINPZUtil::LoadCustomExpressionFromFile(const FString& FilePath, FAudioExpressionData& OutData)
{
	OutData.Reset();
	if (FilePath.IsEmpty() || !FPaths::FileExists(FilePath))
	{
		return false;
	}

	cnpyy::npz_t Data = cnpyy::npz_load(SumeruAINPZUtilPrivate::FStringToString(FilePath));

	const auto KeyFps = SumeruAINPZUtilPrivate::FStringToString("fps");
	if (Data.find(KeyFps) != Data.end())
	{
		cnpyy::NpyArray Verts = Data[KeyFps];
		auto Vals = Verts.as_vec<float>();
		if (!Vals.empty() && Vals[0] > KINDA_SMALL_NUMBER)
		{
			OutData.Interval = 1.0f / static_cast<float>(Vals[0]);
		}
	}

	const auto KeyBlend = SumeruAINPZUtilPrivate::FStringToString("blendshapes");
	if (Data.find(KeyBlend) == Data.end())
	{
		return false;
	}

	cnpyy::NpyArray Verts = Data[KeyBlend];
	auto Vals = Verts.as_vec<float>();
	if (Vals.size() < 61)
	{
		return false;
	}

	OutData.Expressions.Reserve(static_cast<int32>(Vals.size() / 61));
	for (size_t i = 0; i + 61 <= Vals.size(); i += 61)
	{
		FAudioExpression Temp;
		Temp.Data.Reserve(61);
		for (size_t j = i; j < i + 61; ++j)
		{
			Temp.Data.Add(Vals[j]);
		}
		OutData.Expressions.Add(MoveTemp(Temp));
	}

	return OutData.Expressions.Num() > 0;
}
