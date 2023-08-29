// (c) Yuri Kalinin, 2023. All Rights Reserved
// ykasczc@gmail.com

#include "FingerDataExtractorComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/SkinnedAsset.h"
#include "ReferenceSkeleton.h"
#include "Animation/AnimSequence.h"
#include "Engine/World.h"
#include "MediaPipeObserverComponent.h"
#include "MediaPipeLandmarkObserverComponent.h"
#include "MediaPipeClassificationObserverComponent.h"
#include "HeadMountedDisplayTypes.h"
#include "Kismet/KismetMathLibrary.h"
#include "IXRTrackingSystem.h"
#include "FingersTypes.h"
#include "VRHandsFunctionLibrary.h"
#include "Curves/CurveFloat.h"
#include "MediapipeTypes.h"
#include "MediapipeFingersTypes.h"
#include "Components/SkeletalMeshComponent.h"
#include "FingersSolverSetup.h"

#include "Animation/AnimData/AnimDataModel.h"
#include "FingersFKIKSolver.h"
#include "Animation/AnimData/IAnimationDataController.h"

#define __initialized (IsValid(Observer) && IsValid(Classificator))
#define __currentFrame (CaptureStartFrame + GetFrameAtTime((GetWorld()->GetTimeSeconds() - CaptureStartSystemTime) * AnimationPlayRate))
#define __declareFingersArr(fr, side) { { EFingerName::FN_Index, &Animation[fr].side.Index}, { EFingerName::FN_Middle, &Animation[fr].side.Middle }, { EFingerName::FN_Ring, &Animation[fr].side.Ring }, { EFingerName::FN_Pinky, &Animation[fr].side.Pinky }, { EFingerName::FN_Thumb, &Animation[fr].side.Thumb } }
#define __declareFingersKFArr(index, side) { { EFingerName::FN_Index, &KeyFrames[index].side.Index}, { EFingerName::FN_Middle, &KeyFrames[index].side.Middle }, { EFingerName::FN_Ring, &KeyFrames[index].side.Ring }, { EFingerName::FN_Pinky, &KeyFrames[index].side.Pinky }, { EFingerName::FN_Thumb, &KeyFrames[index].side.Thumb } }

// Sets default values for this component's properties
UFingerDataExtractorComponent::UFingerDataExtractorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

// Called when the game starts
void UFingerDataExtractorComponent::BeginPlay()
{
	Super::BeginPlay();

	TArray<UMediaPipeObserverComponent*> Components;
	GetOwner()->GetComponents<UMediaPipeObserverComponent>(Components);

	for (auto Comp : Components)
	{
		if (Comp->GetFName() == PoseObserverName || Comp->GetReadableName() == PoseObserverName.ToString())
		{
			if (UMediaPipeLandmarkObserverComponent* CastedObserver = Cast<UMediaPipeLandmarkObserverComponent>(Comp))
			{
				Observer = CastedObserver;
				AddTickPrerequisiteComponent(Observer);

				UE_LOG(LogTemp, Log, TEXT("FingerDataExtractor: Connecting observer %s"), *Observer->GetName());
			}
		}
		if (Comp->GetFName() == HandClassificatorName || Comp->GetReadableName() == HandClassificatorName.ToString())
		{
			if (UMediaPipeClassificationObserverComponent* CastedObserver = Cast<UMediaPipeClassificationObserverComponent>(Comp))
			{
				Classificator = CastedObserver;
				AddTickPrerequisiteComponent(Classificator);

				UE_LOG(LogTemp, Log, TEXT("FingerDataExtractor: Connecting classificator %s"), *Classificator->GetName());
			}
		}
	}

	TArray<USkeletalMeshComponent*> BodyComponents;
	GetOwner()->GetComponents<USkeletalMeshComponent>(BodyComponents);
	for (auto Comp : BodyComponents)
	{
		if (Comp->GetFName() == SkeletalMeshName || Comp->GetReadableName() == SkeletalMeshName.ToString())
		{
			BodyMesh = Comp;
		}
	}

	if (IsValid(BodyMesh) && IsValid(BodyMesh->GetSkinnedAsset()) && IsValid(RightHandSetup) && IsValid(LeftHandSetup))
	{
		const FReferenceSkeleton& RefSkeleton = BodyMesh->GetSkinnedAsset()->GetRefSkeleton();
		int32 Bone1 = RefSkeleton.FindBoneIndex(RightHandSetup->Fingers[EFingerName::FN_Index].TipBoneName);
		int32 Bone2 = RefSkeleton.FindBoneIndex(RightHandSetup->Fingers[EFingerName::FN_Ring].TipBoneName);

		while (Bone1 != Bone2)
		{
			Bone1 = RefSkeleton.GetParentIndex(Bone1);
			Bone2 = RefSkeleton.GetParentIndex(Bone2);
		}
		HandBoneRight = RefSkeleton.GetBoneName(Bone1);

		Bone1 = RefSkeleton.FindBoneIndex(LeftHandSetup->Fingers[EFingerName::FN_Index].TipBoneName);
		Bone2 = RefSkeleton.FindBoneIndex(LeftHandSetup->Fingers[EFingerName::FN_Ring].TipBoneName);

		while (Bone1 != Bone2)
		{
			Bone1 = RefSkeleton.GetParentIndex(Bone1);
			Bone2 = RefSkeleton.GetParentIndex(Bone2);
		}
		HandBoneLeft = RefSkeleton.GetBoneName(Bone1);
	}	

	SmoothValuesRight.SetNum(SimpleSmooth);
	SmoothValuesLeft.SetNum(SimpleSmooth);

	// Generate reference poses
	if (IsValid(RightHandSetup) && IsValid(LeftHandSetup) && IsValid(BodyMesh))
	{
		UFingersFKIKSolver* RightSolver = UFingersFKIKSolver::CreateFingersFKIKSolver(RightHandSetup, BodyMesh);
		UFingersFKIKSolver* LeftSolver = UFingersFKIKSolver::CreateFingersFKIKSolver(LeftHandSetup, BodyMesh);

		RightSolver->GenerateRawReferencePose(HandReferencePoseRight);
		LeftSolver->GenerateRawReferencePose(HandReferencePoseLeft);

		RightSolver->MarkAsGarbage();
		LeftSolver->MarkAsGarbage();

		UE_LOG(LogTemp, Log, TEXT("HandReferencePose created successfully"));
	}
}

// Called every frame
void UFingerDataExtractorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	bHasFrameData = false;
	if (!__initialized) return;

	// No any mode? Show pose for the current start frame cursor.
	if (Mode == EMediaPipeCaptureMode::None)
	{
		if (AnimationMode == EMediaPipeAnimationMode::Sequence)
		{
			if (Animation.IsValidIndex(CaptureStartFrame))
			{
				if (Animation[CaptureStartFrame].bEvaluatedRight)
				{
					PreviewStateRight = Animation[CaptureStartFrame].Right;
					bHasFrameData = true;
				}
				if (Animation[CaptureStartFrame].bEvaluatedLeft)
				{
					PreviewStateLeft = Animation[CaptureStartFrame].Left;
					bHasFrameData = true;
				}
			}
		}
		else
		{
			float CurrentTime = GetTimeAtFrame(CaptureStartFrame);
			SeekToTime(CurrentTime);
			bHasFrameData = ShowKeyFrame(CurrentTime, PreviewStateRight, PreviewStateLeft);
		}

		return;
	}
	// Playing? Show pose for the current frame
	else if (Mode == EMediaPipeCaptureMode::Play)
	{
		// this is an offset from start time
		float CurrentTime = (GetWorld()->GetTimeSeconds() - CaptureStartSystemTime) * AnimationPlayRate;
		// start frame + offset in times
		const int64 CurrentFrame = PlayStartFrame + GetFrameAtTime(CurrentTime);
		// real play time
		CurrentTime += GetTimeAtFrame(PlayStartFrame);
		OnTime.Broadcast(CurrentTime, CurrentFrame);

		// play from captured sequence
		if (AnimationMode == EMediaPipeAnimationMode::Sequence)
		{
			if (Animation[CurrentFrame].bEvaluatedRight)
			{
				PreviewStateRight = Animation[CurrentFrame].Right;
				bHasFrameData = true;
			}
			if (Animation[CurrentFrame].bEvaluatedLeft)
			{
				PreviewStateLeft = Animation[CurrentFrame].Left;
				bHasFrameData = true;
			}
		}
		// playing keyframed animation
		else if (AnimationMode == EMediaPipeAnimationMode::KeyFrames)
		{
			SeekToTime(CurrentTime);
			bHasFrameData = ShowKeyFrame(CurrentTime, PreviewStateRight, PreviewStateLeft);
		}

		if (CurrentFrame + 1 == Animation.Num())
		{
			Stop();
		}

		return;
	}

	// For preview or capture mode we need to collect data from Mediapipe
	// This function updates PreviewStateRight and PreviewStateLeft
	CaptureFrame(DeltaTime);

	// Caputing data?
	if (Mode == EMediaPipeCaptureMode::Capture)
	{
		int64 CurrentFrame = __currentFrame;
		OnTime.Broadcast(GetTimeAtFrame(CurrentFrame), CurrentFrame);

		// save previous values to cache for futore blending (if needed)
		if (bCaptureHandRight)
		{
			if (Animation[CurrentFrame].bEvaluatedRight)
			{
				Animation[CurrentFrame].CachedRight = Animation[CurrentFrame].Right;
				Animation[CurrentFrame].bEvaluatedCachedRight = true;
			}
			Animation[CurrentFrame].Right = PreviewStateRight;
			Animation[CurrentFrame].bEvaluatedRight = true;
		}
		if (bCaptureHandLeft)
		{
			if (Animation[CurrentFrame].bEvaluatedLeft)
			{
				Animation[CurrentFrame].CachedLeft = Animation[CurrentFrame].Left;
				Animation[CurrentFrame].bEvaluatedCachedLeft = true;
			}
			Animation[CurrentFrame].Left = PreviewStateLeft;
			Animation[CurrentFrame].bEvaluatedLeft = true;
		}

		if (CurrentFrame + 1 == Animation.Num())
		{
			Stop();
		}
	}
}

