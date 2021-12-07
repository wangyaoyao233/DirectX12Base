
Texture2D<float4> normalTex : register(t0);
Texture2D<float4> colorTex : register(t1);
Texture2D<float4> worldPosTex : register(t2);
Texture2D<float4> depthTex : register(t3);
Texture2D<float4> envTex : register(t4);
Texture2D<float4> iblTex : register(t5);

SamplerState sampler0 : register(s0);

cbuffer cbTansMatrix : register(b0)
{
    float4x4 WVP;
    float4x4 World;
    float4 Param;

    //float4 LightDirection;
    //float4 CameraPostion;
};

struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float4 Normal : NORMAL;
    float2 TexCoord : TEXCOORD;
    float4 Diffuse : COLOR;
};



float4 main(PS_INPUT input) : SV_TARGET
{  
    float4 normal = normalize(normalTex.Sample(sampler0, input.TexCoord));
    float4 color = colorTex.Sample(sampler0, input.TexCoord);
    float4 worldPos = worldPosTex.Sample(sampler0, input.TexCoord);
    float4 depth = depthTex.Sample(sampler0, input.TexCoord);

    float4 outColor;

    {
        //ライティング
        float PI = 3.141592653589;
        float3 lightDir = normalize(float3(1, -1, 1));
        float3 cameraPos = float3(0, 2, -5);
        float3 eyev = normalize(worldPos.xyz - cameraPos);
        float3 eyeRefV = normalize(reflect(eyev, normal.xyz));
        float3 lightRefV = normalize(reflect(lightDir, normal.xyz));

        ////マテリアル
        //float roughness = 0.0;
        //float metallic = 1.0;
        //float spec = 0.9;

        float roughness = Param.x;
        float metallic = Param.y;
        float spec = Param.z;

        ////IBL
        float2 iblTexCoord;
        iblTexCoord.x = atan2(normal.x, normal.z) / (PI * 2) + 0.5;
        iblTexCoord.y = acos(normal.y) / PI;
        float3 diffuse = color.rgb * iblTex.Sample(sampler0, iblTexCoord).rgb / PI;
             
      
        //// depth fog
        //float3 fogColor = float3(0, 0, 0);
        //outDiffuse.rgb = lerp(outDiffuse.rgb, fogColor.rgb, depth.x / 15);
    
        //// 環境マッピング  
        float2 envTexCoord;
        envTexCoord.x = atan2(eyeRefV.x, eyeRefV.z) / (PI * 2) + 0.5;
        envTexCoord.y = acos(eyeRefV.y) / PI;

        //フレネル反射率
        float3 f0 = lerp(0.08 * spec, color.rgb, metallic);
        float3 f = f0 + (1.0 - f0) * pow(1.0 - dot(normal.xyz, eyeRefV.xyz), 5);

        //法線分布(GGX)
        float a = roughness * roughness;
        float d = a * a / (PI * (1.0 * (a * a - 1.0) + 1.0) * (1.0 * (a * a - 1.0) + 1.0));
        d = 1.0;

        //幾何減衰(Schlick)
        float k = (roughness + 1) * (roughness + 1) / 8.0;
        float gv = dot(normal.xyz, eyeRefV.xyz) / (dot(normal.xyz, eyeRefV.xyz) * (1.0 - k) + k);
        float g = gv * gv;

        //スペキュラー(Cook-Torrance)
        float3 specular = envTex.SampleBias(sampler0, envTexCoord, roughness * 13.0).rgb;
        specular = specular * d * f * g / (4.0 * dot(normal.xyz, eyeRefV.xyz) * 1.0);

        //color.rgb += envTex.Sample(sampler0, envTexCoord).rgb * f; //auto
        //color.rgb += envTex.SampleBias(sampler0, envTexCoord, 3.0).rgb * f;
        //color.rgb = envTex.SampleBias(sampler0, envTexCoord, 3.0).rgb; //test
    

        outColor.rgb = diffuse * (1.0 - metallic) + specular;
        outColor.a = color.a;

        //明度調整
        outColor.rgb *= PI * 1.5;
    }

    //point lights
    {
        if (Param.w == 1.0)
        {
            float3 lightPosition[3];
            lightPosition[0] = float3(2.0, 2.0, -2.0);
            lightPosition[1] = float3(-2.0, 1.0, -1.0);
            lightPosition[2] = float3(1.0, 2.0, 2.0);

            float3 lightColor[3];
            lightColor[0] = float3(1.0, 0.0, 0.0);
            lightColor[1] = float3(0.0, 1.0, 0.0);
            lightColor[2] = float3(0.0, 0.0, 1.0);

            for (int i = 0; i < 3; i++)
            {
                float3 lightDirection = worldPos.xyz - lightPosition[i];
                float lightLength = length(lightDirection);

                lightDirection = normalize(lightDirection);

                float light = saturate(-dot(normal.xyz, lightDirection));

                outColor.rgb += lightColor[i] * light / lightLength * 3.0;
            }
        }   
    }
    
    return outColor;
}
