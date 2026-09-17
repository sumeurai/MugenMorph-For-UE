// Copyright 2026 SumeruAI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ATF_Type.h"
#include "AudioToFaceAnim.generated.h"

/**
 * 
 */
UCLASS()
class SUMERUAIATF_API UAudioToFaceAnim : public UAnimInstance
{
	GENERATED_BODY()

public:	
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ATF")
        int FaceId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ATF")
		FAudioExpression FacialTempData;

    UFUNCTION(BlueprintCallable, Category = "ATF")
        UAnimInstance* GetSelfInstance();
};
