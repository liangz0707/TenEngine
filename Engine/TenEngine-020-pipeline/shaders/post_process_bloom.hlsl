/**
 * @file post_process_bloom.hlsl
 * @brief Bloom effect using separable Gaussian blur.
 *
 * Implements bloom through:
 * 1. Extract bright pixels
 * 2. Downsample
 * 3. Blur (horizontal then vertical pass)
 * 4. Upsample and composite
 */

Texture2D InputTex : register(t0);
SamplerState LinearClamp : register(s0);

cbuffer BloomParams : register(b0) {
    float Threshold : packoffset(c0.x);      // Brightness threshold
    float Intensity : packoffset(c0.y);      // Bloom intensity
    float2 TexelSize : packoffset(c0.z);     // 1.0 / resolution
};

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

// Brightness extraction pass
float4 PSMain_ExtractBright(PSInput input) : SV_Target {
    float3 color = InputTex.Sample(LinearClamp, input.TexCoord).rgb;
    float brightness = dot(color, float3(0.2126, 0.7152, 0.0722));
    
    // Extract pixels above threshold
    float3 contribution = brightness > Threshold ? color : float3(0, 0, 0);
    
    // Soft knee for smoother transition
    float softThreshold = Threshold * 0.5;
    float softness = saturate((brightness - softThreshold) / (Threshold - softThreshold));
    contribution = color * softness;
    
    return float4(contribution, 1.0);
}

// Horizontal blur pass (9-tap Gaussian)
float4 PSMain_BlurH(PSInput input) : SV_Target {
    float weights[5] = { 0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216 };
    
    float3 result = InputTex.Sample(LinearClamp, input.TexCoord).rgb * weights[0];
    
    for (int i = 1; i < 5; ++i) {
        float2 offset = float2(TexelSize.x * float(i), 0.0);
        result += InputTex.Sample(LinearClamp, input.TexCoord + offset).rgb * weights[i];
        result += InputTex.Sample(LinearClamp, input.TexCoord - offset).rgb * weights[i];
    }
    
    return float4(result, 1.0);
}

// Vertical blur pass (9-tap Gaussian)
float4 PSMain_BlurV(PSInput input) : SV_Target {
    float weights[5] = { 0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216 };
    
    float3 result = InputTex.Sample(LinearClamp, input.TexCoord).rgb * weights[0];
    
    for (int i = 1; i < 5; ++i) {
        float2 offset = float2(0.0, TexelSize.y * float(i));
        result += InputTex.Sample(LinearClamp, input.TexCoord + offset).rgb * weights[i];
        result += InputTex.Sample(LinearClamp, input.TexCoord - offset).rgb * weights[i];
    }
    
    return float4(result, 1.0);
}

// Composite pass - combine bloom with original
float4 PSMain_Composite(PSInput input) : SV_Target {
    float3 original = InputTex.Sample(LinearClamp, input.TexCoord).rgb;
    
    // Bloom would be sampled from pre-blurred texture
    // For this single-file version, we use the input directly
    float3 bloom = original * Intensity;
    
    // Additive blending
    float3 result = original + bloom;
    
    return float4(result, 1.0);
}
