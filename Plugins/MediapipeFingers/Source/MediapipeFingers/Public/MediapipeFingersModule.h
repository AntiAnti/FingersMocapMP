// (c) Yuri Kalinin, 2023. All Rights Reserved
// ykasczc@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FMediapipeFingersModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