void UFingerDataExtractorComponent::CaptureFrame(float DeltaTime)
{
	TArray<FMediaPipeLandmark> LandmarksRight, LandmarksLeft;
	int32 HandsNum = Observer->GetNumDetections();
	int32 RightObjectId = INDEX_NONE, LeftObjectId = INDEX_NONE;

	if (LastUpdateTime < Observer->LastUpdate || DeltaTime < 0.f)
	{
		LastUpdateTime = Observer->LastUpdate;

		// each hand is a separate object
		// within object, there is one classificator with hand name as label

		for (int32 ObjectId = 0; ObjectId < HandsNum; ObjectId++)
		{
			TArray<FMediaPipeClassification> Classifications;
			Classificator->TryGetClassificationList(ObjectId, Classifications);

			for (const auto& Classification : Classifications)
			{
				if (Classification.Label.Equals(TEXT("right"), ESearchCase::IgnoreCase))
				{
					//UE_LOG(LogTemp, Log, TEXT("right = %d"), ObjectId);
					RightObjectId = ObjectId;
				}
				else if (Classification.Label.Equals(TEXT("left"), ESearchCase::IgnoreCase))
				{
					LeftObjectId = ObjectId;
				}
				if (RightObjectId != INDEX_NONE && LeftObjectId != INDEX_NONE) break;
			}
		}

		Observer->TryGetLandmarkList(RightObjectId, LandmarksRight);
		Observer->TryGetLandmarkList(LeftObjectId, LandmarksLeft);
	}

	// Update raw poses
	FFingersRawPreset BlendedPoseRight = PreviewStateRight;
	FFingersRawPreset BlendedPoseLeft = PreviewStateLeft;

	if (RightObjectId != INDEX_NONE)
	{
		FFingersRawPreset NewPose;
		MediapipeDataToXR(true, LandmarksRight, CurrentHandStateRight);
		if (UVRHandsFunctionLibrary::ConvertOpenXRHandTrackingToFingersPose(CurrentHandStateRight, true, NewPose))
		{
			const FVector2f ClampIn = FVector2f(0.4f, 0.8f);
			const FVector2f ClampOut = FVector2f(0.f, 1.f);

			LastUpdatedStateRight = PreviewStateRight;
			float Alpha = FMath::GetMappedRangeValueClamped(ClampIn, ClampOut, GetMeanPresense(EFingerName::FN_Index, LandmarksRight));
			UVRHandsFunctionLibrary::BlendFingerRaw(LastUpdatedStateRight.Index, NewPose.Index, Alpha);
			Alpha = FMath::GetMappedRangeValueClamped(ClampIn, ClampOut, GetMeanPresense(EFingerName::FN_Middle, LandmarksRight));
			UVRHandsFunctionLibrary::BlendFingerRaw(LastUpdatedStateRight.Middle, NewPose.Middle, Alpha);
			Alpha = FMath::GetMappedRangeValueClamped(ClampIn, ClampOut, GetMeanPresense(EFingerName::FN_Ring, LandmarksRight));
			UVRHandsFunctionLibrary::BlendFingerRaw(LastUpdatedStateRight.Ring, NewPose.Ring, Alpha);
			Alpha = FMath::GetMappedRangeValueClamped(ClampIn, ClampOut, GetMeanPresense(EFingerName::FN_Pinky, LandmarksRight));
			UVRHandsFunctionLibrary::BlendFingerRaw(LastUpdatedStateRight.Pinky, NewPose.Pinky, Alpha);
			Alpha = FMath::GetMappedRangeValueClamped(ClampIn, ClampOut, GetMeanPresense(EFingerName::FN_Thumb, LandmarksRight));
			UVRHandsFunctionLibrary::BlendFingerRaw(LastUpdatedStateRight.Thumb, NewPose.Thumb, Alpha);
		}
	}
	if (LeftObjectId != INDEX_NONE)
	{
		FFingersRawPreset NewPose;
		MediapipeDataToXR(false, LandmarksLeft, CurrentHandStateLeft);
		if (UVRHandsFunctionLibrary::ConvertOpenXRHandTrackingToFingersPose(CurrentHandStateLeft, true, NewPose))
		{
			const FVector2f ClampIn = FVector2f(0.5f, 0.8f);
			const FVector2f ClampOut = FVector2f(0.f, 1.f);

			LastUpdatedStateLeft = PreviewStateLeft;
			float Alpha = FMath::GetMappedRangeValueClamped(ClampIn, ClampOut, GetMeanPresense(EFingerName::FN_Index, LandmarksLeft));
			UVRHandsFunctionLibrary::BlendFingerRaw(LastUpdatedStateLeft.Index, NewPose.Index, Alpha);
			Alpha = FMath::GetMappedRangeValueClamped(ClampIn, ClampOut, GetMeanPresense(EFingerName::FN_Middle, LandmarksLeft));
			UVRHandsFunctionLibrary::BlendFingerRaw(LastUpdatedStateLeft.Middle, NewPose.Middle, Alpha);
			Alpha = FMath::GetMappedRangeValueClamped(ClampIn, ClampOut, GetMeanPresense(EFingerName::FN_Ring, LandmarksLeft));
			UVRHandsFunctionLibrary::BlendFingerRaw(LastUpdatedStateLeft.Ring, NewPose.Ring, Alpha);
			Alpha = FMath::GetMappedRangeValueClamped(ClampIn, ClampOut, GetMeanPresense(EFingerName::FN_Pinky, LandmarksLeft));
			UVRHandsFunctionLibrary::BlendFingerRaw(LastUpdatedStateLeft.Pinky, NewPose.Pinky, Alpha);
			Alpha = FMath::GetMappedRangeValueClamped(ClampIn, ClampOut, GetMeanPresense(EFingerName::FN_Thumb, LandmarksLeft));
			UVRHandsFunctionLibrary::BlendFingerRaw(LastUpdatedStateLeft.Thumb, NewPose.Thumb, Alpha);
		}
	}

	if (DeltaTime < 0.f)
	{
		PreviewStateRight = LastUpdatedStateRight;
		PreviewStateLeft = LastUpdatedStateLeft;
	}
	else
	{
		SmoothValuesRight_Index = (SmoothValuesRight_Index + 1) % SimpleSmooth;
		SmoothValuesLeft_Index = (SmoothValuesLeft_Index + 1) % SimpleSmooth;

		if (InterpolationSpeed > 0.f)
		{
			UVRHandsFunctionLibrary::FingersPoseRawInterpTo(PreviewStateRight, LastUpdatedStateRight, DeltaTime, InterpolationSpeed, SmoothValuesRight[SmoothValuesRight_Index]);
			UVRHandsFunctionLibrary::FingersPoseRawInterpTo(PreviewStateLeft, LastUpdatedStateLeft, DeltaTime, InterpolationSpeed, SmoothValuesLeft[SmoothValuesLeft_Index]);
		}
		else
		{
			SmoothValuesRight[SmoothValuesRight_Index] = LastUpdatedStateRight;
			SmoothValuesLeft[SmoothValuesLeft_Index] = LastUpdatedStateLeft;
		}

		if (SimpleSmooth > 1)
		{
			PreviewStateRight = MeanFingersPreset(SmoothValuesRight);
			PreviewStateLeft = MeanFingersPreset(SmoothValuesLeft);
		}
		else
		{
			PreviewStateRight = SmoothValuesRight[SmoothValuesRight_Index];
			PreviewStateLeft = SmoothValuesLeft[SmoothValuesLeft_Index];
		}
	}

	bHasFrameData = true;
}

