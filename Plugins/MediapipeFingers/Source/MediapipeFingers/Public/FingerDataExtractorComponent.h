// (c) Yuri Kalinin, 2023. All Rights Reserved
// ykasczc@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "IXRTrackingSystem.h"
#include "FingersTypes.h"
#include "MediapipeTypes.h"
#include "MediapipeFingersTypes.h"
#include "FingerDataExtractorComponent.generated.h"

class UMediaPipeLandmarkObserverComponent;
class UMediaPipeClassificationObserverComponent;
class UAnimSequence;
class UFingersSolverSetup;
class USkeletalMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFingersCaptureModeChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FFingersTimeChanged, float, Time, int64, Frame);

/**
* Do fingers mocap from Mediapipe landmarks data
*/
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MEDIAPIPEFINGERS_API UFingerDataExtractorComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Sets default values for this component's properties
	UFingerDataExtractorComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Name of the landmark observer component
	UPROPERTY(EditAnywhere, Category = "Setup")
	FName PoseObserverName;

	// Name of the landmark classificator component
	UPROPERTY(EditAnywhere, Category = "Setup")
	FName HandClassificatorName;

	// Name of the skeletal mesh component
	UPROPERTY(EditAnywhere, Category = "Setup")
	FName SkeletalMeshName;

	// Interp speed to smoothen input mediapipe data
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setup")
	float InterpolationSpeed = 16.f;

	// Default input smoothening (using mean value of N last frames)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup")
	int32 SimpleSmooth = 3;

	// Used in all cases when blending is applied, including key frames and baking of animation intervals to existing animation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setup")
	float BlendDuration = 0.5f;

	// Used in all cases when blending is applied, including key frames and baking of animation intervals to existing animation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setup")
	EMediaPipeBlendType BlendType;

	// Curve should provide alpha in time interval from 0 to 1
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(EditCondition="BlendType==EMediaPipeBlendType::FloatCurve"), Category = "Setup")
	class UCurveFloat* BlendCurve = nullptr;

	// SkeletalMesh-specific settings of the right hand
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setup")
	UFingersSolverSetup* RightHandSetup;

	// SkeletalMesh-specific settings of the left hand
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setup")
	UFingersSolverSetup* LeftHandSetup;

	// Reference poses (from ref pose of the skeleton)
	UPROPERTY(BlueprintReadOnly, Category = "Setup")
	FFingersRawPreset HandReferencePoseRight;

	// Reference poses (from ref pose of the skeleton)
	UPROPERTY(BlueprintReadOnly, Category = "Setup")
	FFingersRawPreset HandReferencePoseLeft;


	// Called when animation playing (including fingers data) started
	UPROPERTY(BlueprintAssignable, Category = "Fingers Tracking")
	FFingersCaptureModeChanged OnPlayStarted;

	// Called when data capture started (only in sequence mode)
	UPROPERTY(BlueprintAssignable, Category = "Fingers Tracking")
	FFingersCaptureModeChanged OnCaptureStarted;

	// Called when play/capture finished
	UPROPERTY(BlueprintAssignable, Category = "Fingers Tracking")
	FFingersCaptureModeChanged OnStop;

	// Called every tick when playing or caputing
	UPROPERTY(BlueprintAssignable, Category = "Fingers Tracking")
	FFingersTimeChanged OnTime;

	// Called when animation intervals or key frames were modified
	UPROPERTY(BlueprintAssignable, Category = "Fingers Tracking")
	FFingersCaptureModeChanged OnAnimationUpdated;


	// Play Rate for capture and play
	UPROPERTY(BlueprintReadWrite, Category = "Fingers Tracking")
	float AnimationPlayRate = 1.f;

	// Filter capture by hand
	UPROPERTY(BlueprintReadWrite, Category = "Fingers Tracking")
	bool bCaptureHandRight = true;

	// Filter capture by hand
	UPROPERTY(BlueprintReadWrite, Category = "Fingers Tracking")
	bool bCaptureHandLeft = true;

	// Filter used to apply modifiers and bake animation
	UPROPERTY(BlueprintReadWrite, Category = "Fingers Tracking")
	FHandsFilter EditHandsFilter;

	// This component's state: play, capture, preview etc.
	UPROPERTY(BlueprintReadOnly, Category = "Fingers Tracking")
	EMediaPipeCaptureMode Mode = EMediaPipeCaptureMode::None;

	// Mocap mode: frames sequence or keyframed animation
	UPROPERTY(BlueprintReadOnly, Category = "Fingers Tracking")
	EMediaPipeAnimationMode AnimationMode = EMediaPipeAnimationMode::Sequence;

	// Currently used and processed animation asset. Should always be set.
	UPROPERTY(BlueprintReadOnly, Category = "Fingers Tracking")
	UAnimSequence* CurrentAnimation = nullptr;

	// Output hand state for preview
	UPROPERTY(BlueprintReadOnly, Category = "Fingers Tracking")
	FFingersRawPreset PreviewStateRight;
	// Last state updated from mediapipe packet
	FFingersRawPreset LastUpdatedStateRight;
	UPROPERTY(BlueprintReadOnly, Category = "Fingers Tracking")
	FXRMotionControllerData CurrentHandStateRight;

	// Output hand state for preview
	UPROPERTY(BlueprintReadOnly, Category = "Fingers Tracking")
	FFingersRawPreset PreviewStateLeft;
	// Last state updated from mediapipe packet
	FFingersRawPreset LastUpdatedStateLeft;
	UPROPERTY(BlueprintReadOnly, Category = "Fingers Tracking")
	FXRMotionControllerData CurrentHandStateLeft;

	// Has fingers data to show in current frame?
	UPROPERTY(BlueprintReadOnly, Category = "Fingers Tracking")
	bool bHasFrameData = false;
	
	// Time last frame was received from mediapipe
	UPROPERTY()
	float LastUpdateTime = 0.f;

	// Name of the right hand bone at the skeleton (to bake animation)
	UPROPERTY(BlueprintReadOnly, Category = "Fingers Tracking")
	FName HandBoneRight;

	// Name of the left hand bone at the skeleton (to bake animation)
	UPROPERTY(BlueprintReadOnly, Category = "Fingers Tracking")
	FName HandBoneLeft;

	// Set CurrentAnimation asset. Call it in BeginPlay.
	UFUNCTION(BlueprintCallable, Category = "Fingers Tracking")
	void SetAnimation(UAnimSequence* Sequence);

	// Start capture of capture single frame (in key frame mode)
	UFUNCTION(BlueprintCallable, Category = "Fingers Tracking")
	bool Capture(float StartTime = -1.f);

	// Naviate to the frame or time
	UFUNCTION(BlueprintCallable, Category = "Fingers Tracking")
	bool Seek(int32 Frame);

	// Naviate to the frame or time
	UFUNCTION(BlueprintCallable, Category = "Fingers Tracking")
	bool SeekToTime(float Time);

	// Stop playin/stop capture
	UFUNCTION(BlueprintCallable, Category = "Fingers Tracking")
	void Stop();

	// Play fingers animation from the current position or specified time
	UFUNCTION(BlueprintCallable, Category = "Fingers Tracking")
	void Play(float StartTime = -1.f);

	// Toggle updating finger poses from mediapipe in tick
	UFUNCTION(BlueprintCallable, Category = "Fingers Tracking")
	void SetPreviewMode(bool bEnabled);

	// Toggle sequence/keyframe mode
	UFUNCTION(BlueprintCallable, Category = "Fingers Tracking")
	void SetAnimationMode(EMediaPipeAnimationMode NewMode);

	// Get current position (see Seek)
	UFUNCTION(BlueprintPure, Category = "Fingers Tracking")
	float GetPlayPosition() const;

	/** Get the frame number for the provided time */
	UFUNCTION(BlueprintPure, Category = "Animation")
	int32 GetFrameAtTime(const float Time) const;

	/** Get the time at the given frame */
	UFUNCTION(BlueprintPure, Category = "Animation")
	float GetTimeAtFrame(const int32 Frame) const;


	/** Functions to read/modify captured animation data **/

	// Get list of intervals with captured data (in sequence mode).
	// In theory, last captured interval can be cancelled, but this feature isn't implemented
	// Interval data is used only to blend captured animation to existing when baking
	UFUNCTION(BlueprintPure, Category = "Fingers Tracking")
	void GetAnimationIntervals(TArray<FAnimCaptureInterval>& OutIntervals) const;

	// Real captured data (in sequence mode), i. e. animation for both hands
	UFUNCTION(BlueprintPure, Category = "Fingers Tracking")
	void GetAnimationData(TArray<FAnimSequenceDataFingers>& OutAnimation) const;

	// By deleting interval, we don't remove captured data in this interval
	UFUNCTION(BlueprintCallable, Category = "Fingers Tracking")
	void DeleteAnimationInterval(int64 StartFrame);

	// By deleting interval, we don't remove captured data in this interval
	UFUNCTION(BlueprintCallable, Category = "Fingers Tracking")
	void DeleteAnimationIntervalByIndex(int32 Index);

	// Get all captured key frames
	UFUNCTION(BlueprintPure, Category = "Fingers Tracking")
	void GetAnimationKeyFrames(TArray<FFingersKeyframe>& OutKeys) const;

	// Move key frame on timeline
	UFUNCTION(BlueprintCallable, Category = "Fingers Tracking")
	void UpdateKeyframeTime(int32 KeyFrameIndex, float NewTime);

	// Change blending type of the key frame
	UFUNCTION(BlueprintCallable, Category = "Fingers Tracking")
	void UpdateKeyframeBlendType(int32 KeyFrameIndex, EMediaPipeBlendType NewBlendType);

	// Delete key frame forever (can't be undone)
	UFUNCTION(BlueprintCallable, Category = "Fingers Tracking")
	void DeleteKeyframe(int32 Index);

	// Used to load project from file
	UFUNCTION(BlueprintCallable, Category = "Fingers Tracking")
	void RestoreProject(const TArray<FAnimSequenceDataFingers>& InAnimation, const TArray<FAnimCaptureInterval>& InIntervals, const TArray<FFingersKeyframe>& InKeyFrames);

	/**
	* Save captured animation in animation sequence
	* @param AnimMode		Use either sequence intervals or keyframed animation
	* @param bApplyBlending	Apply blending to start and end of intervals
	*/
	UFUNCTION(BlueprintCallable, Category = "Fingers Tracking")
	void BakeAnimationAsset(EMediaPipeAnimationMode AnimMode, bool bApplyBlending, bool bRightHand = true, bool bLeftHand = true);

	/* Evaluate frame for baking animation. Returns Alpha. */
	float EvaluateFrame(int64 Frame, EMediaPipeAnimationMode ModeOverride, FFingersRawPreset& OutRightHand, FFingersRawPreset& OutLeftHand);

	// Enable/disable hand processing in EditHandsFilter
	UFUNCTION(BlueprintCallable, Category = "Fingers Tracking")
	void SetEditFilterHand(bool bRightHand, bool bIsEnabled);

	// Enable/disable hand processing in EditHandsFilter
	UFUNCTION(BlueprintCallable, Category = "Fingers Tracking")
	void SetEditFilterFinger(bool bRightHand, EFingerName Finger, bool bIsEnabled);

	// Set porcessed interval for modifiers
	UFUNCTION(BlueprintCallable, Category = "Fingers Tracking")
	void SetEditFilterFingerInterval(float TimeStart, float TimeEnd);

	// Get porcessed interval for modifiers
	UFUNCTION(BlueprintPure, Category = "Fingers Tracking")
	void GetEditFilterFingerInterval(float& TimeStart, float& TimeEnd) const;

	// Modifier: simple smooth animation frames (only for sequence)
	UFUNCTION(BlueprintCallable, Category = "Fingers Tracking")
	void Modifier_SimpleSmooth();

	// Modifier: simple smooth finger curl rotations
	UFUNCTION(BlueprintCallable, Category = "Fingers Tracking")
	void Modifier_SmoothCurls();

	// Modifier: remove first curl below zero
	UFUNCTION(BlueprintCallable, Category = "Fingers Tracking")
	void Modifier_RemoveNegaiveCurls();

	// Modifier: multiply fingers rotation around Axis by Muliplier value
	UFUNCTION(BlueprintCallable, Category = "Fingers Tracking")
	void Modifier_MultiplyFingerRotation(EMediaPipeLandmarkAxisMapping Axis, const float Multiplier);

	// Modifier: add Addend to the curls
	UFUNCTION(BlueprintCallable, Category = "Fingers Tracking")
	void Modifier_AddToCurl(const float Addend);

	UFUNCTION(BlueprintPure, Category = "Fingers Tracking")
	USkeletalMeshComponent* GetAnimationMesh() const { return BodyMesh; }

