// ------------------------------------ //
#include "BindDefinitions.h"

#include "Rendering/Graphics.h"
#include "Rendering/Material.h"
#include "Rendering/Model.h"
#include "Rendering/Renderable.h"
#include "Rendering/Scene.h"
#include "Rendering/Shader.h"
#include "Rendering/Texture.h"

#include "Engine.h"

using namespace Leviathan;
// ------------------------------------ //

// Proxies etc.
// ------------------------------------ //
Shader* ShaderFromNameFactory(const std::string& name)
{
    // bs::HShader bsShader;

    // if(name == "BuiltinShader::Standard") {

    //     bsShader = bs::gBuiltinResources().getBuiltinShader(bs::BuiltinShader::Standard);
    // } else if(name == "BuiltinShader::Transparent") {
    //     bsShader = bs::gBuiltinResources().getBuiltinShader(bs::BuiltinShader::Transparent);
    // } else {
    //     bsShader = Engine::Get()->GetGraphics()->LoadShaderByName(name);
    // }

    // if(!bsShader) {
    //     asGetActiveContext()->SetException("bsf shader load failed");
    //     return nullptr;
    // }

    auto result = Engine::Get()->GetGraphics()->LoadShaderByName(name);

    if(result)
        result->AddRef();

    return result.get();
}

Material* MaterialFromShaderFactory(Shader* shader)
{
    auto material = Material::MakeShared<Material>(Shader::WrapPtr(shader));

    if(material)
        material->AddRef();

    return material.get();
}

Material* MaterialEmptyFactory()
{
    auto material = Material::MakeShared<Material>();

    if(material)
        material->AddRef();

    return material.get();
}

Texture* TextureFromNameFactory(const std::string& name)
{
    auto result = Engine::Get()->GetGraphics()->LoadTextureByName(name);

    if(!result) {
        asGetActiveContext()->SetException(
            "no texture could be loaded with the specified name");
        return nullptr;
    }

    if(result)
        result->AddRef();

    return result.get();
}

Rendering::Model* ModelFromNameFactory(const std::string& name)
{
    auto result = Engine::Get()->GetGraphics()->LoadModelByName(name);

    if(!result) {
        asGetActiveContext()->SetException("no model could be loaded with the specified name");
        return nullptr;
    }

    if(result)
        result->AddRef();

    return result.get();
}

namespace Leviathan {

bool BindShader(asIScriptEngine* engine)
{
    ANGELSCRIPT_REGISTER_REF_TYPE("Shader", Shader);

    if(engine->RegisterObjectBehaviour("Shader", asBEHAVE_FACTORY,
           "Shader@ f(const string &in name)", asFUNCTION(ShaderFromNameFactory),
           asCALL_CDECL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }


    return true;
}

bool BindTexture(asIScriptEngine* engine)
{
    ANGELSCRIPT_REGISTER_REF_TYPE("Texture", Texture);

    if(engine->RegisterObjectBehaviour("Texture", asBEHAVE_FACTORY,
           "Texture@ f(const string &in name)", asFUNCTION(TextureFromNameFactory),
           asCALL_CDECL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    return true;
}

bool BindModel(asIScriptEngine* engine)
{
    if(engine->SetDefaultNamespace("Rendering") < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    ANGELSCRIPT_REGISTER_REF_TYPE("Model", Rendering::Model);

    if(engine->RegisterObjectBehaviour("Model", asBEHAVE_FACTORY,
           "Model@ f(const string &in name)", asFUNCTION(ModelFromNameFactory),
           asCALL_CDECL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->SetDefaultNamespace("") < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    return true;
}

bool BindMaterial(asIScriptEngine* engine)
{
    ANGELSCRIPT_REGISTER_REF_TYPE("Material", Material);

    if(engine->RegisterObjectBehaviour("Material", asBEHAVE_FACTORY,
           "Material@ f(Shader@ shader)", asFUNCTION(MaterialFromShaderFactory),
           asCALL_CDECL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectBehaviour("Material", asBEHAVE_FACTORY, "Material@ f()",
           asFUNCTION(MaterialEmptyFactory), asCALL_CDECL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Material",
           "void SetTexture(const string &in parameter, Texture@ texture)",
           asMETHOD(Material, SetTextureWrapper), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

    if(engine->RegisterObjectMethod("Material",
           "void SetFloat4(const string &in parameter, const Float4 &in data)",
           asMETHOD(Material, SetFloat4), asCALL_THISCALL) < 0) {
        ANGELSCRIPT_REGISTERFAIL;
    }

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

    if(!BindTexture(engine))
        return false;

    if(!BindMaterial(engine))
        return false;

    if(!BindModel(engine))
        return false;

    if(!BindSceneNode(engine))
        return false;

    if(!BindScene(engine))
        return false;

    if(!BindRenderable(engine))
        return false;

    return true;
}