void UFingerDataExtractorComponent::CreateNewCaptureInterval(int64 FrameStart, int64 FrameEnd)
{
	const int32 BlendFrames = FMath::Min(
		GetFrameAtTime(BlendDuration),
		(FrameEnd - FrameStart) / 2
	);

	bool bShouldAddInterval = true;
	for (int32 i = 0; i < CaptureIntervals.Num(); i++)
	{
		// interval is inside
		if (CaptureIntervals[i].StartFrame >= FrameStart && CaptureIntervals[i].EndFrame <= FrameEnd)
		{
			CaptureIntervals.RemoveAt(i--);
		}
		// new interval is inside of existing
		else if (CaptureIntervals[i].StartFrame < FrameStart && CaptureIntervals[i].EndFrame > FrameEnd)
		{
			bShouldAddInterval = false;

			// blend new interval to existing
			BlendCapturedData(FrameStart, FrameStart + BlendFrames, true);
			BlendCapturedData(FrameEnd - BlendFrames, FrameEnd, false);
		}
		// overlaps at start
		else if (CaptureIntervals[i].StartFrame < FrameStart && CaptureIntervals[i].EndFrame >= FrameStart && CaptureIntervals[i].EndFrame <= FrameEnd)
		{
			bShouldAddInterval = false;

			BlendCapturedData(FrameStart, FrameStart + BlendFrames, true);
			CaptureIntervals[i].EndFrame = FrameEnd;
		}
		// overlaps at end
		else if (CaptureIntervals[i].StartFrame >= FrameStart && CaptureIntervals[i].StartFrame <= FrameEnd && CaptureIntervals[i].EndFrame > FrameEnd)
		{
			bShouldAddInterval = false;

			BlendCapturedData(FrameEnd - BlendFrames, FrameEnd, false);
			CaptureIntervals[i].StartFrame = FrameStart;
		}
	}

	if (bShouldAddInterval)
	{
		CaptureIntervals.Add(FAnimCaptureInterval(FrameStart, FrameEnd));
	}

	CaptureIntervals.Sort();
	OnAnimationUpdated.Broadcast();
}

void UFingerDataExtractorComponent::MediapipeDataToXR(bool bRightHand, const TArray<FMediaPipeLandmark>& InData, FXRMotionControllerData& HandDataXR) const
{
	const float Scale = 1.f;

	// shape output size
	if (HandDataXR.HandKeyRotations.Num() == 0)
	{
		HandDataXR.HandKeyPositions.Init(FVector::ZeroVector, EHandKeypointCount);
		HandDataXR.HandKeyRotations.Init(FQuat::Identity, EHandKeypointCount);
		HandDataXR.DeviceName = FName(TEXT("Mediapipe-") + FString(bRightHand ? "right" : "left"));
	}
	HandDataXR.bValid = false;

	const TArray<EHandKeypoint> IndexToKeypoint =
	{
		/* 0 */  EHandKeypoint::Wrist,
		/* 1 */  EHandKeypoint::ThumbMetacarpal, EHandKeypoint::ThumbProximal, EHandKeypoint::ThumbDistal, EHandKeypoint::ThumbTip,
		/* 5 */  EHandKeypoint::IndexProximal, EHandKeypoint::IndexIntermediate, EHandKeypoint::IndexDistal, EHandKeypoint::IndexTip,
		/* 9 */  EHandKeypoint::MiddleProximal, EHandKeypoint::MiddleIntermediate, EHandKeypoint::MiddleDistal, EHandKeypoint::MiddleTip,
		/* 13 */ EHandKeypoint::RingProximal, EHandKeypoint::RingIntermediate, EHandKeypoint::RingDistal, EHandKeypoint::RingTip,
		/* 17 */ EHandKeypoint::LittleProximal, EHandKeypoint::LittleIntermediate, EHandKeypoint::LittleDistal, EHandKeypoint::LittleTip
	};

	// parse coordinates
	for (int32 Index = 0; Index < InData.Num(); Index++)
	{
		HandDataXR.HandKeyPositions[(int32)IndexToKeypoint[Index]] = InData[Index].Pos * Scale;
	}

	// restore rotations

	// First, wrist rotation
	FVector WristRight = (HandDataXR.HandKeyPositions[(int32)IndexToKeypoint[17]] - HandDataXR.HandKeyPositions[(int32)IndexToKeypoint[5]])
		* (bRightHand ? 1. : -1.);
	FVector MidFingersPoint = (HandDataXR.HandKeyPositions[(int32)IndexToKeypoint[17]] + HandDataXR.HandKeyPositions[(int32)IndexToKeypoint[5]]) * 0.5f;
	FVector WristForward = MidFingersPoint - HandDataXR.HandKeyPositions[(int32)EHandKeypoint::Wrist];
	HandDataXR.HandKeyRotations[(int32)EHandKeypoint::Wrist] = UKismetMathLibrary::MakeRotFromXY(WristForward, WristRight).Quaternion();
	// copy to palm
	HandDataXR.HandKeyPositions[(int32)EHandKeypoint::Palm] = HandDataXR.HandKeyPositions[(int32)EHandKeypoint::Wrist];
	HandDataXR.HandKeyRotations[(int32)EHandKeypoint::Palm] = HandDataXR.HandKeyRotations[(int32)EHandKeypoint::Wrist];

	// and now fingers rotation
	ComputeFingerRotation(HandDataXR, (int32)EHandKeypoint::LittleTip);
	ComputeFingerRotation(HandDataXR, (int32)EHandKeypoint::RingTip);
	ComputeFingerRotation(HandDataXR, (int32)EHandKeypoint::MiddleTip);
	ComputeFingerRotation(HandDataXR, (int32)EHandKeypoint::IndexTip);
	ComputeFingerRotation(HandDataXR, (int32)EHandKeypoint::ThumbTip);

	HandDataXR.PalmPosition = HandDataXR.HandKeyPositions[(int32)EHandKeypoint::Wrist];
	HandDataXR.PalmRotation = HandDataXR.HandKeyRotations[(int32)EHandKeypoint::Wrist];

	HandDataXR.bValid = true;
}

FFingersRawPreset UFingerDataExtractorComponent::MeanFingersPreset(const TArray<FFingersRawPreset>& Addends) const
{
	FFingersRawPreset p;

	for (const auto& val : Addends)
	{
		MakeFingerSum(p.Index, val.Index);
		MakeFingerSum(p.Middle, val.Middle);
		MakeFingerSum(p.Ring, val.Ring);
		MakeFingerSum(p.Pinky, val.Pinky);
		MakeFingerSum(p.Thumb, val.Thumb);
	}

	float Mul = 1.f / (float)Addends.Num();
	MakeFingerMul(p.Index, Mul);
	MakeFingerMul(p.Middle, Mul);
	MakeFingerMul(p.Ring, Mul);
	MakeFingerMul(p.Pinky, Mul);
	MakeFingerMul(p.Thumb, Mul);

	return p;
}

void UFingerDataExtractorComponent::MakeFingerMul(FFingerRawRotation& InOutFinger, float Multiplier) const
{
	InOutFinger.FingerRoll *= Multiplier;
	InOutFinger.FingerYaw *= Multiplier;
	for (auto& Curl : InOutFinger.Curls)
	{
		Curl *= Multiplier;
	}
}

void UFingerDataExtractorComponent::MakeFingerSum(FFingerRawRotation& InOutFinger, const FFingerRawRotation& Addend) const
{
	InOutFinger.FingerRoll += Addend.FingerRoll;
	InOutFinger.FingerYaw += Addend.FingerYaw;

	if (InOutFinger.Curls.Num() < Addend.Curls.Num())
	{
		InOutFinger.Curls.Init(0.f, Addend.Curls.Num());
	}
	for (int32 i = 0; i < Addend.Curls.Num(); i++)
	{
		InOutFinger.Curls[i] += Addend.Curls[i];
	}
}

void UFingerDataExtractorComponent::SetAnimation(UAnimSequence* Sequence)
{
	if (IsValid(Sequence))
	{
		Animation.SetNum(Sequence->GetNumberOfSampledKeys());
		CurrentAnimation = Sequence;

		UE_LOG(LogTemp, Log, TEXT("Fingers. Animation initialized. Frames number = %d"), Animation.Num());

		if (Mode == EMediaPipeCaptureMode::Capture || Mode == EMediaPipeCaptureMode::Play)
		{
			CaptureStartSystemTime = GetWorld()->GetTimeSeconds();
		}
	}
}

bool UFingerDataExtractorComponent::Seek(int32 Frame)
{
	bool bResult = true;

	CaptureStartFrame = Frame;
	if (Mode != EMediaPipeCaptureMode::Play)
	{
		PlayStartFrame = Frame;
	}

	if (AnimationMode == EMediaPipeAnimationMode::Sequence)
	{
		bResult = (IsValid(CurrentAnimation) && Animation.IsValidIndex(Frame));
	}
	else if (AnimationMode == EMediaPipeAnimationMode::KeyFrames)
	{
		if (KeyFrames.Num() == 0)
		{
			PlayKeyframeIndex = INDEX_NONE;
			return false;
		}

		if (Frame < KeyFrames[0].Frame)
		{
			PlayKeyframeIndex = 0;
		}
		else if (Frame > KeyFrames.Last().Frame)
		{
			PlayKeyframeIndex = KeyFrames.Num() - 1;
		}
		else
		{
			PlayKeyframeIndex = KeyFrames.Num() / 2;
			int32 NextIndex = (PlayKeyframeIndex + 1) % KeyFrames.Num();
			int32 Step = PlayKeyframeIndex / 2;

			while (!(Frame >= KeyFrames[PlayKeyframeIndex].Frame && Frame <= KeyFrames[NextIndex].Frame))
			{
				if (KeyFrames[PlayKeyframeIndex].Frame > Frame)
				{
					PlayKeyframeIndex -= Step;
				}
				else if (KeyFrames[PlayKeyframeIndex].Frame < Frame)
				{
					PlayKeyframeIndex += Step;
				}
				Step = FMath::Max(1, Step / 2);

				PlayKeyframeIndex = FMath::Clamp(PlayKeyframeIndex, 0, KeyFrames.Num() - 2);
				NextIndex = (PlayKeyframeIndex + 1) % KeyFrames.Num();
			}

			return (KeyFrames.IsValidIndex(PlayKeyframeIndex));
		}
		//UE_LOG(LogTemp, Log, TEXT("[AnimFrame = %d --> KeyFrame ID = %d"), Frame, PlayKeyframeIndex);
	}

	return bResult;
}

