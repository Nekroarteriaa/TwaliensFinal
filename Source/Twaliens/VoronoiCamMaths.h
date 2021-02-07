// All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VoronoiCamMaths.generated.h"

/**
 * 
 */
UCLASS()
class TWALIENS_API UVoronoiCamMaths : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

		UFUNCTION(BlueprintPure, Category = "Constants")
		static float Divisor();

	UFUNCTION(BlueprintPure, Category = "Constants")
		static float ScreenDeadZone();

	UFUNCTION(BlueprintPure)
		static float XDeadZone(float X, const float& DeadZone);

	UFUNCTION(BlueprintPure)
		static FVector AbsVector(FVector& PassedVector);

	UFUNCTION(BlueprintPure, Category = "ReturnMethods")
		static void LocationToMiddlePoint(FVector PointA, FVector PointB, FVector& OutVectorA, FVector& OutVectorB);

	UFUNCTION(BlueprintPure)
		static FVector CalculateWorldCameraLocation(FVector Middle, FVector& ZeroToOne, FVector& MaxCameraOffset, FVector& PlayerLocation);

	UFUNCTION(BlueprintPure)
		static void GetVoronoiCameraLocation(FVector MaxCameraOffset, FVector PlayerLocation, APawn* PlayerPawn ,FVector& OutPlayerZeroCameraLocation, FVector& OutPlayerOneCameraLocation);

	UFUNCTION(BlueprintPure)
		static FVector MiddlePointVector(FVector& PlayerZero, FVector& PlayerOne);

	
};
