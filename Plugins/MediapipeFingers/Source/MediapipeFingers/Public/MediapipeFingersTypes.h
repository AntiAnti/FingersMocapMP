// (c) Yuri Kalinin, 2023. All Rights Reserved
// ykasczc@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "FingersTypes.h"
#include "Animation/AnimSequence.h"
#include "MediapipeFingersTypes.generated.h"

UENUM(Blueprintable)
enum class EMediaPipeCaptureMode : uint8
{
	None,
	Preview,
	Capture,
	Play
};

UENUM(Blueprintable)
enum class EMediaPipeBlendType : uint8
{
	Linear,
	Sinusoid,
	FloatCurve
};

UENUM(Blueprintable)
enum class EMediaPipeAnimationMode : uint8
{
	Sequence,
	KeyFrames
};

USTRUCT(BlueprintType)
struct FAnimSequenceDataFingers
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Fingers Animation")
	FFingersRawPreset Right;

	UPROPERTY(BlueprintReadOnly, Category = "Fingers Animation")
	FFingersRawPreset Left;

	UPROPERTY(BlueprintReadOnly, Category = "Fingers Animation")
	bool bEvaluatedRight = false;
	UPROPERTY(BlueprintReadOnly, Category = "Fingers Animation")
	bool bEvaluatedLeft = false;

	UPROPERTY(BlueprintReadOnly, Category = "Fingers Animation")
	FFingersRawPreset CachedRight;

	UPROPERTY(BlueprintReadOnly, Category = "Fingers Animation")
	FFingersRawPreset CachedLeft;

	UPROPERTY(BlueprintReadOnly, Category = "Fingers Animation")
	bool bEvaluatedCachedRight = false;
	UPROPERTY(BlueprintReadOnly, Category = "Fingers Animation")
	bool bEvaluatedCachedLeft = false;
};

USTRUCT(BlueprintType)
struct FFingersKeyframe
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Fingers Animation")
	int64 Frame;

	UPROPERTY(BlueprintReadOnly, Category = "Fingers Animation")
	FFingersRawPreset Right;

	UPROPERTY(BlueprintReadOnly, Category = "Fingers Animation")
	FFingersRawPreset Left;

	UPROPERTY(BlueprintReadOnly, Category = "Fingers Animation")
	bool bEvaluatedRight = false;

	UPROPERTY(BlueprintReadOnly, Category = "Fingers Animation")
	bool bEvaluatedLeft = false;

	// Blend from current to the next point
	UPROPERTY(BlueprintReadWrite, Category = "Fingers Animation")
	EMediaPipeBlendType BlendType;

	bool operator<(const FFingersKeyframe& Other) const
	{
		return Frame < Other.Frame;
	}
};

USTRUCT(BlueprintType)
struct FAnimCaptureInterval
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Anim Capture Interval")
	int32 StartFrame = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "Anim Capture Interval")
	int32 EndFrame = INDEX_NONE;

	bool IsEvaluated() const { return EndFrame > StartFrame; }
	FAnimCaptureInterval() {}
	FAnimCaptureInterval(int32 Start, int32 End)
		: StartFrame(Start), EndFrame(End)
	{}

	bool operator<(const FAnimCaptureInterval& Other) const
	{
		return StartFrame < Other.StartFrame;
	}
};

USTRUCT(BlueprintType)
struct FHandsFilter
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Hand Filter")
	int64 FrameIntervalStart = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "Hand Filter")
	int64 FrameIntervalEnd = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "Hand Filter")
	bool bEnabledRight = true;

	UPROPERTY(BlueprintReadOnly, Category = "Hand Filter")
	TMap<EFingerName, bool> RightFingers;

	UPROPERTY(BlueprintReadOnly, Category = "Hand Filter")
	bool bEnabledLeft = true;

	UPROPERTY(BlueprintReadOnly, Category = "Hand Filter")
	TMap<EFingerName, bool> LeftFingers;

	FHandsFilter()
	{
		RightFingers = { {EFingerName::FN_Index, true}, {EFingerName::FN_Middle, true}, {EFingerName::FN_Pinky, true}, {EFingerName::FN_Ring, true}, {EFingerName::FN_Thumb, true} };
		LeftFingers = { {EFingerName::FN_Index, true}, {EFingerName::FN_Middle, true}, {EFingerName::FN_Pinky, true}, {EFingerName::FN_Ring, true}, {EFingerName::FN_Thumb, true} };
	}

	int64 GetIntervalStart() const
	{
		return (FrameIntervalStart < 0 || FrameIntervalStart > FrameIntervalEnd) ? 0 : FrameIntervalStart;
	}
	int64 GetIntervalEnd(UAnimSequence* Sequence) const
	{
		return (FrameIntervalEnd > FrameIntervalStart) ? FrameIntervalEnd : Sequence->GetNumberOfSampledKeys() - 1;
	}
};