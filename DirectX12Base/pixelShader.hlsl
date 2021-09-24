
Texture2D texture0 : register(t0);
SamplerState sampler0 : register(s0);

struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float4 Normal   : NORMAL;
    float2 TexCoord : TEXCOORD;
    float4 Diffuse  : COLOR;
};



float4 main(PS_INPUT input) : SV_TARGET
{
    float4 tex0Color = texture0.Sample(sampler0, input.TexCoord);
    
    float3 normal = normalize(input.Normal.xyz);
    
    float3 lightDir = normalize(float3(1, -1, 1));
    
    float NDotL = saturate(dot(normal, -lightDir));//ランバート拡散
    
    float4 outDiffuse;  
    outDiffuse = input.Diffuse * tex0Color * NDotL;
    
    
    return outDiffuse;
}
