cbuffer cbTansMatrix : register(b0)
{
    float4x4 WVP;
    float4x4 World;
    float4 Param;

    //float4 LightDirection;
    //float4 CameraPostion;
};


struct VS_INPUT
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD;
};

struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float4 WorldPosition : POSITION0;
    float4 Normal : NORMAL;
    float2 TexCoord : TEXCOORD;
    float4 Diffuse : COLOR;
    float4 Depth : DEPTH;
};


PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output;
      
    float4 position = float4(input.Position, 1.0f);
    output.Position = mul(position, WVP);
    
    output.WorldPosition = mul(position, World);
    output.Depth = output.Position.z;
    
    float4 normal = float4(input.Normal, 0.0f);
    output.Normal = mul(normal, World);
    
    output.TexCoord = input.TexCoord;
    
    output.Diffuse.rgb = 1.0;
    output.Diffuse.a = 1.0;
    
    return output;
}