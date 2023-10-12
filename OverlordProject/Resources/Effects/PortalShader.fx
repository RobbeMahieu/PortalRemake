float4x4 gWorldViewProj : WORLDVIEWPROJECTION; 
float4x4 gPlayerWorldViewProj;

static float PI = 3.1415926535f;

Texture2D gDiffuseMap;
SamplerState samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Clamp;// or Mirror or Clamp or Border
	AddressV = Clamp;// or Mirror or Clamp or Border
};

struct VS_INPUT{
	float3 pos : POSITION;
};
struct VS_OUTPUT{
	float4 pos : SV_POSITION;
	float4 texCoord : TEXCOORD0;
};

DepthStencilState EnableDepth
{
	DepthEnable = TRUE;
	DepthWriteMask = ALL;
};

RasterizerState FrontCulling
{
	CullMode = FRONT;
};

BlendState NoBlending
{
	BlendEnable[0] = FALSE;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VS(VS_INPUT input){

	VS_OUTPUT output;
	output.pos = mul( float4(input.pos,1.0f), gWorldViewProj);

	float4 ndcSpace = mul(float4(input.pos, 1.0f), gPlayerWorldViewProj);
	ndcSpace.x /= ndcSpace.w;
	ndcSpace.y /= ndcSpace.w;
	ndcSpace.z /= ndcSpace.w;
	output.texCoord = ndcSpace;

	return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(VS_OUTPUT input) : SV_TARGET{

	input.texCoord.x = input.texCoord.x / 2 + 0.5;
	input.texCoord.y = input.texCoord.y / -2 + 0.5;

	return gDiffuseMap.Sample(samLinear, input.texCoord);
	return gDiffuseMap.Sample(samLinear, float2(input.pos.x/1280, input.pos.y/720));
}

//--------------------------------------------------------------------------------------
// Technique
//--------------------------------------------------------------------------------------
technique11 Default
{
    pass P0
    {
		SetRasterizerState(FrontCulling);
		SetDepthStencilState(EnableDepth, 0);
		SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);

		SetVertexShader( CompileShader( vs_4_0, VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_4_0, PS() ) );
    }
}

