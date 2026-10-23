sampler2D SrcSamp : register(s0);

// EffectParams (c0):
//   x = Desaturation (0.0 = full color, 1.0 = grayscale)
//   y = Flash / Contrast boost (0.0 = normal, 1.0 = max flash)
//   z = Vignette strength (0.0 = none, 1.0 = heavy edge darkening)
//   w = Fade progress (1.0 = just started, 0.0 = ended)
float4 EffectParams : register(c0);

float4 PS(float2 tex : TEXCOORD0) : COLOR0
{
    float4 color = tex2D(SrcSamp, tex);
    float gray = dot(color.rgb, float3(0.299, 0.587, 0.114));
    float3 boosted = (color.rgb - 0.5) * (1.0 + EffectParams.y * 0.4) + 0.5 + (EffectParams.y * 0.08);
    float3 desat = lerp(boosted, (float3)gray, EffectParams.x);
    float2 coord = (tex - 0.5) * 2.0;
    float dist = dot(coord, coord);
    float vignette = 1.0 - (dist * EffectParams.z * 0.35);
    desat *= saturate(vignette);
    
    return float4(saturate(desat), 1.0);
}
