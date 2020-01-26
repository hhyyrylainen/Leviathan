// ------------------------------------ //
#include "PSO.h"

#include "DiligentCore/Graphics/GraphicsEngine/interface/ShaderResourceBinding.h"
using namespace Leviathan;
// ------------------------------------ //
// SRB
SRB::SRB(const Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding>& srb) : _SRB(srb) {}
// ------------------------------------ //
// PSO
PSO::PSO(const Diligent::RefCntAutoPtr<Diligent::IPipelineState>& pso) : _PSO(pso) {}
// ------------------------------------ //
std::shared_ptr<SRB> PSO::CreateShaderResourceBinding(bool createstatic /*= true*/)
{
    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> srb;

    _PSO->CreateShaderResourceBinding(&srb, createstatic);

    if(!srb)
        return nullptr;

    return std::make_shared<SRB>(srb);
}