bool UFingerDataExtractorComponent::SeekToTime(float Time)
{
	if (CurrentAnimation) return Seek(GetFrameAtTime(Time));
	return false;
}

void UFingerDataExtractorComponent::Stop()
{
	int32 CurrentFrame = __currentFrame;
	if (CurrentFrame >= Animation.Num())
	{
		CurrentFrame = Animation.Num() - 1;
	}

	if (Mode == EMediaPipeCaptureMode::Capture)
	{
		// save captured segment
		CreateNewCaptureInterval(CaptureStartFrame, CurrentFrame);		
	}
	else if (Mode == EMediaPipeCaptureMode::Play)
	{
		// do nothing
	}

	Mode = EMediaPipeCaptureMode::None;
	CaptureStartFrame = (CurrentFrame + 1) % Animation.Num();
	OnStop.Broadcast();
}

void UFingerDataExtractorComponent::Play(float StartTime)
{
	if (StartTime >= 0.f)
	{
		int32 Frame = GetFrameAtTime(StartTime);
		Seek(Frame);
	}
	PlayStartFrame = CaptureStartFrame;
	CaptureStartSystemTime = GetWorld()->GetTimeSeconds();
	Mode = EMediaPipeCaptureMode::Play;
	OnPlayStarted.Broadcast();
}

bool UFingerDataExtractorComponent::Capture(float StartTime)
{
	if (!IsValid(CurrentAnimation))
	{
		return false;
	}
	if (Mode == EMediaPipeCaptureMode::Capture)
	{
		return false;
	}
	if (StartTime >= 0.f)
	{
		SeekToTime(StartTime);
	}

	if (AnimationMode == EMediaPipeAnimationMode::Sequence)
	{
		CaptureStartSystemTime = GetWorld()->GetTimeSeconds();
		Mode = EMediaPipeCaptureMode::Capture;
		OnCaptureStarted.Broadcast();
	}
	else // AnimationMode == EMediaPipeAnimationMode::KeyFrames
	{
		CaptureFrame(-1.f);

		FFingersKeyframe NewFrame;
		if (bCaptureHandRight)
		{
			NewFrame.Right = PreviewStateRight;
			NewFrame.bEvaluatedRight = true;
		}
		if (bCaptureHandLeft)
		{
			NewFrame.Left = PreviewStateLeft;
			NewFrame.bEvaluatedLeft = true;
		}
		NewFrame.BlendType = BlendType;
		NewFrame.Frame = CaptureStartFrame;

		bool bUpdated = false;
		for (auto& f : KeyFrames)
		{
			if (f.Frame == CaptureStartFrame)
			{
				f = NewFrame;
				bUpdated = true;
			}
		}

		if (!bUpdated)
		{
			KeyFrames.Add(NewFrame);
			KeyFrames.Sort();
		}
		OnAnimationUpdated.Broadcast();
	}
	return true;
}

bool UFingerDataExtractorComponent::ShowKeyFrame(float CurrentTime, FFingersRawPreset& OutRight, FFingersRawPreset& OutLeft)
{
	bool bRetValue = bHasFrameData;
	int64 CurrentFrame = GetFrameAtTime(CurrentTime);

	if (KeyFrames.IsValidIndex(PlayKeyframeIndex))
	{
		int32 NextIndex = PlayKeyframeIndex + 1;
		if (KeyFrames.IsValidIndex(NextIndex) && CurrentFrame >= KeyFrames[NextIndex].Frame)
		{
			PlayKeyframeIndex++; NextIndex++;
		}

		if (PlayKeyframeIndex == 0 && CurrentFrame <= KeyFrames[0].Frame)
		{
			if (KeyFrames[0].bEvaluatedRight)
			{
				OutRight = KeyFrames[0].Right;
				bRetValue = true;
			}
			if (KeyFrames[0].bEvaluatedLeft)
			{
				OutLeft = KeyFrames[0].Left;
				bRetValue = true;
			}
		}
		else if (CurrentFrame >= KeyFrames.Last().Frame)
		{
			if (KeyFrames.Last().bEvaluatedRight)
			{
				OutRight = KeyFrames.Last().Right;
				bRetValue = true;
			}
			if (KeyFrames.Last().bEvaluatedLeft)
			{
				OutLeft = KeyFrames.Last().Left;
				bRetValue = true;
			}
		}
		else
		{
			GetFrameHandValue(CurrentTime, KeyFrames[PlayKeyframeIndex], KeyFrames[NextIndex], OutRight, OutLeft);
			bRetValue = true;
		}
	}

	return bRetValue;
}

float UFingerDataExtractorComponent::GetMeanPresense(EFingerName Finger, const TArray<FMediaPipeLandmark>& Data) const
{
	// hand pose landmarks don't have presense and visibility info
	return 1.f;

	TArray<int32> Indices;
	switch (Finger)
	{
		case EFingerName::FN_Index:
			Indices = { 5, 6, 7, 8 }; break;
		case EFingerName::FN_Middle:
			Indices = { 9, 10, 11, 12 }; break;
		case EFingerName::FN_Ring:
			Indices = { 13, 14, 15, 16 }; break;
		case EFingerName::FN_Pinky:
			Indices = { 17, 18, 19, 20 }; break;
		case EFingerName::FN_Thumb:
			Indices = { 1, 2, 3, 4 }; break;
		default: return 0.f;
	}

	if (Data.Num() <= Indices.Last())
	{
		return 0.f;
	}

	float Presense = 0.f;
	for (int32 Index : Indices)
	{
		Presense += Data[Index].Visibility;
	}
	//UE_LOG(LogTemp, Log, TEXT("Finger %d - presense = %f"), (int)Finger, Presense / 4.f);
	return (Presense / Indices.Num());
}

void UFingerDataExtractorComponent::SetPreviewMode(bool bEnabled)
{
	if (!__initialized) return;

	if (Mode == EMediaPipeCaptureMode::None || Mode == EMediaPipeCaptureMode::Preview)
	{
		Mode = bEnabled ? EMediaPipeCaptureMode::Preview : EMediaPipeCaptureMode::None;

		if (bEnabled)
		{
			SetComponentTickEnabled(true);
		}
	}
}

void UFingerDataExtractorComponent::SetAnimationMode(EMediaPipeAnimationMode NewMode)
{
	AnimationMode = NewMode;
}

float UFingerDataExtractorComponent::GetPlayPosition() const
{
	float Offset = (Mode == EMediaPipeCaptureMode::Capture || Mode == EMediaPipeCaptureMode::Play)
		? (GetWorld()->GetTimeSeconds() - CaptureStartSystemTime) * AnimationPlayRate
		: 0.f;

	return GetTimeAtFrame(PlayStartFrame) + Offset;
}

void UFingerDataExtractorComponent::ComputeFingerRotation(FXRMotionControllerData& InOutData, const int32 TipIndex) const
{
	const int32 StartIndex = TipIndex - 3;
	const FVector Start = InOutData.HandKeyPositions[TipIndex - 3];
	const FVector End = InOutData.HandKeyPositions[TipIndex];
	const FVector MidJoint = InOutData.HandKeyPositions[TipIndex - 2];

	FVector WristPos = InOutData.HandKeyPositions[(int32)EHandKeypoint::Wrist];
	FQuat WristRot = InOutData.HandKeyRotations[(int32)EHandKeypoint::Wrist];

	float dp = FVector::DotProduct((End - Start).GetSafeNormal(), (MidJoint - Start).GetSafeNormal());
	FVector FingerRightDirection;
	if (dp > 0.9f)
	{
		// vRight = vUp ^ vForward
		float Alpha = FMath::Min(1.f, (dp - 0.9f) * 20.f);
		// right vector
		FVector NormalA = (MidJoint - Start).GetSafeNormal() ^ (End - Start).GetSafeNormal();
		FVector NormalB = WristRot.GetUpVector() ^ (End - Start).GetSafeNormal();
		FingerRightDirection = FMath::Lerp(NormalA, NormalB, Alpha);
	}
	else
	{
		FingerRightDirection = (MidJoint - Start).GetSafeNormal() ^ (End - Start).GetSafeNormal();
	}

	for (int32 Index = StartIndex; Index < TipIndex; Index++)
	{
		FVector Forward = InOutData.HandKeyPositions[Index + 1] - InOutData.HandKeyPositions[Index];
		InOutData.HandKeyRotations[Index] = UKismetMathLibrary::MakeRotFromXY(Forward, FingerRightDirection).Quaternion();
	}
	InOutData.HandKeyRotations[TipIndex] = InOutData.HandKeyRotations[TipIndex - 1];
}

