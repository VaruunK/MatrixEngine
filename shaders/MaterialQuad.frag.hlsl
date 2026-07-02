Texture2D BaseColor : register(t0, space2);
Texture2D Normal : register(t1, space2);
Texture2D ORM : register(t2, space2);
Texture2D Emissive : register(t3, space2);

SamplerState BaseColorSampler : register(s0, space2);
SamplerState NormalSampler : register(s1, space2);
SamplerState ORMSampler : register(s2, space2);
SamplerState EmissiveSampler : register(s3, space2);

float4 main(float2 TexCoord : TEXCOORD0) : SV_Target
{
    float3 baseColor = BaseColor.Sample(BaseColorSampler, TexCoord).rgb;
    float3 emissive = Emissive.Sample(EmissiveSampler, TexCoord).rgb;
    float3 normal = Normal.Sample(NormalSampler, TexCoord).xyz * 2.0f - 1.0f;
    float3 orm = ORM.Sample(ORMSampler, TexCoord).rgb;

    float ao = orm.r;
    float roughness = orm.g;
    float metallic = orm.b;

    float3 debugInfluence = normal * 0.0001f + float3(ao, roughness, metallic) * 0.0001f;

    return float4(baseColor + emissive + debugInfluence, 1.0f);
}