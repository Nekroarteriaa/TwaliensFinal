// All rights reserved.

#include "VoronoiCamMaths.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "Runtime/Engine/Classes/GameFramework/Actor.h"
#include "Runtime/Engine/Classes/Components/ActorComponent.h"
#include "Object.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"


float UVoronoiCamMaths::Divisor()
{
	return 500.0f;
}

float UVoronoiCamMaths::ScreenDeadZone()
{
	return 550.0f;
}

float UVoronoiCamMaths::XDeadZone(float X, const float & DeadZone)
{
	float SignXValue = UKismetMathLibrary::SignOfFloat(X);
	float XAbs = UKismetMathLibrary::Abs(X);

	float TempSubstration = XAbs - DeadZone;
	float temp = UKismetMathLibrary::Max(TempSubstration, 0);

	auto xDeadZn = temp * SignXValue;

	return xDeadZn;
}

FVector UVoronoiCamMaths::AbsVector(FVector & PassedVector)
{
	float Xabs = UKismetMathLibrary::Abs(PassedVector.X);
	float Yabs = UKismetMathLibrary::Abs(PassedVector.Y);
	float Zabs = UKismetMathLibrary::Abs(PassedVector.Z);

	FVector absVect = FVector(Xabs, Yabs, Zabs);

	return absVect;
}

void UVoronoiCamMaths::LocationToMiddlePoint(FVector PointA, FVector PointB, FVector & OutVectorA, FVector & OutVectorB)
{
	FVector differenceVector = PointA - PointB;
	FVector normalizedDifferenceVector = differenceVector.GetSafeNormal();
	float differenceVectorLength = differenceVector.Size();

	//Applying Juicy Math 

	float deadZn = XDeadZone(differenceVectorLength, ScreenDeadZone());
	float rateofChange = deadZn / Divisor();

	auto atanRoC = UKismetMathLibrary::Atan(rateofChange);
	float piDivision = atanRoC / UKismetMathLibrary::GetPI();

	OutVectorA = normalizedDifferenceVector * piDivision;
	OutVectorB = UKismetMathLibrary::NegateVector(OutVectorA);

}

FVector UVoronoiCamMaths::CalculateWorldCameraLocation(FVector Middle, FVector & ZeroToOne, FVector & MaxCameraOffset, FVector & PlayerLocation)
{
	float zeroToOneLength = ZeroToOne.Size();
	FVector fromPlayerToMiddle = Middle - PlayerLocation;
	FVector negatedfromPTM = UKismetMathLibrary::NegateVector(fromPlayerToMiddle);
	FVector normalfromPTM = fromPlayerToMiddle.GetSafeNormal();
	FVector maxOffsetFromCentre = normalfromPTM * MaxCameraOffset;
	FVector additionVector = negatedfromPTM + maxOffsetFromCentre;

	FVector cameraPositionRelative = additionVector * zeroToOneLength;
	FVector cameraGlobalPosition = Middle + cameraPositionRelative;

	return cameraGlobalPosition;
}

void UVoronoiCamMaths::GetVoronoiCameraLocation(FVector MaxCameraOffset, FVector PlayerLocation, APawn* PlayerPawn ,FVector & OutPlayerZeroCameraLocation, FVector & OutPlayerOneCameraLocation)
{
	auto world = PlayerPawn->GetWorld();
	auto playerZero = UGameplayStatics::GetPlayerController(world, 0); //UGameplayStatics::GetPlayerController(UWorld::GetWorld(), 0);
	auto playerOne = UGameplayStatics::GetPlayerController(world, 1);

	FVector playerZeroLocation = playerZero->GetPawn()->GetActorLocation();
	FVector playerOneLocation = playerOne->GetPawn()->GetActorLocation();

	FVector Middle = MiddlePointVector(playerZeroLocation, playerOneLocation);


	//UE_LOG(YourLog, Warning, TEXT("MyCharacter's Location is %s"),		*MyCharacter->GetActorLocation().ToString());

	// Calculate Voronoi node locations
	FVector calculatedPlayerZero;
	FVector calculatedPlayerOne;
	LocationToMiddlePoint(playerZeroLocation, playerOneLocation, calculatedPlayerOne, calculatedPlayerZero);

	FVector playerZeroCoordfromMinusOneToOne = calculatedPlayerZero * 2.f;
	FVector playerOneCoordfromMinusOneToOne = calculatedPlayerOne * 2.f;

	playerZeroCoordfromMinusOneToOne = AbsVector(playerZeroCoordfromMinusOneToOne);
	playerOneCoordfromMinusOneToOne = AbsVector(playerOneCoordfromMinusOneToOne);
	

	OutPlayerZeroCameraLocation = CalculateWorldCameraLocation(Middle, playerZeroCoordfromMinusOneToOne, MaxCameraOffset, PlayerLocation);
	OutPlayerOneCameraLocation = CalculateWorldCameraLocation(Middle, playerOneCoordfromMinusOneToOne, MaxCameraOffset, PlayerLocation);



}

FVector UVoronoiCamMaths::MiddlePointVector(FVector & PlayerZero, FVector & PlayerOne)
{
	FVector additionVector = PlayerZero + PlayerOne;
	additionVector = additionVector / 2;
	return additionVector;
}

