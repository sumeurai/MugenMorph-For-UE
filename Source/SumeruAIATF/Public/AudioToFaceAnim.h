// Copyright SumeruAI. All Rights Reserved.

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
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
        int FaceId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FAudioExpression FacialTempData;

    UFUNCTION(BlueprintCallable)
        UAnimInstance* GetSelfInstance();
};
