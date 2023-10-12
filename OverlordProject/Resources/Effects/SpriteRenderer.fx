float4x4 gTransform : WorldViewProjection;
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
    uint TextureId : TEXCOORD0;
    float4 TransformData : POSITION; //PosX, PosY, Depth (PosZ), Rotation
    float4 TransformData2 : POSITION1; //PivotX, PivotY, ScaleX, ScaleY
    float4 Color : COLOR;
};

struct GS_DATA
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
    float2 TexCoord : TEXCOORD0;
};

//VERTEX SHADER
//*************
VS_DATA MainVS(VS_DATA input)
{
    return input;
}

//GEOMETRY SHADER
//***************
void CreateVertex(inout TriangleStream<GS_DATA> triStream, float3 pos, float4 col, float2 texCoord, float rotation, float2 rotCosSin, float2 offset, float2 pivotOffset)
{
    if (rotation != 0)
    {
        float2 rotatedPoint = float2(0,0);

        // Translate to correct position
        rotatedPoint += offset - pivotOffset;

		//Rotate and translate back
        pos.x += (rotatedPoint.x * rotCosSin.x) - (rotatedPoint.y * rotCosSin.y);
        pos.y += (rotatedPoint.x * rotCosSin.y) + (rotatedPoint.y * rotCosSin.x);
    }
    else
    {
        pos.x += offset.x - pivotOffset.x;
        pos.y += offset.y - pivotOffset.y;
    }

	//Geometry Vertex Output
    GS_DATA geomData = (GS_DATA) 0;
    geomData.Position = mul(float4(pos, 1.0f), gTransform);
    geomData.Color = col;
    geomData.TexCoord = texCoord;
    triStream.Append(geomData);
}

[maxvertexcount(4)]
void MainGS(point VS_DATA vertex[1], inout TriangleStream<GS_DATA> triStream)
{
	//Given Data (Vertex Data)
    float3 position = vertex[0].TransformData.xyz;  
    float rotation = vertex[0].TransformData.w;
    float2 pivot = vertex[0].TransformData2.xy;
    float2 scale = vertex[0].TransformData2.zw;

    float width = gTextureSize.x * scale.x;
    float height = gTextureSize.y * scale.y;
    pivot.x *= width;
    pivot.y *= height;

    float2 rotCosSin = float2(0, 0);
    if (rotation != 0) {
        rotCosSin = float2 (cos(rotation), sin(rotation));
    }

	//VERTEX 1 [LT]
    CreateVertex(triStream, position, vertex[0].Color, float2(0, 0), rotation, rotCosSin, float2(0,0), pivot);

	//VERTEX 2 [RT]
    CreateVertex(triStream, position, vertex[0].Color, float2(1, 0), rotation, rotCosSin, float2(width, 0), pivot);

	//VERTEX 3 [LB]
    CreateVertex(triStream, position, vertex[0].Color, float2(0, 1), rotation, rotCosSin, float2(0, height), pivot);

	//VERTEX 4 [RB]
    CreateVertex(triStream, position, vertex[0].Color, float2(1, 1), rotation, rotCosSin, float2(width, height), pivot);
}

//PIXEL SHADER
//************
float4 MainPS(GS_DATA input) : SV_TARGET
{

    float4 color = gSpriteTexture.Sample(samPoint, input.TexCoord) * input.Color;
    return color;
}

// Default Technique
technique11 Default
{
    pass p0
    {
        SetRasterizerState(BackCulling);
        SetBlendState(EnableBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetDepthStencilState(NoDepth,0);

        SetVertexShader(CompileShader(vs_4_0, MainVS()));
        SetGeometryShader(CompileShader(gs_4_0, MainGS()));
        SetPixelShader(CompileShader(ps_4_0, MainPS()));
    }
}