protected:
	UMediaPipeLandmarkObserverComponent* Observer = nullptr;
	UMediaPipeClassificationObserverComponent* Classificator = nullptr;

	USkeletalMeshComponent* BodyMesh = nullptr;

	// Captured animation in frames
	TArray<FAnimSequenceDataFingers> Animation;
	TArray<FAnimCaptureInterval> CaptureIntervals;
	// Keyframed animation (alternative to sequenced)
	TArray<FFingersKeyframe> KeyFrames;

	// Capture data
	float CaptureStartSystemTime = 0.f;
	int64 PlayStartFrame = 0;
	int64 CaptureStartFrame = 0;
	int32 PlayKeyframeIndex = 0;

	// Simple smooth
	TArray<FFingersRawPreset> SmoothValuesRight;
	int32 SmoothValuesRight_Index = INDEX_NONE;
	TArray<FFingersRawPreset> SmoothValuesLeft;
	int32 SmoothValuesLeft_Index = INDEX_NONE;

	// Saves current frame from mediapipe to PreviewHand variables
	void CaptureFrame(float DeltaTime);
	bool ShowKeyFrame(float CurrentTime, FFingersRawPreset& OutRight, FFingersRawPreset& OutLeft);

	void CreateNewCaptureInterval(int64 FrameStart, int64 FrameEnd);

	// Convert mediapipe data to OpenXR format of hand tracking data
	void MediapipeDataToXR(bool bRightHand, const TArray<FMediaPipeLandmark>& InData, FXRMotionControllerData& HandDataXR) const;
	void ComputeFingerRotation(FXRMotionControllerData& InOutData, const int32 TipIndex) const;
	float GetMeanPresense(EFingerName Finger, const TArray<FMediaPipeLandmark>& Data) const;
	float GetAlphaByBlendType(float InAlpha, EMediaPipeBlendType InBlendType) const;

	FFingersRawPreset MeanFingersPreset(const TArray<FFingersRawPreset>& Members) const;
	void MakeFingerSum(FFingerRawRotation& InOutFinger, const FFingerRawRotation& Addend) const;
	void MakeFingerMul(FFingerRawRotation& InOutFinger, float Multiplier) const;

	// blend two states using BlendType
	FFingerRawRotation BlendFinger(const FFingerRawRotation& A, const FFingerRawRotation& B, float Alpha) const;
	FFingerRawRotation BlendFinger(const FFingerRawRotation& A, const FFingerRawRotation& B, float Alpha, EMediaPipeBlendType InBlendType) const;

	// blend in Animation array. Forward = from Cached to Current, Backward = from Current to Cached
	void BlendCapturedData(int64 From, int64 To, bool bForwardDirection);
	// used in keyframe mode, using BlendType
	void GetFrameHandValue(float Time, const FFingersKeyframe& A, const FFingersKeyframe& B, FFingersRawPreset& OutRight, FFingersRawPreset& OutLeft) const;

	// Editor-only function
	void GetBonePosesForTime(const UAnimSequenceBase* AnimationSequenceBase, const TArray<FName>& BoneNames, float Time, bool bExtractRootMotion, TArray<FTransform>& Poses, const USkeletalMesh* PreviewMesh) const;
};
