//=============================================================================
//// Shader uses position and texture
//=============================================================================
SamplerState samPoint
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Mirror;
    AddressV = Mirror;
};

Texture2D gTexture;

DepthStencilState EnableDepth
{
	DepthEnable = TRUE;
	DepthWriteMask = ALL;
};

RasterizerState BackFaceCulling
{
	CullMode = BACK;
};


//IN/OUT STRUCTS
//--------------
struct VS_INPUT
{
    float3 Position : POSITION;
	float2 TexCoord : TEXCOORD0;

};

struct PS_INPUT
{
    float4 Position : SV_POSITION;
	float2 TexCoord : TEXCOORD1;
};


//VERTEX SHADER
//-------------
PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;
	output.Position = float4(input.Position,1);

	output.TexCoord = (float2)0;
	output.TexCoord.x = input.Position.x / 2 + 0.5;
	output.TexCoord.y = input.Position.y / -2 + 0.5;
	
	return output;
}


//PIXEL SHADER
//------------
float4 PS(PS_INPUT input) : SV_Target
{
	float2 center = float2(0.5f, 0.5f);
	float darkness = pow(1.0f - distance(input.TexCoord, center),1.0f);
	float4 color = gTexture.Sample(samPoint, input.TexCoord)*darkness;
	color.a = 1.0;

    return color;
}


//TECHNIQUE
//---------
technique11 Grayscale
{
    pass P0
    {          
		SetRasterizerState(BackFaceCulling);
		SetDepthStencilState(EnableDepth, 0);

		SetVertexShader( CompileShader( vs_4_0, VS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS() ) );
    }
}

