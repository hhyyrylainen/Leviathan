options
{
    sort = backtofront;
    transparent = true;
};

shader ScreenSpaceGUI
{
    depth
    {
        write = false;
        read = false;
    };

    blend
    {
        target
        {
            enabled = true;
            color = { srcA, srcIA, add };
        };
    };

    raster
    {
        cull = none;
    };
    
    code
    {
        [alias(image)]
        SamplerState imageSampler
        {
            Filter = MIN_MAG_MIP_LINEAR;
            AddressU = Clamp;
            AddressV = Clamp;
        };
        
        Texture2D image = black;
        
        void vsmain(in float2 position : POSITION,
        	in float2 uv : TEXCOORD0,
            out float4 outPosition : SV_Position,
	        out float2 outUV : TEXCOORD0)
        {
            outUV = uv;
            outPosition = float4(position, 1, 1.f);
        }
        
        float4 fsmain(in float4 position : SV_Position, float2 uv : TEXCOORD0) : SV_Target0
        {
            return image.Sample(imageSampler, uv);
        }   
    };
};