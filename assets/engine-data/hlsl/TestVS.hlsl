cbuffer Trans
{
    matrix mvp;
};


struct IN
{
    float3 pos : Position;
    float3 normal : Normal;
    float3 tangent : Tangent;
    float3 bitangent : Bitangent;
    float2 textCoord0 : UVf;
    float2 textCoord1 : UVs;
};

struct OUT
{
    float3 color : Color;
    float4 pos : SV_Position;
};

OUT main(IN i)
{
    OUT o;
    
    o.color = i.normal;
    o.pos = mul(float4(i.pos, 1.f), mvp);
    
    return o;
}


