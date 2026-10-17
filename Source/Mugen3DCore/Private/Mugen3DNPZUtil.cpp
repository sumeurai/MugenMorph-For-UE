// Copyright 2026 Mugen3D. All Rights Reserved.

#include "Mugen3DNPZUtil.h"
#include "Mugen3DLog.h"
#include "cnpyy.h"
#include "Misc/Paths.h"

struct UMugen3DNPZUtil::FNpzState
{
	cnpyy::npz_t Data;
};

namespace Mugen3DNPZUtilPrivate
{
	static std::string FStringToString(const FString& InFString)
	{
		const char* CharArray = TCHAR_TO_UTF8(*InFString);
		return std::string(CharArray);
	}
}

UMugen3DNPZUtil::UMugen3DNPZUtil()
	: Width(0)
	, Height(0)
	, State(MakePimpl<FNpzState>())
{
}

UMugen3DNPZUtil::~UMugen3DNPZUtil() = default;

void UMugen3DNPZUtil::LoadNPZ(const FString& FilePath)
{
	UE_LOG(LogTemp, Log, TEXT("*** Load NPZ File : %s ***"), *FilePath);
	State->Data = cnpyy::npz_load(Mugen3DNPZUtilPrivate::FStringToString(FilePath));
}

TArray<float> UMugen3DNPZUtil::GetVertices()
{
	TArray<float> ResultArray;
	const auto Key = Mugen3DNPZUtilPrivate::FStringToString("vertices");
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

TArray<uint8> UMugen3DNPZUtil::GetTextureData()
{
	TArray<uint8> ResultArray;
	const auto Key = Mugen3DNPZUtilPrivate::FStringToString("texture");
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

FVector UMugen3DNPZUtil::GetBoneData(const FString& Name)
{
	FVector Vector;
	const auto Key = Mugen3DNPZUtilPrivate::FStringToString(Name);
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

FVector UMugen3DNPZUtil::GetColor()
{
	FVector Vector;
	const auto Key = Mugen3DNPZUtilPrivate::FStringToString("color_shift");
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

FVector UMugen3DNPZUtil::GetCheekColor()
{
	FVector Vector;
	const auto Key = Mugen3DNPZUtilPrivate::FStringToString("cheek_color");
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

FVector UMugen3DNPZUtil::GetColorShift()
{
	FVector Vector;
	const auto Key = Mugen3DNPZUtilPrivate::FStringToString("color_shift");
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

FVector UMugen3DNPZUtil::GetHeadTranslate()
{
	FVector Vector;
	const auto Key = Mugen3DNPZUtilPrivate::FStringToString("head_translate");
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

FVector UMugen3DNPZUtil::GetHeadScale()
{
	FVector Vector;
	const auto Key = Mugen3DNPZUtilPrivate::FStringToString("head_scale");
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

TArray<float> UMugen3DNPZUtil::GetMale()
{
	TArray<float> ResultArray;
	const auto Key = Mugen3DNPZUtilPrivate::FStringToString("dv_male");
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

TArray<float> UMugen3DNPZUtil::GetMaleVertices()
{
	TArray<float> ResultArray;
	const auto Key = Mugen3DNPZUtilPrivate::FStringToString("male_vertices");
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

FVector UMugen3DNPZUtil::GetMaleScale()
{
	FVector Vector;
	const auto Key = Mugen3DNPZUtilPrivate::FStringToString("scale_male");
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

float UMugen3DNPZUtil::GetMaleScaleF()
{
	const auto Key = Mugen3DNPZUtilPrivate::FStringToString("scale_male");
	if (State->Data.find(Key) != State->Data.end())
	{
		cnpyy::NpyArray Verts = State->Data[Key];
		auto Vals = Verts.as_vec<float>();
		return Vals[0];
	}
	return 1.0f;
}

TArray<float> UMugen3DNPZUtil::GetFemale()
{
	TArray<float> ResultArray;
	const auto Key = Mugen3DNPZUtilPrivate::FStringToString("dv_female");
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

TArray<float> UMugen3DNPZUtil::GetFemaleVertices()
{
	TArray<float> ResultArray;
	const auto Key = Mugen3DNPZUtilPrivate::FStringToString("female_vertices");
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

FVector UMugen3DNPZUtil::GetFemaleScale()
{
	FVector Vector;
	const auto Key = Mugen3DNPZUtilPrivate::FStringToString("scale_female");
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

float UMugen3DNPZUtil::GetFemaleScaleF()
{
	const auto Key = Mugen3DNPZUtilPrivate::FStringToString("scale_female");
	if (State->Data.find(Key) != State->Data.end())
	{
		cnpyy::NpyArray Verts = State->Data[Key];
		auto Vals = Verts.as_vec<float>();
		return Vals[0];
	}
	return 1.0f;
}

FAudioExpressionData UMugen3DNPZUtil::GetCustomExpressionData()
{
	FAudioExpressionData AED;
	AED.Reset();

	const auto KeyFrames = Mugen3DNPZUtilPrivate::FStringToString("frames");
	if (State->Data.find(KeyFrames) != State->Data.end())
	{
		cnpyy::NpyArray Verts = State->Data[KeyFrames];
		auto Vals = Verts.as_vec<float>();
		(void)Vals[0];
	}

	const auto KeyFps = Mugen3DNPZUtilPrivate::FStringToString("fps");
	if (State->Data.find(KeyFps) != State->Data.end())
	{
		cnpyy::NpyArray Verts = State->Data[KeyFps];
		auto Vals = Verts.as_vec<float>();
		AED.Interval = 1.0f / static_cast<float>(Vals[0]);
		Mugen3D::Mugen3DLog("fps : " + Mugen3D::IntToS(Vals[0]) + " " + Mugen3D::FloatToS(AED.Interval));
	}

	const auto KeyBlend = Mugen3DNPZUtilPrivate::FStringToString("blendshapes");
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

	Mugen3D::Mugen3DLog("frames : " + Mugen3D::IntToS(AED.Expressions.Num()));
	return AED;
}

bool UMugen3DNPZUtil::LoadCustomExpressionFromFile(const FString& FilePath, FAudioExpressionData& OutData)
{
	OutData.Reset();
	if (FilePath.IsEmpty() || !FPaths::FileExists(FilePath))
	{
		return false;
	}

	cnpyy::npz_t Data = cnpyy::npz_load(Mugen3DNPZUtilPrivate::FStringToString(FilePath));

	const auto KeyFps = Mugen3DNPZUtilPrivate::FStringToString("fps");
	if (Data.find(KeyFps) != Data.end())
	{
		cnpyy::NpyArray Verts = Data[KeyFps];
		auto Vals = Verts.as_vec<float>();
		if (!Vals.empty() && Vals[0] > KINDA_SMALL_NUMBER)
		{
			OutData.Interval = 1.0f / static_cast<float>(Vals[0]);
		}
	}

	const auto KeyBlend = Mugen3DNPZUtilPrivate::FStringToString("blendshapes");
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
