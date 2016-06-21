#pragma once
//! \file Supresses warnings caused by including CEGUI

#include <glm/glm.hpp>
#if (GLM_VERSION_MAJOR == 0) && (GLM_VERSION_MINOR == 9) && (GLM_VERSION_PATCH == 5)
#define GLM_FORCE_RADIANS
#endif

#include "CEGUI/AnimationInstance.h"
#include "CEGUI/AnimationManager.h"
#include "CEGUI/Clipboard.h"
#include "CEGUI/FontManager.h"
#include "CEGUI/GUIContext.h"
#include "CEGUI/InputAggregator.h"
#include "CEGUI/InputEvent.h"
#include "CEGUI/RenderTarget.h"
#include "CEGUI/RendererModules/Ogre/Renderer.h"
#include "CEGUI/SchemeManager.h"
#include "CEGUI/System.h"
#include "CEGUI/Window.h"
#include "CEGUI/WindowManager.h"
#include "CEGUI/widgets/Combobox.h"
#include "CEGUI/widgets/FrameWindow.h"
#include "CEGUI/widgets/ListWidget.h"
#include "CEGUI/widgets/ListboxItem.h"
#include "CEGUI/widgets/PushButton.h"
#include "CEGUI/widgets/TabControl.h"


