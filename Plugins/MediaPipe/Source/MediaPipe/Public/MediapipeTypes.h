// (c) wongfei - https://github.com/wongfei
// 2021, All Rights Reserved. Apache License Version 2.0
// Modified and updated: YuriNK (ykasczc@gmail.com)

#pragma once

#include "CoreMinimal.h"
#include "MediapipeTypes.generated.h"

UENUM(Blueprintable)
enum class EMediaPipeLandmarkListType : uint8
{
	LandmarkList = 0,
	NormalizedLandmarkList,
	MultiLandmarkList,
	MultiNormalizedLandmarkList
};

UENUM(Blueprintable)
enum class EMediaPipeLandmarkAxisMapping : uint8
{
	X = 0,
	Y,
	Z,
	NegX,
	NegY,
	NegZ
};

/* uint16 isn't supported for enums exposed to blueprints, unfortunately */
UENUM(BlueprintType)
enum class ECV_VideoCaptureAPI : uint8
{
	CAP_ANY,
	CAP_VFW,
	CAP_V4L,
	CAP_V4L2,
	CAP_FIREWIRE,
	CAP_FIREWARE,
	CAP_IEEE1394,
	CAP_DC1394,
	CAP_CMU1394,
	CAP_QT,
	CAP_UNICAP,
	CAP_DSHOW,
	CAP_PVAPI,
	CAP_OPENNI,
	CAP_OPENNI_ASUS,
	CAP_ANDROID,
	CAP_XIAPI,
	CAP_AVFOUNDATION,
	CAP_GIGANETIX,
	CAP_MSMF,
	CAP_WINRT,
	CAP_INTELPERC,
	CAP_OPENNI2,
	CAP_OPENNI2_ASUS,
	CAP_GPHOTO2,
	CAP_GSTREAMER,
	CAP_FFMPEG,
	CAP_IMAGES,
	CAP_ARAVIS,
	CAP_OPENCV_MJPEG,
	CAP_INTEL_MFX,
	CAP_XINE,
	CAP_MAX					UMETA(Hidden)
};

UENUM()
enum class ECV_VideoCaptureAPI_ToInt : uint16
{
	CAP_ANY = 0,
	CAP_VFW = 200,
	CAP_V4L = 200,
	CAP_V4L2 = CAP_V4L,
	CAP_FIREWIRE = 300,
	CAP_FIREWARE = CAP_FIREWIRE,
	CAP_IEEE1394 = CAP_FIREWIRE,
	CAP_DC1394 = CAP_FIREWIRE,
	CAP_CMU1394 = CAP_FIREWIRE,
	CAP_QT = 500,
	CAP_UNICAP = 600,
	CAP_DSHOW = 700,
	CAP_PVAPI = 800,
	CAP_OPENNI = 900,
	CAP_OPENNI_ASUS = 910,
	CAP_ANDROID = 1000,
	CAP_XIAPI = 1100,
	CAP_AVFOUNDATION = 1200,
	CAP_GIGANETIX = 1300,
	CAP_MSMF = 1400,
	CAP_WINRT = 1410,
	CAP_INTELPERC = 1500,
	CAP_OPENNI2 = 1600,
	CAP_OPENNI2_ASUS = 1610,
	CAP_GPHOTO2 = 1700,
	CAP_GSTREAMER = 1800,
	CAP_FFMPEG = 1900,
	CAP_IMAGES = 2000,
	CAP_ARAVIS = 2100,
	CAP_OPENCV_MJPEG = 2200,
	CAP_INTEL_MFX = 2300,
	CAP_XINE = 2400
};

USTRUCT(BlueprintType)
struct FMediaPipeLandmark
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Mediapipe Landmark")
	FVector Pos = FVector::ZeroVector;

	/**
	* Probability that this keypoint is located within the frame and not occluded by another bigger body part or another object.
	*/
	UPROPERTY(BlueprintReadOnly, Category = "Mediapipe Landmark")
	float Visibility = 0.f;

	/**
	* Probability that a keypoint is located within the frame.
	*/
	UPROPERTY(BlueprintReadOnly, Category = "Mediapipe Landmark")
	float Presence = 0.f;
};


// classification.proto

USTRUCT(BlueprintType)
struct FMediaPipeClassification
{
	GENERATED_BODY()

	// Not Object ID
	UPROPERTY(BlueprintReadOnly, Category = "Mediapipe Classification")
	int Index;

	UPROPERTY(BlueprintReadOnly, Category = "Mediapipe Classification")
	float Score;

	// aka CategoryName
	UPROPERTY(BlueprintReadOnly, Category = "Mediapipe Classification")
	FString Label;

	UPROPERTY(BlueprintReadOnly, Category = "Mediapipe Classification")
	FString DisplayName;
};
