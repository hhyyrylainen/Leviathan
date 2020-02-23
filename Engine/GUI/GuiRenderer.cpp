// ------------------------------------ //
#include "GuiRenderer.h"

#include "Common/Matrix.h"
#include "Rendering/Buffer.h"
#include "Rendering/GeometryHelpers.h"
#include "Rendering/Graphics.h"
#include "Rendering/PSO.h"
#include "Rendering/Shader.h"
#include "Rendering/Texture.h"

#include "GUI/GuiShaders.h"

using namespace Leviathan;
using namespace Leviathan::GUI;
// ------------------------------------ //
// GuiRenderer::Implementation
struct GuiRenderer::Implementation {
    //! \brief Collects rendering resources for a certain type of GUI rendering
    class RenderType {
    public:
        std::shared_ptr<PSO> _PSO;
        std::shared_ptr<Rendering::Buffer> ViewBuffer;
        std::shared_ptr<Rendering::Buffer> PSConstants;
        std::shared_ptr<SRB> _SRB;

        // Cached lookup of this
        Diligent::IShaderResourceVariable* TextureVariable = nullptr;
    };

public:
    Implementation(GuiRenderer& owner) : Owner(owner) {}

    void InitShaders()
    {
        Diligent::ShaderCreateInfo info;
        info.SourceLanguage = Diligent::SHADER_SOURCE_LANGUAGE_HLSL;
        info.UseCombinedTextureSamplers = true;
        info.EntryPoint = "main";

        {
            info.Desc.ShaderType = Diligent::SHADER_TYPE_VERTEX;
            info.Desc.Name = "TransparentAlpha GUI vertex";
            info.Source = TextureAlphaVSSource;

            MakeShader(info, TransparentAlphaShader);
        }
        {
            info.Desc.ShaderType = Diligent::SHADER_TYPE_PIXEL;
            info.Desc.Name = "TransparentAlpha GUI pixel";
            info.Source = TextureAlphaPSSource;

            MakeShader(info, TransparentAlphaShaderPS);
        }

        {
            info.Desc.ShaderType = Diligent::SHADER_TYPE_VERTEX;
            info.Desc.Name = "TransparentColour GUI vertex";
            info.Source = TextureVertexColourVSSource;

            MakeShader(info, VertexColourShader);
        }
        {
            info.Desc.ShaderType = Diligent::SHADER_TYPE_PIXEL;
            info.Desc.Name = "TransparentColour GUI pixel";
            info.Source = TextureVertexColourPSSource;

            MakeShader(info, VertexColourShaderPS);
        }
    }

