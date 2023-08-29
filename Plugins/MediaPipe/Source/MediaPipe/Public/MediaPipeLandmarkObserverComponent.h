// (c) wongfei - https://github.com/wongfei
// 2021, All Rights Reserved. Apache License Version 2.0
// Modified and updated: YuriNK (ykasczc@gmail.com)

#pragma once

#include "MediaPipeObserverComponent.h"
#include "MediapipeTypes.h"
#include "MediaPipeLandmarkObserverComponent.generated.h"

/**
* Observer for output stream containing landmarks
*/
UCLASS(ClassGroup="MediaPipe", meta=(BlueprintSpawnableComponent))
class MEDIAPIPE_API UMediaPipeLandmarkObserverComponent : public UMediaPipeObserverComponent
{
	GENERATED_BODY()

public:
	UMediaPipeLandmarkObserverComponent();

	// Config

	// Mediapile data type:
	// Landmark is a vector in 3D space
	// NormalizedLandmark: x and y are coordinates of the input image (normalized to 0..1) and z is a corresponding value ("height")
	// Use Multi for array of landmarks
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MediaPipe|Landmarks")
	EMediaPipeLandmarkListType LandmarkListType = EMediaPipeLandmarkListType::MultiNormalizedLandmarkList;

	// Convert world orientation of Mediapipe to Unreal world
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MediaPipe|Landmarks")
	EMediaPipeLandmarkAxisMapping AxisX = EMediaPipeLandmarkAxisMapping::Z;

	// Convert world orientation of Mediapipe to Unreal world
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MediaPipe|Landmarks")
	EMediaPipeLandmarkAxisMapping AxisY = EMediaPipeLandmarkAxisMapping::X;

	// Convert world orientation of Mediapipe to Unreal world
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MediaPipe|Landmarks")
	EMediaPipeLandmarkAxisMapping AxisZ = EMediaPipeLandmarkAxisMapping::NegY;

	// Convert world scale of Mediapipe to Unreal world
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MediaPipe|Landmarks")
	FVector WorldScale = FVector::OneVector;

	// Minimal landmark visibility to accept it as detected.
	// Visibility is in the range of [0..1]	and after user-applied sigmoid denotes the probability that
	// this keypoint is located within the frame and not occluded by another bigger body part or another object.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MediaPipe|Landmarks")
	float MinVisibility = -1;

	// Minimal landmark presence to accept it as detected.
	// Presence is in the range of [0..1] and after user-applied sigmoid denotes the probability that a keypoint is located within the frame.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MediaPipe|Landmarks")
	float MinPresence = -1;

	// Getters

	UFUNCTION(BlueprintPure, Category = "MediaPipe")
	const TArray<FMediaPipeLandmark>& GetLandmarkList(int ObjectId);

	UFUNCTION(BlueprintPure, Category = "MediaPipe")
	const FMediaPipeLandmark& GetLandmark(int ObjectId, int LandmarkId);

	UFUNCTION(BlueprintPure, Category = "MediaPipe")
	bool TryGetLandmarkList(int ObjectId, TArray<FMediaPipeLandmark>& LandmarkList);

	UFUNCTION(BlueprintPure, Category = "MediaPipe")
	bool TryGetLandmark(int ObjectId, int LandmarkId, FMediaPipeLandmark& Landmark);

	// Utils

	UFUNCTION(BlueprintCallable, Category = "MediaPipe")
	void DrawDebugLandmarks(int ObjectId, const FTransform& Transform, float PrimitiveScale = 1.0f, FLinearColor Color = FLinearColor(0, 1, 0, 1));

protected:
	void OnUmpPacket(class IUmpObserver* Observer) override;

	// ObjectId -> Landmarks
	TArray< TArray<FMediaPipeLandmark> > MultiLandmarks;
};
