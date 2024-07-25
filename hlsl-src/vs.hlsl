struct VS_IN
{
    float3 pos : POSITION0;
    float4 color : COLOR0;
};

struct VS_OUT
{
    float4 pos : SV_Position;
    float4 color : COLOR0;
};

VS_OUT main(VS_IN IN)
{
    VS_OUT OUT;
    OUT.pos = float4(IN.pos, 1.0f);
    OUT.color = IN.color;
    return OUT;
}