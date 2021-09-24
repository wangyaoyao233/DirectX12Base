cbuffer cbTansMatrix : register(b0)
{
    float4x4 WVP;
    float4x4 World;
};


struct VS_INPUT
{
    float3 Position : POSITION;
    float3 Normal   : NORMAL;
    float2 TexCoord : TEXCOORD;
};

struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float4 Normal   : NORMAL;
    float2 TexCoord : TEXCOORD;
    float4 Diffuse  : COLOR;
};


PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output;
      
    float4 position = float4(input.Position, 1.0f);
    output.Position = mul(position, WVP);
    
    float4 normal = float4(input.Normal, 0.0f);
    output.Normal = mul(normal, World);
    
    output.TexCoord = input.TexCoord;
    
    output.Diffuse.rgb = float3(1.0,1.0,1.0);
    output.Diffuse.a = 1.0;
    
    return output;
}

/*

//    // translate
//    matrix m_t = matrix(
//    1, 0, 0, 0,
//    0, 1, 0, 0,
//    0, 0, 1, 0,
//    100, 0, 0, 1
//);
  
//    // rotate z
//    float ang = 1;
//    matrix m_r = matrix(
//    cos(ang), sin(ang), 0, 0,
//    -sin(ang), cos(ang), 0, 0,
//    0, 0, 1, 0,
//    100, 0, 0, 1
//);


    float4 position = float4(input.Position, 1.0f);
    //position = mul(position, m_t);
    //position = mul(position, m_r);

*/