    void InitTransparentAlphaPSO()
    {
        TransparentAlpha = std::make_unique<RenderType>();

        Diligent::PipelineStateDesc PSODesc;
        PSODesc.Name = "TransparentAlpha GUI PSO";

        PSODesc.IsComputePipeline = false;
        PSODesc.GraphicsPipeline.NumRenderTargets = 1;

        // Set the render operation target format to the backbuffer one
        PSODesc.GraphicsPipeline.RTVFormats[0] = Owner.Graph->GetBackBufferFormat();
        // No depth as transparency can't use that
        // PSODesc.GraphicsPipeline.DSVFormat = Owner.Graph->GetDepthBufferFormat();
        PSODesc.GraphicsPipeline.DepthStencilDesc.DepthEnable = false;

        PSODesc.GraphicsPipeline.PrimitiveTopology =
            Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

        // Alpha blend
        auto& rt0Blend = PSODesc.GraphicsPipeline.BlendDesc.RenderTargets[0];
        rt0Blend.BlendEnable = true;
        rt0Blend.SrcBlend = Diligent::BLEND_FACTOR_SRC_ALPHA;
        rt0Blend.DestBlend = Diligent::BLEND_FACTOR_INV_SRC_ALPHA;
        rt0Blend.BlendOp = Diligent::BLEND_OPERATION_ADD;
        rt0Blend.SrcBlendAlpha = Diligent::BLEND_FACTOR_INV_SRC_ALPHA;
        rt0Blend.DestBlendAlpha = Diligent::BLEND_FACTOR_ZERO;
        rt0Blend.BlendOpAlpha = Diligent::BLEND_OPERATION_ADD;
        rt0Blend.RenderTargetWriteMask = Diligent::COLOR_MASK_ALL;

        // PSODesc.GraphicsPipeline.RasterizerDesc.CullMode = Diligent::CULL_MODE_BACK;
        // No culling used on the GUI
        PSODesc.GraphicsPipeline.RasterizerDesc.CullMode = Diligent::CULL_MODE_NONE;

        PSODesc.GraphicsPipeline.pVS = TransparentAlphaShader->GetFirstVariant();
        PSODesc.GraphicsPipeline.pPS = TransparentAlphaShaderPS->GetFirstVariant();

        // Shader resources
        // TODO: determine if it is better to update a single SRB than to have one per GUI
        // object. Currently a single one is updated
        Diligent::ShaderResourceVariableDesc Vars[] = {{Diligent::SHADER_TYPE_PIXEL,
            "g_Texture", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC}};
        PSODesc.ResourceLayout.Variables = Vars;
        PSODesc.ResourceLayout.NumVariables = std::size(Vars);

        // We don't change the texture sampler
        Diligent::SamplerDesc SamLinearClampDesc{Diligent::FILTER_TYPE_LINEAR,
            Diligent::FILTER_TYPE_LINEAR, Diligent::FILTER_TYPE_LINEAR,
            Diligent::TEXTURE_ADDRESS_CLAMP, Diligent::TEXTURE_ADDRESS_CLAMP,
            Diligent::TEXTURE_ADDRESS_CLAMP};
        Diligent::StaticSamplerDesc StaticSamplers[] = {
            {Diligent::SHADER_TYPE_PIXEL, "g_Texture", SamLinearClampDesc}};
        PSODesc.ResourceLayout.StaticSamplers = StaticSamplers;
        PSODesc.ResourceLayout.NumStaticSamplers = std::size(StaticSamplers);

        const auto [elements, elementCount] =
            GeometryHelpers::GetLayoutForQuad().GetElements();

        PSODesc.GraphicsPipeline.InputLayout.LayoutElements = elements;
        PSODesc.GraphicsPipeline.InputLayout.NumElements = elementCount;

        // End setting up options. Create PSO
        TransparentAlpha->_PSO = Owner.Graph->CreatePSO(PSODesc);

        LEVIATHAN_ASSERT(TransparentAlpha->_PSO, "GuiView PSO creation failed");

        // Uniform constant buffer setup
        Diligent::BufferDesc bufferDesc;
        bufferDesc.Name = "GUI VS constants CB";
        bufferDesc.uiSizeInBytes = sizeof(Matrix4);
        bufferDesc.Usage = Diligent::USAGE_DYNAMIC;
        bufferDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
        bufferDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;

        TransparentAlpha->ViewBuffer = Owner.Graph->CreateBuffer(bufferDesc);

        // The buffer is bound permanently (but the buffer contents can be written)
        // Not sure if this is the right way to go about making a GUI renderer. See above
        // comment on how SRBs should be used
        TransparentAlpha->_PSO->GetInternal()
            ->GetStaticVariableByName(Diligent::SHADER_TYPE_VERTEX, "Constants")
            ->Set(TransparentAlpha->ViewBuffer->GetInternal().RawPtr());

        bufferDesc.Name = "GUI PS constants CB";
        // Must be a multiple of 16, and as a single float < 16 we just set it to 16
        bufferDesc.uiSizeInBytes = 16;
        // TODO: this could use USAGE_DEFAULT to skip writing when not needed. but maybe that
        // is actually slower?
        bufferDesc.Usage = Diligent::USAGE_DYNAMIC;

        TransparentAlpha->PSConstants = Owner.Graph->CreateBuffer(bufferDesc);

        TransparentAlpha->_PSO->GetInternal()
            ->GetStaticVariableByName(Diligent::SHADER_TYPE_PIXEL, "Constants")
            ->Set(TransparentAlpha->PSConstants->GetInternal().RawPtr());

        // Shader resource binding object with static bindings initialized
        TransparentAlpha->_SRB = TransparentAlpha->_PSO->CreateShaderResourceBinding(true);
    }

