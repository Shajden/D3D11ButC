struct VS_OUT
{
    float4 pos : SV_Position;
    float4 color : COLOR0;
};

float4 main(VS_OUT IN) : SV_Target0
{
    return IN.color;
}