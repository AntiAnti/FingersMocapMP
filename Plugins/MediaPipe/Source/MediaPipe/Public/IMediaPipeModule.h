// (c) wongfei - https://github.com/wongfei
// 2021, All Rights Reserved. Apache License Version 2.0
// Modified and updated: YuriNK (ykasczc@gmail.com)

#pragma once

#include "Modules/ModuleManager.h"

class MEDIAPIPE_API IMediaPipeModule : public IModuleInterface
{
public:

	static inline IMediaPipeModule& Get()
	{
		return FModuleManager::LoadModuleChecked<IMediaPipeModule>("MediaPipe");
	}

	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("MediaPipe");
	}

	virtual class IUmpPipeline* CreatePipeline() = 0;
};
