
Texture2D<float4> normalTex : register(t0);
Texture2D<float4> colorTex : register(t1);
Texture2D<float4> worldPosTex : register(t2);
Texture2D<float4> depthTex : register(t3);


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
    float4 normal = normalTex.Sample(sampler0, input.TexCoord);
    float4 color = colorTex.Sample(sampler0, input.TexCoord);
    float4 worldPos = worldPosTex.Sample(sampler0, input.TexCoord);
    float4 depth = depthTex.Sample(sampler0, input.TexCoord);
    
    normal = normalize(normal);
    
    float3 lightDir = normalize(float3(1, -1, 1));
    //float3 lightDir = normalize(LightDirection.xyz);
    
    float NDotL = saturate(-dot(normal.xyz, lightDir)); //ランバート拡散
    
    float4 outDiffuse;
    outDiffuse = color * NDotL;
     
    //// specular
    float3 cameraPos = float3(0, 2, -5);
    //float3 eyev = normalize(input.WorldPostion.xyz - CameraPostion.xyz);
    float3 eyev = normalize(worldPos.xyz - cameraPos);
    float3 refv = normalize(reflect(lightDir, normal.xyz));
    float spec = saturate(-dot(eyev, refv));
    spec = pow(spec, 30);
    outDiffuse.rgb += spec;
      
    // fog
    float4 fogColor = float4(0, 0, 0, 1);
    float4 finalColor;
    finalColor.rgb = lerp(outDiffuse.rgb, fogColor.rgb, depth.x / 15);
    finalColor.a = outDiffuse.a;
    
    
    return finalColor;
}