FFingerRawRotation UFingerDataExtractorComponent::BlendFinger(const FFingerRawRotation& A, const FFingerRawRotation& B, float Alpha) const
{
	return BlendFinger(A, B, Alpha, BlendType);
}

float UFingerDataExtractorComponent::GetAlphaByBlendType(float InAlpha, EMediaPipeBlendType InBlendType) const
{
	float Alpha = FMath::Clamp(InAlpha, 0.f, 1.f);

	switch (InBlendType)
	{
		case EMediaPipeBlendType::FloatCurve:
			if (IsValid(BlendCurve))
			{
				Alpha = BlendCurve->GetFloatValue(Alpha);
				break;
			}
			// for invalid blend curve, use linear
		case EMediaPipeBlendType::Linear:
			// do nothing, use default alpha
			break;
		case EMediaPipeBlendType::Sinusoid:
			Alpha = FMath::Sin((Alpha * PI) - PI * 0.5f);
			break;
	}

	return Alpha;
}

FFingerRawRotation UFingerDataExtractorComponent::BlendFinger(const FFingerRawRotation& A, const FFingerRawRotation& B, float Alpha, EMediaPipeBlendType InBlendType) const
{
	FFingerRawRotation ret = A;
	UVRHandsFunctionLibrary::BlendFingerRaw(ret, B, GetAlphaByBlendType(Alpha, InBlendType));
	return ret;
}

void UFingerDataExtractorComponent::GetFrameHandValue(float Time, const FFingersKeyframe& A, const FFingersKeyframe& B, FFingersRawPreset& OutRight, FFingersRawPreset& OutLeft) const
{
	float AFrameTime = GetTimeAtFrame(A.Frame);
	float BFrameTime = GetTimeAtFrame(B.Frame);

	float Alpha = (Time - AFrameTime) / (BFrameTime - AFrameTime);
	Alpha = FMath::Clamp(Alpha, 0.f, 1.f);

	OutRight = UVRHandsFunctionLibrary::BlendFingersPoseRaw(A.Right, B.Right, GetAlphaByBlendType(Alpha, A.BlendType));
	OutLeft = UVRHandsFunctionLibrary::BlendFingersPoseRaw(A.Left, B.Left, GetAlphaByBlendType(Alpha, A.BlendType));
	/*
	OutRight.Index = BlendFinger(A.Right.Index, B.Right.Index, Alpha, A.BlendType);
	OutRight.Middle = BlendFinger(A.Right.Middle, B.Right.Middle, Alpha, A.BlendType);
	OutRight.Ring = BlendFinger(A.Right.Ring, B.Right.Ring, Alpha, A.BlendType);
	OutRight.Pinky = BlendFinger(A.Right.Pinky, B.Right.Pinky, Alpha, A.BlendType);
	OutRight.Thumb = BlendFinger(A.Right.Thumb, B.Right.Thumb, Alpha, A.BlendType);

	OutLeft.Index = BlendFinger(A.Left.Index, B.Left.Index, Alpha, A.BlendType);
	OutLeft.Middle = BlendFinger(A.Left.Middle, B.Left.Middle, Alpha, A.BlendType);
	OutLeft.Ring = BlendFinger(A.Left.Ring, B.Left.Ring, Alpha, A.BlendType);
	OutLeft.Pinky = BlendFinger(A.Left.Pinky, B.Left.Pinky, Alpha, A.BlendType);
	OutLeft.Thumb = BlendFinger(A.Left.Thumb, B.Left.Thumb, Alpha, A.BlendType);
	*/
}

void UFingerDataExtractorComponent::BlendCapturedData(int64 From, int64 To, bool bForwardDirection)
{
	if (!Animation.IsValidIndex(From) || !Animation.IsValidIndex(To) || To < From)
	{
		return;
	}

	const float Dist = (float)(To - From);
	for (int64 i = From; i <= To; i++)
	{
		FAnimSequenceDataFingers& CurrentPose = Animation[i];

		float Alpha = (float)(i - From) / Dist;
		if (!bForwardDirection)
		{
			Alpha = 1.f - Alpha;
		}

		if (CurrentPose.bEvaluatedCachedRight && bCaptureHandRight)
		{
			CurrentPose.Right.Index = BlendFinger(CurrentPose.CachedRight.Index, CurrentPose.Right.Index, Alpha);
			CurrentPose.Right.Middle = BlendFinger(CurrentPose.CachedRight.Middle, CurrentPose.Right.Middle, Alpha);
			CurrentPose.Right.Ring = BlendFinger(CurrentPose.CachedRight.Ring, CurrentPose.Right.Ring, Alpha);
			CurrentPose.Right.Pinky = BlendFinger(CurrentPose.CachedRight.Pinky, CurrentPose.Right.Pinky, Alpha);
			CurrentPose.Right.Thumb = BlendFinger(CurrentPose.CachedRight.Thumb, CurrentPose.Right.Thumb, Alpha);
			CurrentPose.bEvaluatedCachedRight = false;
		}

		if (CurrentPose.bEvaluatedCachedLeft && bCaptureHandLeft)
		{
			CurrentPose.Left.Index = BlendFinger(CurrentPose.CachedLeft.Index, CurrentPose.Left.Index, Alpha);
			CurrentPose.Left.Middle = BlendFinger(CurrentPose.CachedLeft.Middle, CurrentPose.Left.Middle, Alpha);
			CurrentPose.Left.Ring = BlendFinger(CurrentPose.CachedLeft.Ring, CurrentPose.Left.Ring, Alpha);
			CurrentPose.Left.Pinky = BlendFinger(CurrentPose.CachedLeft.Pinky, CurrentPose.Left.Pinky, Alpha);
			CurrentPose.Left.Thumb = BlendFinger(CurrentPose.CachedLeft.Thumb, CurrentPose.Left.Thumb, Alpha);
			CurrentPose.bEvaluatedCachedLeft = false;
		}
	}
}

int32 UFingerDataExtractorComponent::GetFrameAtTime(const float Time) const
{
	return FMath::Clamp(CurrentAnimation->GetSamplingFrameRate().AsFrameTime(Time).RoundToFrame().Value, 0, CurrentAnimation->GetNumberOfSampledKeys() - 1);
}

float UFingerDataExtractorComponent::GetTimeAtFrame(const int32 Frame) const
{
	return FMath::Clamp((float)CurrentAnimation->GetSamplingFrameRate().AsSeconds(Frame), 0.f, CurrentAnimation->GetPlayLength());
}

void UFingerDataExtractorComponent::GetAnimationIntervals(TArray<FAnimCaptureInterval>& OutIntervals) const
{
	OutIntervals = CaptureIntervals;
}

void UFingerDataExtractorComponent::GetAnimationData(TArray<FAnimSequenceDataFingers>& OutAnimation) const
{
	OutAnimation = Animation;
}

void UFingerDataExtractorComponent::DeleteAnimationInterval(int64 StartFrame)
{
	for (int32 i = 0; i < CaptureIntervals.Num(); i++)
	{
		if (CaptureIntervals[i].StartFrame == StartFrame)
		{
			CaptureIntervals.RemoveAt(i);
			OnAnimationUpdated.Broadcast();
			return;
		}
	}
}

void UFingerDataExtractorComponent::DeleteAnimationIntervalByIndex(int32 Index)
{
	if (CaptureIntervals.IsValidIndex(Index))
	{
		CaptureIntervals.RemoveAt(Index);
		OnAnimationUpdated.Broadcast();
	}
}

void UFingerDataExtractorComponent::GetAnimationKeyFrames(TArray<FFingersKeyframe>& OutKeys) const
{
	OutKeys = KeyFrames;
}

void UFingerDataExtractorComponent::UpdateKeyframeTime(int32 KeyFrameIndex, float NewTime)
{
	if (KeyFrames.IsValidIndex(KeyFrameIndex))
	{
		if (NewTime >= 0.f && NewTime <= CurrentAnimation->GetPlayLength())
		{
			KeyFrames[KeyFrameIndex].Frame = GetFrameAtTime(NewTime);
			KeyFrames.Sort();
			OnAnimationUpdated.Broadcast();
		}
	}
}

void UFingerDataExtractorComponent::UpdateKeyframeBlendType(int32 KeyFrameIndex, EMediaPipeBlendType NewBlendType)
{
	if (KeyFrames.IsValidIndex(KeyFrameIndex))
	{
		if (KeyFrames[KeyFrameIndex].BlendType != NewBlendType)
		{
			KeyFrames[KeyFrameIndex].BlendType = NewBlendType;
			KeyFrames.Sort();
			OnAnimationUpdated.Broadcast();
		}
	}
}

void UFingerDataExtractorComponent::DeleteKeyframe(int32 Index)
{
	if (KeyFrames.IsValidIndex(Index))
	{
		KeyFrames.RemoveAt(Index);
		OnAnimationUpdated.Broadcast();
	}
}

