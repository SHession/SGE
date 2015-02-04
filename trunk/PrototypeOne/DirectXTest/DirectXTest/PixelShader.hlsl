

Texture2D    tex : register( t0 );  //Texture to be drawn
SamplerState samLinear : register( s0 ); //Sample state of the texture

struct VS_OUTPUT //THe output of the vertex into the pixel shader
{
	float4 Pos : SV_POSITION;
    float4 Normal : NORMAL;
	float2 Tex : TEXCOORD0;
};


float4 PS( VS_OUTPUT input) : SV_TARGET
{

	return tex.Sample(samLinear, input.Tex) ; //sample the texture based on the uvs
}