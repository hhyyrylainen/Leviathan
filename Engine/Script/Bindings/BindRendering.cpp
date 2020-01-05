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
    ANGELSCRIPT_REGISTER_REF_TYPE("SceneAttachable", SceneAttachable);

    if(engine->RegisterObjectMethod("SceneAttachable", "void DetachFromParent()",
           asMETHOD(SceneAttachable, DetachFromParent), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    ANGELSCRIPT_REGISTER_REF_TYPE("SceneNode", SceneNode);

    ANGLESCRIPT_BASE_CLASS_CASTS(SceneAttachable, "SceneAttachable", SceneNode, "SceneNode");

    if(engine->RegisterObjectMethod("SceneNode", "void SetPosition(const Float3 &in position)",
           asMETHOD(SceneNode, SetPosition), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("SceneNode", "Float3 GetPosition() const",
           asMETHOD(SceneNode, GetPosition), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("SceneNode",
           "void SetOrientation(const Quaternion &in orientation)",
           asMETHOD(SceneNode, SetOrientation), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("SceneNode", "Quaternion GetOrientation() const",
           asMETHOD(SceneNode, GetOrientation), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("SceneNode", "void AttachObject(SceneAttachable@ object)",
           asMETHOD(SceneNode, AttachObjectWrapper), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("SceneNode", "bool DetachObject(SceneAttachable@ object)",
           asMETHOD(SceneNode, DetachObjectWrapper), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("SceneNode", "void DetachFromParent()",
           asMETHOD(SceneNode, DetachFromParent), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }


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
