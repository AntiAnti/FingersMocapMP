// (c) wongfei - https://github.com/wongfei
// 2021, All Rights Reserved. Apache License Version 2.0
// Modified and updated: YuriNK (ykasczc@gmail.com)

#pragma once

#include "ump_api.h"
#include "Components/ActorComponent.h"
#include "MediaPipeObserverComponent.generated.h"

class IMediaPipePipelineCallback
{
public:
	virtual void OnPipelineStarting(class IUmpPipeline* Pipeline) = 0;
	virtual void OnPipelineStopping(class IUmpPipeline* Pipeline) = 0;
};

/**
* Observer receives output stream from the Pipeline component's active graph
* This is a base class.
* See UMediaPipeClassificationObserverComponent, UMediaPipeLandmarkObserverComponent, UMediaPipeFaceMeshObserverComponent for implementation.
*/
UCLASS(ClassGroup="MediaPipe")
class MEDIAPIPE_API UMediaPipeObserverComponent : 
	public UActorComponent, 
	public IMediaPipePipelineCallback, 
	public IUmpPacketCallback
{
	GENERATED_BODY()

public:
	UMediaPipeObserverComponent();

	// Config

	// Should be equal to the same parameter of the Pipeline component
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MediaPipe|Observer")
	FString PipelineName;

	// Name of the output stream in Mediapipe graph
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MediaPipe|Observer")
	FString StreamName;

	// Automatically connect to pipline with specified name in the same actor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MediaPipe|Observer")
	bool bAllowAutoBind = true;

	// Runtime

	// Last time the packet was received
	UPROPERTY(BlueprintReadOnly, Category = "MediaPipe")
	float LastUpdate;

	// Getters

	// Get number of detected objects
	UFUNCTION(BlueprintPure, Category = "MediaPipe")
	int GetNumDetections();

	// Has any detected objects?
	UFUNCTION(BlueprintPure, Category = "MediaPipe")
	bool HaveDetections();

	int32 GetPacketCounter() const { return PacketCounter.GetValue(); }
	void ResetPacketCounter() { PacketCounter.Reset(); }

protected:
	void UpdateTimestamp();

protected:
	friend class UMediaPipePipelineComponent;
	void OnPipelineStarting(class IUmpPipeline* Pipeline) override;
	void OnPipelineStopping(class IUmpPipeline* Pipeline) override;
	void OnUmpPresence(class IUmpObserver* observer, bool present) override;
	void OnUmpPacket(class IUmpObserver* observer) override {}

protected:
	class IUmpObserver* Impl = nullptr;
	TAtomic<int> NumDetections = 0;
	FThreadSafeCounter PacketCounter;
};
