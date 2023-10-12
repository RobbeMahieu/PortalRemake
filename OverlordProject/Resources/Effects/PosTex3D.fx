float4x4 gWorld : WORLD;
float4x4 gWorldViewProj : WORLDVIEWPROJECTION;
float3 gLightDirection = float3(-0.577f, -0.577f, 0.577f);
float4 gColor = float4(1.0f, 1.0f, 1.0f, 1.0f);

struct VS_INPUT {
	float3 pos : POSITION;
	float2 texCoord : TEXCOORD;
};
struct VS_OUTPUT {
	float4 pos : SV_POSITION;
	float2 texCoord : TEXCOORD;
};

Texture2D gDiffuseMap;
SamplerState samLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;// or Mirror or Clamp or Border
	AddressV = Wrap;// or Mirror or Clamp or Border
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
VS_OUTPUT VS(VS_INPUT input) {
	VS_OUTPUT output;

	output.pos = mul(float4(input.pos, 1.0f), gWorldViewProj);
	output.texCoord = input.texCoord;

	return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(VS_OUTPUT input) : SV_TARGET{


	return gDiffuseMap.Sample(samLinear,input.texCoord);
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

		SetVertexShader(CompileShader(vs_4_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, PS()));
	}
}

