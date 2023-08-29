// (c) wongfei - https://github.com/wongfei
// 2021, All Rights Reserved. Apache License Version 2.0
// Modified and updated: YuriNK (ykasczc@gmail.com)

#pragma once

#include "MediaPipeObserverComponent.h"
#include "MediaPipePresenceObserverComponent.generated.h"

UCLASS(ClassGroup="MediaPipe", meta=(BlueprintSpawnableComponent))
class MEDIAPIPE_API UMediaPipePresenceObserverComponent : public UMediaPipeObserverComponent
{
	GENERATED_BODY()

public:
	UMediaPipePresenceObserverComponent();

	UPROPERTY(Category="MediaPipe", BlueprintReadOnly)
	bool bPresent;

protected:
	void OnUmpPacket(class IUmpObserver* Observer) override;
};