void UFingerDataExtractorComponent::RestoreProject(const TArray<FAnimSequenceDataFingers>& InAnimation, const TArray<FAnimCaptureInterval>& InIntervals, const TArray<FFingersKeyframe>& InKeyFrames)
{
	Animation = InAnimation;
	CaptureIntervals = InIntervals;
	KeyFrames = InKeyFrames;
	if (Mode != EMediaPipeCaptureMode::None)
	{
		Stop();
	}
	Seek(0);
	OnAnimationUpdated.Broadcast();
}

void UFingerDataExtractorComponent::BakeAnimationAsset(EMediaPipeAnimationMode AnimMode, bool bApplyBlending, bool bRightHand, bool bLeftHand)
{
	if (!IsValid(CurrentAnimation))
	{
		return;
	}

	// Request to bake animation (processed in the MediapipeFingersEditor module)
	//OnBakeAnimation.Broadcast(this, AnimationMode, bApplyBlending, bRightHand, bLeftHand);

#if WITH_EDITOR
	UAnimSequence* AnimSequence = CurrentAnimation;
	const FReferenceSkeleton& RefSkeleton = IsValid(AnimSequence->GetPreviewMesh())
		? AnimSequence->GetPreviewMesh()->GetRefSkeleton()
		: AnimSequence->GetSkeleton()->GetReferenceSkeleton();

	// Init list of bones
	TArray<FName> RightBones, LeftBones;
	TArray<FName> BoneNames;
	int32 LastBoneIndex;
	for (const auto& Finger : RightHandSetup->Fingers)
	{
		LastBoneIndex = RefSkeleton.FindBoneIndex(Finger.Value.TipBoneName);
		RightBones.Add(RefSkeleton.GetBoneName(LastBoneIndex));
		for (int32 i = 0; i < Finger.Value.KnucklesNum - 1; i++)
		{
			LastBoneIndex = RefSkeleton.GetParentIndex(LastBoneIndex);
			RightBones.Add(RefSkeleton.GetBoneName(LastBoneIndex));
		}
	}
	for (const auto& Finger : LeftHandSetup->Fingers)
	{
		LastBoneIndex = RefSkeleton.FindBoneIndex(Finger.Value.TipBoneName);
		LeftBones.Add(RefSkeleton.GetBoneName(LastBoneIndex));
		for (int32 i = 0; i < Finger.Value.KnucklesNum - 1; i++)
		{
			LastBoneIndex = RefSkeleton.GetParentIndex(LastBoneIndex);
			LeftBones.Add(RefSkeleton.GetBoneName(LastBoneIndex));
		}
	}
	BoneNames = RightBones; BoneNames.Append(LeftBones);

	UFingersFKIKSolver* RightSolver = UFingersFKIKSolver::CreateFingersFKIKSolver(RightHandSetup, GetAnimationMesh());
	UFingersFKIKSolver* LeftSolver = UFingersFKIKSolver::CreateFingersFKIKSolver(LeftHandSetup, GetAnimationMesh());
	IAnimationDataController& Controller = AnimSequence->GetController();

	const int32 KeysNum = AnimSequence->GetDataModel()->GetNumberOfKeys();

	// Tracks to save data
	TMap<FName, FRawAnimSequenceTrack> OutTracks;
	TArray<FTransform> FrameTransforms;
	FrameTransforms.SetNum(BoneNames.Num());

	TMap<FName, int32> TransformIndexByBone;
	for (int32 i = 0; i < BoneNames.Num(); i++)
	{
		TransformIndexByBone.Add(BoneNames[i], i);
	}

	// Initialize containers
	for (const auto& BoneName : BoneNames)
	{
		FRawAnimSequenceTrack& Track = OutTracks.Add(BoneName);
		Track.PosKeys.SetNumUninitialized(KeysNum);
		Track.RotKeys.SetNumUninitialized(KeysNum);
		Track.ScaleKeys.SetNumUninitialized(KeysNum);
	}

	FFingersRawPreset RightHand, LeftHand;
	// Evaluate frames and fill containers
	for (int32 FrameIndex = 0; FrameIndex < KeysNum; FrameIndex++)
	{
		float Time = GetTimeAtFrame(FrameIndex);
		float BlendAlpha = EvaluateFrame(FrameIndex, AnimMode, RightHand, LeftHand);

		// Generate hand poses for current frame
		RightSolver->ApplyVRInputRaw(RightHand);
		RightSolver->UpdateFingers();
		LeftSolver->ApplyVRInputRaw(LeftHand);
		LeftSolver->UpdateFingers();

		GetBonePosesForTime(AnimSequence, BoneNames, Time, false, FrameTransforms, AnimSequence->GetPreviewMesh());

		for (const auto& Finger : RightSolver->Fingers)
		{
			for (const auto& Knuckle : Finger.Value.Knuckles)
			{
				FTransform AnimBoneTr = FrameTransforms[TransformIndexByBone[Knuckle.BoneName]];
				FTransform ApplyBoneTr;
				if (BlendAlpha == 0.f)
				{
					ApplyBoneTr = AnimBoneTr;
				}
				else if (BlendAlpha == 1.f || !bApplyBlending)
				{
					ApplyBoneTr = Knuckle.RelativeTransform;
				}
				else
				{
					ApplyBoneTr = UKismetMathLibrary::TLerp(AnimBoneTr, Knuckle.RelativeTransform, BlendAlpha);
				}

				OutTracks[Knuckle.BoneName].PosKeys[FrameIndex] = (FVector3f)AnimBoneTr.GetTranslation();
				OutTracks[Knuckle.BoneName].ScaleKeys[FrameIndex] = (FVector3f)AnimBoneTr.GetScale3D();
				OutTracks[Knuckle.BoneName].RotKeys[FrameIndex] = (FQuat4f)ApplyBoneTr.GetRotation();
			}
		}

		for (const auto& Finger : LeftSolver->Fingers)
		{
			for (const auto& Knuckle : Finger.Value.Knuckles)
			{
				FTransform AnimBoneTr = FrameTransforms[TransformIndexByBone[Knuckle.BoneName]];
				FTransform ApplyBoneTr = (BlendAlpha == 1.f)
					? Knuckle.RelativeTransform
					: UKismetMathLibrary::TLerp(AnimBoneTr, Knuckle.RelativeTransform, BlendAlpha);

				OutTracks[Knuckle.BoneName].PosKeys[FrameIndex] = (FVector3f)AnimBoneTr.GetTranslation();
				OutTracks[Knuckle.BoneName].ScaleKeys[FrameIndex] = (FVector3f)AnimBoneTr.GetScale3D();
				OutTracks[Knuckle.BoneName].RotKeys[FrameIndex] = (FQuat4f)ApplyBoneTr.GetRotation();
			}
		}
	}

	// Save new keys in DataModel
	CurrentAnimation->Modify();
	Controller.OpenBracket(NSLOCTEXT("MediapipeFingers", "BakeFingersAnimation", "Bake Fingers Animation"));

	for (const auto& Track : OutTracks)
	{
		const FName& BoneName = Track.Key;

		if ((RightBones.Contains(BoneName) && bRightHand) ||
			(LeftBones.Contains(BoneName) && bLeftHand))
		{
			Controller.RemoveBoneTrack(BoneName);
			Controller.AddBoneCurve(BoneName);
			Controller.SetBoneTrackKeys(BoneName, Track.Value.PosKeys, Track.Value.RotKeys, Track.Value.ScaleKeys);
		}
	}

	Controller.CloseBracket();

	RightSolver->MarkAsGarbage();
	LeftSolver->MarkAsGarbage();
#endif
}

float UFingerDataExtractorComponent::EvaluateFrame(int64 Frame, EMediaPipeAnimationMode ModeOverride, FFingersRawPreset& OutRightHand, FFingersRawPreset& OutLeftHand)
{
	if (ModeOverride == EMediaPipeAnimationMode::Sequence)
	{
		if (Animation.IsValidIndex(Frame))
		{
			if (Animation[Frame].bEvaluatedRight)
			{
				OutRightHand = Animation[Frame].Right;
			}
			if (Animation[Frame].bEvaluatedLeft)
			{
				OutLeftHand = Animation[Frame].Left;
			}
		}

		int32 BlendDurationFrames = GetFrameAtTime(BlendDuration);
		int32 i = 1;
		bool bEval = Animation[Frame].bEvaluatedRight || Animation[Frame].bEvaluatedLeft;
		if (!bEval)
		{
			return 0.f;
		}

		for (; i <= BlendDurationFrames; i++)
		{
			bool bEval1 = Animation.IsValidIndex(Frame - i)
				? (Animation[Frame - i].bEvaluatedRight || Animation[Frame - i].bEvaluatedLeft)
				: bEval;
			bool bEval2 = Animation.IsValidIndex(Frame + i)
				? (Animation[Frame + i].bEvaluatedRight || Animation[Frame + i].bEvaluatedLeft)
				: bEval;

			if (bEval1 != bEval || bEval2 != bEval)
			{
				break;
			}
		}

		if (i > BlendDurationFrames)
		{
			return 1.f;
		}
		else
		{
			return (float)i / (float)BlendDurationFrames;
		}
	}
	else
	{
		float CurrentTime = GetTimeAtFrame(Frame);
		SeekToTime(CurrentTime);
		ShowKeyFrame(CurrentTime, OutRightHand, OutLeftHand);

		return 1.0f;
	}
}

