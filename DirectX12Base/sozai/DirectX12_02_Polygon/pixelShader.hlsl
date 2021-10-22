


struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float4 Normal   : NORMAL;
    float2 TexCoord : TEXCOORD;
    float4 Diffuse  : COLOR;
};



float4 main(PS_INPUT input) : SV_TARGET
{
	return input.Diffuse;
}
