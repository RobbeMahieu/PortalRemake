float4x4 gTransform : WORLDVIEWPROJECTION;
Texture2D gSpriteTexture;
float2 gTextureSize;

SamplerState samPoint
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = WRAP;
	AddressV = WRAP;
};

BlendState EnableBlending
{
	BlendEnable[0] = TRUE;
	SrcBlend = SRC_ALPHA;
	DestBlend = INV_SRC_ALPHA;
};

DepthStencilState NoDepth
{
	DepthEnable = FALSE;
};

RasterizerState BackCulling
{
	CullMode = BACK;
};


//SHADER STRUCTS
//**************
struct VS_DATA
{
	int Channel : TEXCOORD2; //Texture Channel
	float3 Position : POSITION; //Left-Top Character Quad Starting Position
	float4 Color: COLOR; //Color of the vertex
	float2 TexCoord: TEXCOORD0; //Left-Top Character Texture Coordinate on Texture
	float2 CharSize: TEXCOORD1; //Size of the character (in screenspace)
};

struct GS_DATA
{
	float4 Position : SV_POSITION; //HOMOGENEOUS clipping space position
	float4 Color: COLOR; //Color of the vertex
	float2 TexCoord: TEXCOORD0; //Texcoord of the vertex
	int Channel : TEXCOORD1; //Channel of the vertex
};

//VERTEX SHADER
//*************
VS_DATA MainVS(VS_DATA input)
{
	return input;
}

//GEOMETRY SHADER
//***************
void CreateVertex(inout TriangleStream<GS_DATA> triStream, float3 pos, float4 col, float2 texCoord, int channel)
{
	//Geometry Vertex Output
	GS_DATA geomData = (GS_DATA)0;
	geomData.Position = mul(float4(pos, 1.0f), gTransform);
	geomData.Color = col;
	geomData.TexCoord = texCoord;
	geomData.Channel = channel;
	triStream.Append(geomData);
}

[maxvertexcount(4)]
void MainGS(point VS_DATA vertex[1], inout TriangleStream<GS_DATA> triStream)
{
	//VERTEX 1 [LT]
	float3 position = vertex[0].Position;
	float2 uvCoord = vertex[0].TexCoord;
	float2 uvOffset = float2(vertex[0].CharSize.x / gTextureSize.x, vertex[0].CharSize.y / gTextureSize.y);
	CreateVertex(triStream, position, vertex[0].Color, uvCoord, vertex[0].Channel);

	//VERTEX 2 [RT]
	position = float3(vertex[0].Position.x + vertex[0].CharSize.x, vertex[0].Position.y, vertex[0].Position.z);
	uvCoord = float2(vertex[0].TexCoord.x + uvOffset.x, vertex[0].TexCoord.y);
	CreateVertex(triStream, position, vertex[0].Color, uvCoord, vertex[0].Channel);

	//VERTEX 3 [LB]
	position = float3(vertex[0].Position.x, vertex[0].Position.y + vertex[0].CharSize.y, vertex[0].Position.z);
	uvCoord = float2(vertex[0].TexCoord.x, vertex[0].TexCoord.y + uvOffset.y);
	CreateVertex(triStream, position, vertex[0].Color, uvCoord, vertex[0].Channel);

	//VERTEX 4 [RB]
	position = vertex[0].Position + float3(vertex[0].CharSize, 0);
	uvCoord = vertex[0].TexCoord + uvOffset;
	CreateVertex(triStream, position, vertex[0].Color, uvCoord, vertex[0].Channel);
}

//PIXEL SHADER
//************
float4 MainPS(GS_DATA input) : SV_TARGET{
	return gSpriteTexture.Sample(samPoint, input.TexCoord)[input.Channel] * input.Color;
}

// Default Technique
technique10 Default {

	pass p0 {
		SetRasterizerState(BackCulling);
		SetBlendState(EnableBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetVertexShader(CompileShader(vs_4_0, MainVS()));
		SetGeometryShader(CompileShader(gs_4_0, MainGS()));
		SetPixelShader(CompileShader(ps_4_0, MainPS()));
	}
}
