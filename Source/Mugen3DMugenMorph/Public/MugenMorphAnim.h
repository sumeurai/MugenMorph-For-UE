// Copyright 2026 Mugen3D. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "MugenMorph_Type.h"
#include "MugenMorphAnim.generated.h"

/**
 * 
 */
UCLASS()
class MUGEN3DMUGENMORPH_API UMugenMorphAnim : public UAnimInstance
{
	GENERATED_BODY()

public:	
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MugenMorph")
        int FaceId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MugenMorph")
		FAudioExpression FacialTempData;

    UFUNCTION(BlueprintCallable, Category = "MugenMorph")
        UAnimInstance* GetSelfInstance();
};
