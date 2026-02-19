/**
 * @file post_process_tonemap.hlsl
 * @brief ACES Filmic Tonemapping for HDR to LDR conversion.
 *
 * Implements the ACES (Academy Color Encoding System) filmic tonemap
 * for cinematic-looking HDR rendering.
 */

Texture2D InputTex : register(t0);
SamplerState LinearClamp : register(s0);

// Exposure adjustment (can be controlled via constant buffer)
cbuffer TonemapParams : register(b0) {
    float Exposure : packoffset(c0.x);
    float Contrast : packoffset(c0.y);
    float Saturation : packoffset(c0.z);
    float Padding : packoffset(c0.w);
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

// ACES Filmic Tonemap approximation
// Based on: https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
float3 ACESFilm(float3 x) {
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return saturate((x * (a * x + b)) / (x * (c * x + d) + e));
}

// Alternative: Reinhard tonemap (simpler, more neutral)
float3 Reinhard(float3 x) {
    return x / (1.0 + x);
}

// Alternative: Uncharted 2 tonemap
float3 Uncharted2ToneMap(float3 x) {
    float A = 0.15;
    float B = 0.50;
    float C = 0.10;
    float D = 0.20;
    float E = 0.02;
    float F = 0.30;
    return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

// Fullscreen quad vertex shader
VSOutput VSMain(VSInput input) {
    VSOutput output;
    output.Position = float4(input.Position, 0.0, 1.0);
    output.TexCoord = input.TexCoord;
    return output;
}

// Tonemap pixel shader using ACES
float4 PSMain(PSInput input) : SV_Target {
    // Sample HDR color
    float3 hdrColor = InputTex.Sample(LinearClamp, input.TexCoord).rgb;
    
    // Apply exposure
    hdrColor *= Exposure;
    
    // Apply ACES tonemap
    float3 ldrColor = ACESFilm(hdrColor);
    
    // Apply saturation adjustment
    float luminance = dot(ldrColor, float3(0.2126, 0.7152, 0.0722));
    ldrColor = lerp(float3(luminance, luminance, luminance), ldrColor, Saturation);
    
    return float4(ldrColor, 1.0);
}

// Alternative technique using Reinhard
float4 PSMain_Reinhard(PSInput input) : SV_Target {
    float3 hdrColor = InputTex.Sample(LinearClamp, input.TexCoord).rgb;
    hdrColor *= Exposure;
    float3 ldrColor = Reinhard(hdrColor);
    return float4(ldrColor, 1.0);
}
