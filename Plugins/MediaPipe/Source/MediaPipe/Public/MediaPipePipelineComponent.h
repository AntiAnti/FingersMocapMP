// (c) wongfei - https://github.com/wongfei
// 2021, All Rights Reserved. Apache License Version 2.0
// Modified and updated: YuriNK (ykasczc@gmail.com)

#pragma once

#include "ump_api.h"
#include "Components/ActorComponent.h"
#include "DynamicTexture.h"
#include "MediapipeTypes.h"
#include "MediaPipePipelineComponent.generated.h"

DECLARE_STATS_GROUP(TEXT("MediaPipe"), STATGROUP_MediaPipe, STATCAT_Advanced);

class FDynamicTexture;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDynamicTextureCreatedSignature, UTexture2D*, NewTexture);

/**
* Pipeline reads input source (camera or video file) and sends input data to the specified mediapipe graph to process it
*/
UCLASS(ClassGroup="MediaPipe", meta=(BlueprintSpawnableComponent))
class MEDIAPIPE_API UMediaPipePipelineComponent : 
	public UActorComponent, 
	public IUmpFrameCallback
{
	GENERATED_BODY()

public:
	UMediaPipePipelineComponent();

	// UObject
	void BeginDestroy() override;

	// UActorComponent
	void InitializeComponent() override;
	void BeginPlay() override;
	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	void UninitializeComponent() override;
	void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// IMediaPipeFrameCallback
	void OnUmpFrame(class IUmpFrame* frame) override;

	// Core config

	// Unique name (pipe0)
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "MediaPipe|Graph")
	FString PipelineName;

	// Path after [Plugin]/ThirdParty/mediapipe/Data
	UPROPERTY(Category="MediaPipe|Graph", meta=(DisplayName="Graphs Folder"), BlueprintReadWrite, EditAnywhere)
	FString GraphPrefix = TEXT("mediapipe/unreal/");

	// Graph file extension 
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "MediaPipe|Graph")
	FString GraphExt = TEXT(".pbtxt");

	// Names of the graph files (GraphPrefix/GraphNodes.GraphExt)
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "MediaPipe|Graph")
	TArray<FString> GraphNodes;

	// Connect UMediaPipeObserver components in the same actor as stream observes
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "MediaPipe|Graph")
	bool bAutoBindObservers = true;

	// Capture config

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "MediaPipe|Capture")
	FString InputFile;

	/* ID of the video capturing device to open.
	To open default camera using default backend just pass 0.
	To backward compatibility usage of camera_id + domain_offset (CAP_*) is valid when CaptureAPI is CAP_ANY.
	For ref: this value is used in cv::VideoCapture::Open(camera_id, apiReference) function.
	*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "MediaPipe|Capture")
	int32 CameraId = 0;

	// Use 0 for any camera. Also use CAP_DSHOW (700) and CAP_MSMF (1400)
	// https://docs.opencv.org/3.4/d4/d15/group__videoio__flags__base.html
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "MediaPipe|Capture")
	ECV_VideoCaptureAPI CaptureAPI = ECV_VideoCaptureAPI::CAP_ANY;

	// Override capture resolution. (0, 0) - use default resolution of the input source.
	// Note: mediapipe will resize it to 512*512.
	// Note: your camera should support this resolution.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "MediaPipe|Capture")
	FVector2f CaptureResolution;

	// Override input source frame rate (0 - use default)
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "MediaPipe|Capture")
	int32 CaptureFPS = 0;

	// Video texture

	// Generate texture asset from video caemra/file input
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "MediaPipe|VideoTexture")
	bool bEnableVideoTexture;

	// Called when video texture object is ready
	UPROPERTY(BlueprintAssignable, Category = "MediaPipe|VideoTexture")
	FDynamicTextureCreatedSignature OnVideoTextureCreated;

	// Utils config

	// Create window with input video data
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MediaPipe|Utils")
	bool bEnableOverlay;

	// Stats

	// ID of the last processed frame
	UPROPERTY(BlueprintReadOnly, Category = "MediaPipe|Stats")
	int32 LastFrameId = 0;

	// Time stamp of the last processed frame
	UPROPERTY(BlueprintReadOnly, Category = "MediaPipe|Stats")
	float LastFrameTimestamp = 0;

	// Methods

	UFUNCTION(BlueprintCallable, Category="MediaPipe")
	void AddObserver(class UMediaPipeObserverComponent* Observer);

	UFUNCTION(BlueprintCallable, Category="MediaPipe")
	void RemoveObserver(class UMediaPipeObserverComponent* Observer);

	UFUNCTION(BlueprintCallable, Category="MediaPipe")
	void RemoveAllObservers();

	UFUNCTION(BlueprintCallable, Category="MediaPipe")
	bool Start();

	UFUNCTION(BlueprintCallable, Category="MediaPipe")
	void Stop();

protected:
	bool CreatePipeline();
	void ReleasePipeline();

	int16 GetVideoCaptureAPI(ECV_VideoCaptureAPI Value) const;

protected:
	class IUmpPipeline* Impl = nullptr;
	bool IsPipelineRunning = false;
	float StatAccum = 0;

	UPROPERTY()
	TArray<class UMediaPipeObserverComponent*> Observers;

	TUniquePtr<FDynamicTexture> VideoTexture;
};