void UFingerDataExtractorComponent::SetEditFilterHand(bool bRightHand, bool bIsEnabled)
{
	if (bRightHand)
	{
		EditHandsFilter.bEnabledRight = bIsEnabled;
	}
	else
	{
		EditHandsFilter.bEnabledLeft = bIsEnabled;
	}
}

void UFingerDataExtractorComponent::SetEditFilterFinger(bool bRightHand, EFingerName Finger, bool bIsEnabled)
{
	if (bRightHand)
	{
		EditHandsFilter.RightFingers[Finger] = bIsEnabled;
	}
	else
	{
		EditHandsFilter.LeftFingers[Finger] = bIsEnabled;
	}
}

void UFingerDataExtractorComponent::SetEditFilterFingerInterval(float TimeStart, float TimeEnd)
{
	if (TimeStart <= TimeEnd && TimeStart >= 0)
	{
		EditHandsFilter.FrameIntervalStart = GetFrameAtTime(TimeStart);
		EditHandsFilter.FrameIntervalEnd = GetFrameAtTime(TimeEnd);
	}
}

void UFingerDataExtractorComponent::GetEditFilterFingerInterval(float& TimeStart, float& TimeEnd) const
{
	TimeStart = GetTimeAtFrame(EditHandsFilter.GetIntervalStart());
	TimeEnd = GetTimeAtFrame(EditHandsFilter.GetIntervalEnd(CurrentAnimation));
}

// This modifier doesn't work for key frane mode
void UFingerDataExtractorComponent::Modifier_SimpleSmooth()
{
	int64 FrameStart = EditHandsFilter.GetIntervalStart();
	int64 FrameEnd = EditHandsFilter.GetIntervalEnd(CurrentAnimation);

	for (int64 Frame = FrameStart; Frame <= FrameEnd; Frame++)
	{
		int32 PrevFrameIndex = FMath::Max(0, Frame - 1);
		int32 NextFrameIndex = FMath::Min(Animation.Num() - 1, Frame + 1);

		if (EditHandsFilter.bEnabledRight)
		{
			TMap<EFingerName, FFingerRawRotation*> Fingers0 = __declareFingersArr(PrevFrameIndex, Right);
			TMap<EFingerName, FFingerRawRotation*> Fingers1 = __declareFingersArr(Frame, Right);
			TMap<EFingerName, FFingerRawRotation*> Fingers2 = __declareFingersArr(NextFrameIndex, Right);

			for (auto& Fing : Fingers1)
			{
				if (EditHandsFilter.RightFingers[Fing.Key])
				{
					MakeFingerSum(*Fing.Value, *Fingers0[Fing.Key]);
					MakeFingerSum(*Fing.Value, *Fingers2[Fing.Key]);
					MakeFingerMul(*Fing.Value, 0.3f);
				}
			}
		}

		if (EditHandsFilter.bEnabledLeft)
		{
			TMap<EFingerName, FFingerRawRotation*> Fingers0 = __declareFingersArr(PrevFrameIndex, Left);
			TMap<EFingerName, FFingerRawRotation*> Fingers1 = __declareFingersArr(Frame, Left);
			TMap<EFingerName, FFingerRawRotation*> Fingers2 = __declareFingersArr(NextFrameIndex, Left);

			for (auto& Fing : Fingers1)
			{
				if (EditHandsFilter.LeftFingers[Fing.Key])
				{
					MakeFingerSum(*Fing.Value, *Fingers0[Fing.Key]);
					MakeFingerSum(*Fing.Value, *Fingers2[Fing.Key]);
					MakeFingerMul(*Fing.Value, 0.3f);
				}
			}
		}
	}
}

void UFingerDataExtractorComponent::Modifier_SmoothCurls()
{
	int64 FrameStart = EditHandsFilter.GetIntervalStart();
	int64 FrameEnd = EditHandsFilter.GetIntervalEnd(CurrentAnimation);

	int64 IndexStart = (AnimationMode == EMediaPipeAnimationMode::Sequence) ? FrameStart : 0;
	int64 IndexEnd = (AnimationMode == EMediaPipeAnimationMode::Sequence) ? FrameEnd : KeyFrames.Num() - 1;
	for (int64 Frame = IndexStart; Frame <= IndexEnd; Frame++)
	{
		if (AnimationMode == EMediaPipeAnimationMode::KeyFrames && (KeyFrames.IsEmpty() || KeyFrames[Frame].Frame < FrameStart || KeyFrames[Frame].Frame > FrameEnd))
		{
			continue;
		}

		if (EditHandsFilter.bEnabledRight)
		{
			TMap<EFingerName, FFingerRawRotation*> Fingers;
			if (AnimationMode == EMediaPipeAnimationMode::Sequence)
				Fingers = __declareFingersArr(Frame, Right);
			else
				Fingers = __declareFingersKFArr(Frame, Right);

			for (auto& Fing : Fingers)
			{
				if (EditHandsFilter.RightFingers[Fing.Key])
				{
					int32 LastIndex = Fing.Value->Curls.Num() - 1;
					for (int32 i = 0; i <= LastIndex; i++)
					{
						float v;
						if (i == 0)
							v = (Fing.Value->Curls[i] + Fing.Value->Curls[i + 1]) * 0.5f;
						else if (i == LastIndex)
							v = (Fing.Value->Curls[i] + Fing.Value->Curls[i - 1]) * 0.5f;
						else
							v = (Fing.Value->Curls[i] + Fing.Value->Curls[i - 1] + Fing.Value->Curls[i + 1]) / 3.f;
						Fing.Value->Curls[i] = (i == 0) ? v : FMath::Max(0.f, v);
					}
				}
			}
		}
		if (EditHandsFilter.bEnabledLeft)
		{
			TMap<EFingerName, FFingerRawRotation*> Fingers;
			if (AnimationMode == EMediaPipeAnimationMode::Sequence)
				Fingers = __declareFingersArr(Frame, Left);
			else
				Fingers = __declareFingersKFArr(Frame, Left);

			for (auto& Fing : Fingers)
			{
				if (EditHandsFilter.LeftFingers[Fing.Key])
				{
					int32 LastIndex = Fing.Value->Curls.Num() - 1;
					for (int32 i = 0; i <= LastIndex; i++)
					{
						float v;
						if (i == 0)
							v = (Fing.Value->Curls[i] + Fing.Value->Curls[i + 1]) * 0.5f;
						else if (i == LastIndex)
							v = (Fing.Value->Curls[i] + Fing.Value->Curls[i - 1]) * 0.5f;
						else
							v = (Fing.Value->Curls[i] + Fing.Value->Curls[i - 1] + Fing.Value->Curls[i + 1]) / 3.f;
						Fing.Value->Curls[i] = (i == 0) ? v : FMath::Max(0.f, v);
					}
				}
			}
		}
	}
}

void UFingerDataExtractorComponent::Modifier_RemoveNegaiveCurls()
{
	int64 FrameStart = EditHandsFilter.GetIntervalStart();
	int64 FrameEnd = EditHandsFilter.GetIntervalEnd(CurrentAnimation);

	int64 IndexStart = (AnimationMode == EMediaPipeAnimationMode::Sequence) ? FrameStart : 0;
	int64 IndexEnd = (AnimationMode == EMediaPipeAnimationMode::Sequence) ? FrameEnd : KeyFrames.Num() - 1;
	for (int64 Frame = IndexStart; Frame <= IndexEnd; Frame++)
	{
		if (AnimationMode == EMediaPipeAnimationMode::KeyFrames && (KeyFrames.IsEmpty() || KeyFrames[Frame].Frame < FrameStart || KeyFrames[Frame].Frame > FrameEnd))
		{
			continue;
		}

		if (EditHandsFilter.bEnabledRight)
		{
			TMap<EFingerName, FFingerRawRotation*> Fingers;
			if (AnimationMode == EMediaPipeAnimationMode::Sequence)
				Fingers = __declareFingersArr(Frame, Right);
			else
				Fingers = __declareFingersKFArr(Frame, Right);

			for (auto& Fing : Fingers)
			{
				if (EditHandsFilter.RightFingers[Fing.Key])
				{
					for (float& Curl : Fing.Value->Curls)
					{
						if (Curl < 0.f) Curl = 0.f;
					}
				}
			}
		}
		if (EditHandsFilter.bEnabledLeft)
		{
			TMap<EFingerName, FFingerRawRotation*> Fingers;
			if (AnimationMode == EMediaPipeAnimationMode::Sequence)
				Fingers = __declareFingersArr(Frame, Left);
			else
				Fingers = __declareFingersKFArr(Frame, Left);

			for (auto& Fing : Fingers)
			{
				if (EditHandsFilter.LeftFingers[Fing.Key])
				{
					for (float& Curl : Fing.Value->Curls)
					{
						if (Curl < 0.f) Curl = 0.f;
					}
				}
			}
		}
	}
}

