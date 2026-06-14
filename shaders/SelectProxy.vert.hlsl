cbuffer UniformBlock : register(b0, space1) {
    float4x4 MatrixTransform : packoffset(c0);
};

struct Input {
    float4 Position : TEXCOORD0;
};

struct Output {
    float4 Position : SV_Position;
};

Output main(Input input) {
    Output output;
    output.Position = mul(MatrixTransform, input.Position);
    return output;
}