    void UpdateTechniqueBuffers(RenderType& type, float xoffset, float yoffset, float alpha)
    {
        {
            Rendering::MappedBuffer mapped(*Owner.Graph, *type.ViewBuffer, Diligent::MAP_WRITE,
                Diligent::MAP_FLAG_DISCARD);

            // Orthographic projection (setting last param to 0 disables depth adjustment)
            const auto projectionMatrix = Matrix4::ProjectionOrthographic(
                0.f - xoffset, 100.f - xoffset, 0.f - yoffset, 100.f - yoffset, -100.f, 100.f)
                                              .Transpose();

            mapped.Write(&projectionMatrix, sizeof(projectionMatrix));
        }

        // TODO: maybe it is possible to skip setting this if this hasn't changed?
        {
            Rendering::MappedBuffer mapped(*Owner.Graph, *type.PSConstants,
                Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);

            mapped.Write(&alpha, sizeof(alpha));
        }
    }

    void RenderWithTechnique(RenderType& type, Mesh& mesh, Texture& texture)
    {
        Owner.Graph->SetActivePSO(*type._PSO);

        if(!type.TextureVariable) {
            type.TextureVariable = type._SRB->GetInternal()->GetVariableByName(
                Diligent::SHADER_TYPE_PIXEL, "g_Texture");

            LEVIATHAN_ASSERT(type.TextureVariable, "gui shader texture not found");
        }

        type.TextureVariable->Set(texture.GetDefaultSRV());

        // TODO: find out if the resources could be pre-transitioned somehow
        Owner.Graph->CommitShaderResources(
            type._SRB->GetInternal(), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        Owner.Graph->DrawMesh(mesh);
    }

private:
    void MakeShader(Diligent::ShaderCreateInfo& info, CountedPtr<Shader>& target)
    {
        target = Owner.Graph->CreateShader(info, ShaderVariationInfo{});

        if(!target) {
            LOG_FATAL(
                std::string("Failed to compile GUI rendering shader: ") + info.Desc.Name);
        }
    }

public:
    GuiRenderer& Owner;

    std::unique_ptr<RenderType> TransparentAlpha;

    // Not setup properly yet
    std::unique_ptr<RenderType> VertexColour;


    // Shader objects. These are separate from the render types because these are created at
    // different time than PSOs
    CountedPtr<Shader> TransparentAlphaShader;
    CountedPtr<Shader> TransparentAlphaShaderPS;

    CountedPtr<Shader> VertexColourShader;
    CountedPtr<Shader> VertexColourShaderPS;
};
// ------------------------------------ //
// GuiRenderer
GuiRenderer::GuiRenderer(GuiManager& owner) : Owner(owner) {}

GuiRenderer::~GuiRenderer() {}
// ------------------------------------ //
void GuiRenderer::Init(Graphics* graphics, Window* window)
{
    Graph = graphics;
    Wind = window;

    Pimpl = std::make_unique<Implementation>(*this);

    // Load all shaders here to not stutter when new GUI things popup
    // TODO: maybe these could even be shared between windows
    Pimpl->InitShaders();
}
// ------------------------------------ //
DLLEXPORT void GuiRenderer::DrawTransparentWithAlpha(Mesh& mesh, Texture& texture,
    float xoffset /*= 0.f*/, float yoffset /*= 0.f*/, float alpha /*= 1.f*/)
{
    if(!Pimpl->TransparentAlpha)
        Pimpl->InitTransparentAlphaPSO();

    Pimpl->UpdateTechniqueBuffers(*Pimpl->TransparentAlpha, xoffset, yoffset, alpha);

    Pimpl->RenderWithTechnique(*Pimpl->TransparentAlpha, mesh, texture);
}
// ------------------------------------ //
void GuiRenderer::OnBeginRendering()
{
    // If shaders are not compatible between render contexts this is where the shaders need to
    // be setup instead
}

void GuiRenderer::OnEndRendering() {}
