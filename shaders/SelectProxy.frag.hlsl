cbuffer UniformBlock : register(b0, space3) {
    float4 IdColor;
};

float4 main() : SV_Target
{
    return IdColor;
}