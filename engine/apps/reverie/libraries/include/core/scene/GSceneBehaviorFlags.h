#pragma once

// Internal
#include "fortress/layer/framework/GFlags.h"

namespace rev {

enum class SceneBehaviorFlag {
    kEnabled = 1 << 0, // Whether or not the object is enabled
    kInvisible = 1 << 1, // Whether or not the object is visible
    kScriptGenerated = 1 << 2, // If the object is generated via script, right now (2/26/21) this just means Python 
    kMoved = 1 << 3 // If the object moved and needs transform uniforms updated
    //kStaleRenderSettings = 1 << 4 ///< If the render settings for the scene object changed and uniforms need to be updated 
};
MAKE_BITWISE(SceneBehaviorFlag);
MAKE_FLAGS(SceneBehaviorFlag, SceneBehaviorFlags);



} // end namespacing

