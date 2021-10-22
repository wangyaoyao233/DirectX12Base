
Texture2D<float4> texture0 : register(t0);
Texture2D<float4> texture1 : register(t1);


SamplerState sampler0 : register(s0);


struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float4 Normal : NORMAL;
    float2 TexCoord : TEXCOORD;
    float4 Diffuse : COLOR;
};



float4 main(PS_INPUT input) : SV_TARGET
{
    float4 normal = texture0.Sample(sampler0, input.TexCoord);
    float4 color = texture1.Sample(sampler0, input.TexCoord);
    
    normal = normalize(normal);
    
    float3 lightDir = normalize(float3(1, -1, 1));
    //float3 lightDir = normalize(LightDirection.xyz);
    
    float NDotL = saturate(-dot(normal.xyz, lightDir)); //ランバート拡散
    
    float4 outDiffuse;
    outDiffuse = color * NDotL;
     
    //// specular
    //float3 eyev = normalize(input.WorldPostion.xyz - CameraPostion.xyz);
    //float3 refv = normalize(reflect(lightDir, normal.xyz));
    //float spec = saturate(-dot(eyev, refv));
    //spec = pow(spec, 30);
    //outDiffuse.rgb += spec;
      
    
    return outDiffuse;
}
