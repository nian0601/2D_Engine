#pragma once

#include "FW_Assert.h"
#include "FW_GrowingArray.h"
#include "FW_Input.h"
#include "FW_Rect.h"
#include "FW_Renderer.h"
#include "FW_String.h"
#include "FW_Vector2.h"

#include "imgui/imgui.h"

typedef unsigned int FW_EntityID;
static const FW_EntityID InvalidEntity = (unsigned int)-1;
#define MAX_COMPONENTS 1024