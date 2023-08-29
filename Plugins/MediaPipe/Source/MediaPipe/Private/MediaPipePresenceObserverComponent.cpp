// (c) wongfei - https://github.com/wongfei
// 2021, All Rights Reserved. Apache License Version 2.0
// Modified and updated: YuriNK (ykasczc@gmail.com)

#include "MediaPipePresenceObserverComponent.h"
#include "MediaPipePipelineComponent.h"
#include "MediaPipeShared.h"

UMediaPipePresenceObserverComponent::UMediaPipePresenceObserverComponent()
{
}

void UMediaPipePresenceObserverComponent::OnUmpPacket(IUmpObserver* Observer)
{
	if (!UmpCompareType<bool>(Observer))
	{
		PLUGIN_LOG(Error, TEXT("Invalid Presence message"));
		return;
	}

	bPresent = UmpCastPacket<bool>(Observer->GetData());
	UpdateTimestamp();
}
