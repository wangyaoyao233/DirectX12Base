
Texture2D<float4> texture0 : register(t0);
SamplerState sampler0 : register(s0);

struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float4 WorldPosition : POSITION0;
    float4 Normal : NORMAL;
    float2 TexCoord : TEXCOORD;
    float4 Diffuse : COLOR;
};

struct PS_OUTPUT
{
    float4 Normal : SV_TARGET0;
    float4 Diffuse : SV_TARGET1;
    float4 WorldPosition : SV_TARGET2;
    float4 Depth : SV_TARGET3;
};


PS_OUTPUT main(PS_INPUT input)
{
    PS_OUTPUT output;
    
    output.Normal = input.Normal;
    output.Diffuse = texture0.Sample(sampler0, input.TexCoord);   
    output.WorldPosition = input.WorldPosition;
        
    //we get the depth value for this pixel, note that it is stored as z/w so we perform the necessary division to get the depth.
	// Get the depth value of the pixel by dividing the Z pixel depth by the homogeneous W coordinate.
    output.Depth = input.Position.z / input.Position.w;
       
    return output;
}
