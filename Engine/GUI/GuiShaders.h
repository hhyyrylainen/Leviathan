// Leviathan Game Engine
// Copyright (c) 2012-2020 Henri Hyyryläinen
//! \file Contains shader code strings for the GuiRenderer

namespace Leviathan {

// ------------------------------------ //
// TextureAlpha
constexpr auto TextureAlphaVSSource = R"(
    cbuffer Constants
    {
        float4x4 g_ProjectionMatrix;
    };

    struct VSInput
    {
        float2 Pos : ATTRIB0;
        float2 UV  : ATTRIB1;
    };

    struct PSInput 
    { 
        float4 Pos : SV_POSITION;
        float2 UV  : TEX_COORD; 
    };

    void main(in VSInput VSIn, out PSInput PSIn)
    {
        PSIn.Pos = mul(g_ProjectionMatrix, float4(VSIn.Pos.xy, 0.0, 1.0));
        PSIn.UV = VSIn.UV;
    }
    )";

constexpr auto TextureAlphaPSSource = R"(
    cbuffer Constants
    {
        float g_Alpha;
    };
    
    Texture2D g_Texture;
    SamplerState g_Texture_sampler;

    struct PSInput 
    { 
        float4 Pos : SV_POSITION;
        float2 UV : TEX_COORD; 
    };

    float4 main(in PSInput PSIn) : SV_Target
    {
        return g_Texture.Sample(g_Texture_sampler, PSIn.UV) * g_Alpha;
    }
    )";

// ------------------------------------ //
// TextureVertexColour (also with alpha)
constexpr auto TextureVertexColourVSSource = R"(
    cbuffer Constants
    {
        float4x4 g_ProjectionMatrix;
    };

    struct VSInput
    {
        float2 Pos : ATTRIB0;
        float2 UV  : ATTRIB1;
        float4 Col : ATTRIB2;
    };

    struct PSInput 
    { 
        float4 Pos : SV_POSITION;
        float4 Col : COLOR;
        float2 UV  : TEX_COORD; 
    };

    void main(in VSInput VSIn, out PSInput PSIn)
    {
        PSIn.Pos = mul(g_ProjectionMatrix, float4(VSIn.Pos.xy, 0.0, 1.0));
        PSIn.UV = VSIn.UV;
        PSIn.Col = VSIn.Col;
    }
    )";

constexpr auto TextureVertexColourPSSource = R"(
    cbuffer Constants
    {
        float g_Alpha;
    };
    
    Texture2D g_Texture;
    SamplerState g_Texture_sampler;

    struct PSInput 
    { 
        float4 Pos : SV_POSITION;
        float4 Col : COLOR;
        float2 UV : TEX_COORD; 
    };

    float4 main(in PSInput PSIn) : SV_Target
    {
        return PSIn.Col * g_Texture.Sample(g_Texture_sampler, PSIn.UV) * g_Alpha;
    }
    )";
} // namespace Leviathan
