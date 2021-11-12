
Texture2D<float4> normalTex : register(t0);
Texture2D<float4> colorTex : register(t1);
Texture2D<float4> worldPosTex : register(t2);
Texture2D<float4> depthTex : register(t3);
Texture2D<float4> envTex : register(t4);

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
    float4 outDiffuse;
    
    float4 normal = normalTex.Sample(sampler0, input.TexCoord);
    float4 color = colorTex.Sample(sampler0, input.TexCoord);
    float4 worldPos = worldPosTex.Sample(sampler0, input.TexCoord);
    float4 depth = depthTex.Sample(sampler0, input.TexCoord);
    
    //// ランバート拡散 
    normal = normalize(normal);   
    float3 lightDir = normalize(float3(1, -1, 1)); 
    float NDotL = saturate(-dot(normal.xyz, lightDir));   
    outDiffuse = color * NDotL;
     
    //// specular
    float3 cameraPos = float3(0, 2, -5);
    float3 eyev = normalize(worldPos.xyz - cameraPos);
    float3 lightRefV = normalize(reflect(lightDir, normal.xyz));
    float spec = saturate(-dot(eyev, lightRefV));
    spec = pow(spec, 30);
    outDiffuse.rgb += spec;
      
    //// depth fog
    //float3 fogColor = float3(0, 0, 0);
    //outDiffuse.rgb = lerp(outDiffuse.rgb, fogColor.rgb, depth.x / 15);
    
    //// 環境マッピング
    float3 eyeRefV = normalize(reflect(eyev, normal.xyz));

    //フレネル反射率
    float f0 = 0.0;
    float f = f0 + (1.0 - f0) * pow(1.0 - dot(normal.xyz, eyeRefV.xyz), 5);
    
    float2 envTexCoord;
    float PI = 3.141592653589;
    envTexCoord.x = atan2(eyeRefV.x, eyeRefV.z) / (PI * 2) + 0.5;
    envTexCoord.y = acos(eyeRefV.y) / PI;
    
    outDiffuse.rgb += envTex.Sample(sampler0, envTexCoord).rgb * f;
    
    
    return outDiffuse;
}
