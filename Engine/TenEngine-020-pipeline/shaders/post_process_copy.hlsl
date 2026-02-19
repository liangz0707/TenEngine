/**
 * @file post_process_copy.hlsl
 * @brief Simple copy shader for post-processing pass-through.
 *
 * Copies input texture to output without modification.
 * Used for debug visualization and as a base for other effects.
 */

Texture2D InputTex : register(t0);
SamplerState LinearClamp : register(s0);

struct VSInput {
    float2 Position : POSITION;
    float2 TexCoord : TEXCOORD0;
};

struct VSOutput {
    float4 Position : SV_Position;
    float2 TexCoord : TEXCOORD0;
};

struct PSInput {
    float4 Position : SV_Position;
    float2 TexCoord : TEXCOORD0;
};

// Fullscreen quad vertex shader
VSOutput VSMain(VSInput input) {
    VSOutput output;
    output.Position = float4(input.Position, 0.0, 1.0);
    output.TexCoord = input.TexCoord;
    return output;
}

// Simple copy pixel shader
float4 PSMain(PSInput input) : SV_Target {
    return InputTex.Sample(LinearClamp, input.TexCoord);
}