void UFingerDataExtractorComponent::Modifier_MultiplyFingerRotation(EMediaPipeLandmarkAxisMapping Axis, const float Multiplier)
{
	// x = finger roll
	// y = curls
	// z = finger yaw
	
	int64 FrameStart = EditHandsFilter.GetIntervalStart();
	int64 FrameEnd = EditHandsFilter.GetIntervalEnd(CurrentAnimation);

	int64 IndexStart = (AnimationMode == EMediaPipeAnimationMode::Sequence) ? FrameStart : 0;
	int64 IndexEnd = (AnimationMode == EMediaPipeAnimationMode::Sequence) ? FrameEnd : KeyFrames.Num() - 1;
	for (int64 Frame = IndexStart; Frame <= IndexEnd; Frame++)
	{
		if (AnimationMode == EMediaPipeAnimationMode::KeyFrames && (KeyFrames.IsEmpty() || KeyFrames[Frame].Frame < FrameStart || KeyFrames[Frame].Frame > FrameEnd))
		{
			continue;
		}

		if (EditHandsFilter.bEnabledRight)
		{
			TMap<EFingerName, const FFingerRawRotation*> RefPoseFingers {
				{ EFingerName::FN_Index, & HandReferencePoseRight.Index},
				{ EFingerName::FN_Middle, &HandReferencePoseRight.Middle },
				{ EFingerName::FN_Ring, &HandReferencePoseRight.Ring },
				{ EFingerName::FN_Pinky, &HandReferencePoseRight.Pinky },
				{ EFingerName::FN_Thumb, &HandReferencePoseRight.Thumb }
			};

			TMap<EFingerName, FFingerRawRotation*> Fingers;
			if (AnimationMode == EMediaPipeAnimationMode::Sequence)
				Fingers = __declareFingersArr(Frame, Right);
			else
				Fingers = __declareFingersKFArr(Frame, Right);

			for (auto& Fing : Fingers)
			{
				if (EditHandsFilter.RightFingers[Fing.Key])
				{
					if (Axis == EMediaPipeLandmarkAxisMapping::X)
					{
						Fing.Value->FingerRoll = RefPoseFingers[Fing.Key]->FingerRoll + (Fing.Value->FingerRoll - RefPoseFingers[Fing.Key]->FingerRoll) * Multiplier;
					}
					else if (Axis == EMediaPipeLandmarkAxisMapping::Z)
					{
						Fing.Value->FingerYaw = RefPoseFingers[Fing.Key]->FingerYaw + (Fing.Value->FingerYaw - RefPoseFingers[Fing.Key]->FingerYaw) * Multiplier;
					}
					else
					{
						for (float& Curl : Fing.Value->Curls)
						{
							Curl *= Multiplier;
						}
					}
				}
			}
		}
		if (EditHandsFilter.bEnabledLeft)
		{
			TMap<EFingerName, const FFingerRawRotation*> RefPoseFingers {
				{ EFingerName::FN_Index, & HandReferencePoseLeft.Index},
				{ EFingerName::FN_Middle, &HandReferencePoseLeft.Middle },
				{ EFingerName::FN_Ring, &HandReferencePoseLeft.Ring },
				{ EFingerName::FN_Pinky, &HandReferencePoseLeft.Pinky },
				{ EFingerName::FN_Thumb, &HandReferencePoseLeft.Thumb }
			};

			TMap<EFingerName, FFingerRawRotation*> Fingers;
			if (AnimationMode == EMediaPipeAnimationMode::Sequence)
				Fingers = __declareFingersArr(Frame, Left);
			else
				Fingers = __declareFingersKFArr(Frame, Left);

			for (auto& Fing : Fingers)
			{
				if (EditHandsFilter.LeftFingers[Fing.Key])
				{
					if (Axis == EMediaPipeLandmarkAxisMapping::X)
					{
						Fing.Value->FingerRoll = RefPoseFingers[Fing.Key]->FingerRoll + (Fing.Value->FingerRoll - RefPoseFingers[Fing.Key]->FingerRoll) * Multiplier;
					}
					else if (Axis == EMediaPipeLandmarkAxisMapping::Z)
					{
						Fing.Value->FingerYaw = RefPoseFingers[Fing.Key]->FingerYaw + (Fing.Value->FingerYaw - RefPoseFingers[Fing.Key]->FingerYaw) * Multiplier;
					}
					else
					{
						for (float& Curl : Fing.Value->Curls)
						{
							Curl *= Multiplier;
						}
					}
				}
			}
		}
	}
}

void UFingerDataExtractorComponent::Modifier_AddToCurl(const float Addend)
{
	int64 FrameStart = EditHandsFilter.GetIntervalStart();
	int64 FrameEnd = EditHandsFilter.GetIntervalEnd(CurrentAnimation);

	int64 IndexStart = (AnimationMode == EMediaPipeAnimationMode::Sequence) ? FrameStart : 0;
	int64 IndexEnd = (AnimationMode == EMediaPipeAnimationMode::Sequence) ? FrameEnd : KeyFrames.Num() - 1;
	for (int64 Frame = IndexStart; Frame <= IndexEnd; Frame++)
	{
		if (AnimationMode == EMediaPipeAnimationMode::KeyFrames && (KeyFrames.IsEmpty() || KeyFrames[Frame].Frame < FrameStart || KeyFrames[Frame].Frame > FrameEnd))
		{
			continue;
		}

		if (EditHandsFilter.bEnabledRight)
		{
			TMap<EFingerName, FFingerRawRotation*> Fingers = __declareFingersArr(Frame, Right);
			for (auto& Fing : Fingers)
			{
				if (EditHandsFilter.RightFingers[Fing.Key])
				{
					for (int32 i = 0; i < Fing.Value->Curls.Num(); i++)
					{
						float& Curl = Fing.Value->Curls[i];
						Curl += Addend;
						if (i > 0) Curl = FMath::Max(0, Curl);
					}
				}
			}
		}
		if (EditHandsFilter.bEnabledLeft)
		{
			TMap<EFingerName, FFingerRawRotation*> Fingers = __declareFingersArr(Frame, Left);
			for (auto& Fing : Fingers)
			{
				if (EditHandsFilter.LeftFingers[Fing.Key])
				{
					for (int32 i = 0; i < Fing.Value->Curls.Num(); i++)
					{
						float& Curl = Fing.Value->Curls[i];
						Curl += Addend;
						if (i > 0) Curl = FMath::Max(0, Curl);
					}
				}
			}
		}
	}
}

void UFingerDataExtractorComponent::GetBonePosesForTime(const UAnimSequenceBase* AnimationSequenceBase, const TArray<FName>& BoneNames, float Time, bool bExtractRootMotion, TArray<FTransform>& Poses, const USkeletalMesh* PreviewMesh) const
{
#if WITH_EDITOR
#if ENGINE_MINOR_VERSION > 1
	Poses.Empty(BoneNames.Num());
	if (!AnimationSequenceBase && AnimationSequenceBase->GetSkeleton())
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid Animation Sequence supplied for GetBonePosesForTime"));
	}

	Poses.AddDefaulted(BoneNames.Num());

	// Need this for FCompactPose
	FMemMark Mark(FMemStack::Get());

	const FReferenceSkeleton& RefSkeleton = (PreviewMesh) ? PreviewMesh->GetRefSkeleton() : AnimationSequenceBase->GetSkeleton()->GetReferenceSkeleton();

	if (Time < 0.f || Time > AnimationSequenceBase->GetDataModel()->GetPlayLength())
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid time value %f for Animation Sequence %s supplied for GetBonePosesForTime"), Time, *AnimationSequenceBase->GetName());
	}

	if (BoneNames.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid or no bone names specified to retrieve poses given Animation Sequence %s in GetBonePosesForTime"), *AnimationSequenceBase->GetName());
	}

	int32 Frame = AnimationSequenceBase->GetFrameAtTime(Time);

	for (int32 BoneNameIndex = 0; BoneNameIndex < BoneNames.Num(); ++BoneNameIndex)
	{
		const FName& BoneName = BoneNames[BoneNameIndex];
		FTransform& Transform = Poses[BoneNameIndex];

		if (AnimationSequenceBase->GetDataModel()->IsValidBoneTrackName(BoneName))
		{
			const EAnimInterpolationType InterpolationType = [AnimationSequenceBase]() -> EAnimInterpolationType
			{
				if (const UAnimSequence* AnimationSequence = Cast<const UAnimSequence>(AnimationSequenceBase))
				{
					return AnimationSequence->Interpolation;
				}

				return EAnimInterpolationType::Linear;
			}();

			Transform = AnimationSequenceBase->GetDataModel()->EvaluateBoneTrackTransform(BoneName, FFrameTime(Frame), InterpolationType);
		}
		else
		{
			// otherwise, get ref pose if exists
			const int32 BoneIndex = RefSkeleton.FindBoneIndex(BoneName);
			if (BoneIndex != INDEX_NONE)
			{
				Transform = RefSkeleton.GetRefBonePose()[BoneIndex];
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Invalid bone name %s for Animation Sequence %s supplied for GetBonePosesForTime"), *BoneName.ToString(), *AnimationSequenceBase->GetName());
				Transform = FTransform::Identity;
			}
		}
	}
#else
	UAnimationBlueprintLibrary::GetBonePosesForTime(AnimationSequenceBase, BoneNames, Time, bExtractRootMotion, Poses, PreviewMesh);
#endif
#endif
}

#undef __declareFingersKFArr
#undef __declareFingersArr
#undef __initialized
#undef __currentFrame