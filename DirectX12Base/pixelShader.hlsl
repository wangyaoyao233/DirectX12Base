
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
    return input.Diffuse * texture0.Sample(sampler0, input.TexCoord);
}
