#include "stdafx.h"
#include "SpriteFontLoader.h"

SpriteFont* SpriteFontLoader::LoadContent(const ContentLoadInfo& loadInfo)
{
	const auto pReader = new BinaryReader();
	pReader->Open(loadInfo.assetFullPath);

	if (!pReader->Exists())
	{
		Logger::LogError(L"Failed to read the assetFile!\nPath: \'{}\'", loadInfo.assetSubPath);
		return nullptr;
	}

	// BMF font identification
	char id1 = pReader->Read<char>();
	char id2 = pReader->Read<char>();
	char id3 = pReader->Read<char>();

	if (id1 != 66 || id2 != 77 || id3 != 70) {
		Logger::LogError(L"SpriteFontLoader::LoadContent > Not a valid .fnt font");
		return nullptr;
	}

	// Version 3 check
	char version = pReader->Read<char>();
	if (version != 0x3) {
		Logger::LogError(L"SpriteFontLoader::LoadContent > Only .fnt version 3 is supported");
		return nullptr;
	}

	SpriteFontDesc fontDesc{};

	//**********
	// BLOCK 0 *
	//**********
	
	//Retrieve the blockId and blockSize
	char blockId = pReader->Read<char>();
	int blockSize = pReader->Read<int>();

	//Retrieve the FontSize [fontDesc.fontSize]
	fontDesc.fontSize = pReader->Read<short>();

	//Move the binreader to the start of the FontName [BinaryReader::MoveBufferPosition(...) or you can set its position using BinaryReader::SetBufferPosition(...))
	pReader->MoveBufferPosition(12);
	//Retrieve the FontName [fontDesc.fontName]
	fontDesc.fontName = pReader->ReadNullString();

	//**********
	// BLOCK 1 *
	//**********

	//Retrieve the blockId and blockSize
	blockId = pReader->Read<char>();
	blockSize = pReader->Read<int>();

	//Retrieve Texture Width & Height [fontDesc.textureWidth/textureHeight]
	pReader->MoveBufferPosition(4);
	fontDesc.textureWidth = pReader->Read<short>();
	fontDesc.textureHeight = pReader->Read<short>();

	//Retrieve PageCount
	short pages = pReader->Read<short>();
	if (pages > 1) {
		Logger::LogError(L"SpriteFontLoader::LoadContent > Only one texture per font is allowed!");
		return nullptr;
	}

	//Advance to Block2 (Move Reader)
	pReader->MoveBufferPosition(5);

	//**********
	// BLOCK 2 *
	//**********
	
	//Retrieve the blockId and blockSize
	blockId = pReader->Read<char>();
	blockSize = pReader->Read<int>();
	
	//Retrieve the PageName (BinaryReader::ReadNullString)
	std::wstring pageName = pReader->ReadNullString();

	//Construct the full path to the page texture file
	auto texturePath = loadInfo.assetFullPath.parent_path().append(pageName);
	fontDesc.pTexture = ContentManager::Load<TextureData>(texturePath);

	//**********
	// BLOCK 3 *
	//**********
	//Retrieve the blockId and blockSize
	blockId = pReader->Read<char>();
	blockSize = pReader->Read<int>();
	
	//Retrieve Character Count (see documentation)
	int charCount{ blockSize / 20 };
	
	// Retrieve each character
	for (int i{}; i < charCount; ++i) {
		FontMetric character{};
		character.character = char(pReader->Read<int>());
		short xPos = pReader->Read<short>();
		short yPos = pReader->Read<short>();
		character.width = pReader->Read<short>();
		character.height = pReader->Read<short>();
		character.offsetX = pReader->Read<short>();
		character.offsetY = pReader->Read<short>();
		character.advanceX = pReader->Read<short>();
		character.page = pReader->Read<char>();

		char channel = pReader->Read<char>();
		switch (channel) {
			case 0b1:
				character.channel = 2;
				break;
			case 0b10:
				character.channel = 1;
				break;
			case 0b100:
				character.channel = 0;
				break;
			case 0b1000:
				character.channel = 3;
				break;
		}

		character.texCoord = XMFLOAT2(float(xPos) / fontDesc.textureWidth, float(yPos) / fontDesc.textureHeight);

		fontDesc.metrics[character.character] = character;
	}

	delete pReader;
	return new SpriteFont(fontDesc);
}

void SpriteFontLoader::Destroy(SpriteFont* objToDestroy)
{
	SafeDelete(objToDestroy);
}
