// ------------------------------------ //
#include "BindDefinitions.h"

#include "Rendering/Material.h"
#include "Rendering/Renderable.h"
#include "Rendering/Scene.h"
#include "Rendering/Shader.h"
#include "Rendering/Texture.h"


using namespace Leviathan;
// ------------------------------------ //

// Proxies etc.
// ------------------------------------ //


namespace Leviathan {

bool BindShader(asIScriptEngine* engine)
{
    ANGELSCRIPT_REGISTER_REF_TYPE("Shader", Shader);

    return true;
}

bool BindMaterial(asIScriptEngine* engine)
{
    ANGELSCRIPT_REGISTER_REF_TYPE("Material", Material);

    return true;
}

bool BindSceneNode(asIScriptEngine* engine)
{
    ANGELSCRIPT_REGISTER_REF_TYPE("SceneNode", SceneNode);

    return true;
}

bool BindScene(asIScriptEngine* engine)
{
    ANGELSCRIPT_REGISTER_REF_TYPE("Scene", Scene);

    return true;
}

bool BindRenderable(asIScriptEngine* engine)
{
    ANGELSCRIPT_REGISTER_REF_TYPE("Renderable", Renderable);

    return true;
}

} // namespace Leviathan

bool Leviathan::BindRendering(asIScriptEngine* engine)
{
    if(!BindShader(engine))
        return false;

    if(!BindMaterial(engine))
        return false;

    if(!BindSceneNode(engine))
        return false;

    if(!BindScene(engine))
        return false;

    if(!BindRenderable(engine))
        return false;

    return true;
}
