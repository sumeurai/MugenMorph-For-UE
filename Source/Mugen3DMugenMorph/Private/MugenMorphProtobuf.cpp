// Copyright 2026 Mugen3D. All Rights Reserved.

#include "MugenMorphProtobuf.h"

namespace MugenMorphProtobufPrivate
{
	class FProtoReader
	{
	public:
		FProtoReader(const uint8* InData, int32 InStart, int32 InEnd)
			: Data(InData)
			, Pos(InStart)
			, End(InEnd)
			, bFailed(false)
		{
		}

		bool TryReadTag(int32& OutField, int32& OutWire)
		{
			OutField = 0;
			OutWire = 0;
			if (Pos >= End || bFailed)
			{
				return false;
			}

			const uint64 Tag = ReadVarint();
			if (bFailed)
			{
				return false;
			}
			OutField = static_cast<int32>(Tag >> 3);
			OutWire = static_cast<int32>(Tag & 7);
			return true;
		}

		uint64 ReadVarint()
		{
			uint64 Result = 0;
			int32 Shift = 0;
			while (Pos < End)
			{
				const uint8 Byte = Data[Pos++];
				Result |= static_cast<uint64>(Byte & 0x7F) << Shift;
				if ((Byte & 0x80) == 0)
				{
					return Result;
				}
				Shift += 7;
				if (Shift > 63)
				{
					bFailed = true;
					return 0;
				}
			}
			bFailed = true;
			return 0;
		}

		TArray<uint8> ReadBytes()
		{
			TArray<uint8> Copy;
			int32 Len = static_cast<int32>(ReadVarint());
			if (bFailed)
			{
				return Copy;
			}
			if (Len < 0)
			{
				Len = 0;
			}
			const int32 Remain = End - Pos;
			if (Len > Remain)
			{
				Len = Remain;
			}
			if (Len > 0)
			{
				Copy.Append(Data + Pos, Len);
				Pos += Len;
			}
			return Copy;
		}

		double ReadDouble()
		{
			if (!Ensure(8))
			{
				return 0.0;
			}
			double Value = 0.0;
			FMemory::Memcpy(&Value, Data + Pos, 8);
			Pos += 8;
			return Value;
		}

		float ReadFloat()
		{
			if (!Ensure(4))
			{
				return 0.f;
			}
			float Value = 0.f;
			FMemory::Memcpy(&Value, Data + Pos, 4);
			Pos += 4;
			return Value;
		}

		void Skip(int32 Wire)
		{
			switch (Wire)
			{
			case 0:
				ReadVarint();
				break;
			case 1:
				if (Ensure(8))
				{
					Pos += 8;
				}
				break;
			case 2:
			{
				int32 Len = static_cast<int32>(ReadVarint());
				const int32 Remain = End - Pos;
				if (Len > Remain)
				{
					Len = Remain;
				}
				if (Len > 0)
				{
					Pos += Len;
				}
				break;
			}
			case 5:
				if (Ensure(4))
				{
					Pos += 4;
				}
				break;
			default:
				bFailed = true;
				break;
			}
		}

		bool HasFailed() const { return bFailed; }

	private:
		bool Ensure(int32 Count)
		{
			if (Pos + Count > End)
			{
				bFailed = true;
				return false;
			}
			return true;
		}

		const uint8* Data;
		int32 Pos;
		int32 End;
		bool bFailed;
	};

	static void ReadMessage(const uint8* Data, int32 Start, int32 End, FMugenMorphProtobufResult& Result, int32 Depth)
	{
		FProtoReader Reader(Data, Start, End);
		int32 Field = 0;
		int32 Wire = 0;
		while (Reader.TryReadTag(Field, Wire))
		{
			if (Field < 1 || Wire == 3 || Wire == 4 || Wire == 6 || Wire == 7)
			{
				break;
			}

			if (Wire == 0)
			{
				const int32 Value = static_cast<int32>(Reader.ReadVarint());
				if (Depth == 0 && Field == 1)
				{
					Result.Code = Value;
				}
				else if (Depth == 1)
				{
					if (Field == 1)
					{
						Result.Fps = static_cast<float>(Value);
					}
					else if (Field == 2)
					{
						Result.NumFrames = Value;
					}
				}
				continue;
			}

			if (Wire == 1)
			{
				const double Value = Reader.ReadDouble();
				if (Depth == 1 && Field == 1)
				{
					Result.Fps = static_cast<float>(Value);
				}
				continue;
			}

			if (Wire == 5)
			{
				const float Value = Reader.ReadFloat();
				if (Depth == 1 && Field == 1)
				{
					Result.Fps = Value;
				}
				continue;
			}

			if (Wire != 2)
			{
				Reader.Skip(Wire);
				continue;
			}

			TArray<uint8> Raw = Reader.ReadBytes();
			if (Depth == 0)
			{
				if (Field == 2)
				{
					if (Raw.Num() > 0)
					{
						FUTF8ToTCHAR Convert(reinterpret_cast<const ANSICHAR*>(Raw.GetData()), Raw.Num());
						Result.Message = FString(Convert.Length(), Convert.Get());
					}
				}
				else if (Field == 3 && Raw.Num() > 0)
				{
					ReadMessage(Raw.GetData(), 0, Raw.Num(), Result, 1);
				}
			}
			else if (Field == 3)
			{
				Result.Audio = MoveTemp(Raw);
			}
			else if (Field == 4)
			{
				Result.Blendshapes = MoveTemp(Raw);
			}
		}
	}
}

FMugenMorphProtobufResult FMugenMorphProtobufParser::Parse(const TArray<uint8>& Data)
{
	return Parse(Data.GetData(), Data.Num());
}

FMugenMorphProtobufResult FMugenMorphProtobufParser::Parse(const uint8* Data, int32 Num)
{
	FMugenMorphProtobufResult Result;
	if (!Data || Num <= 0)
	{
		return Result;
	}
	MugenMorphProtobufPrivate::ReadMessage(Data, 0, Num, Result, 0);
	return Result;
}
