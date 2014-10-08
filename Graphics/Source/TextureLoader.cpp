#include "TextureLoader.h"
#include "GraphicsExceptions.h"

TextureLoader::TextureLoader(ID3D11Device* p_Device, ID3D11DeviceContext* p_DeviceContext)
{
	m_Device = p_Device;
	m_DeviceContext = p_DeviceContext;
	m_CompabilityList.push_back("bmp");
	m_CompabilityList.push_back("gif");
	m_CompabilityList.push_back("ico");
	m_CompabilityList.push_back("jpeg");
	m_CompabilityList.push_back("jpe");
	m_CompabilityList.push_back("jpg");
	m_CompabilityList.push_back("png");
	m_CompabilityList.push_back("tiff");
	m_CompabilityList.push_back("tif");
	m_CompabilityList.push_back("phot");
	m_CompabilityList.push_back("dds");
	m_CompabilityList.shrink_to_fit();
}

TextureLoader::TextureLoader()
{
}

TextureLoader::~TextureLoader(){}

void TextureLoader::destroy()
{
	m_Device = nullptr;
	m_DeviceContext = nullptr;
	m_CompabilityList.clear();
}

ID3D11ShaderResourceView* TextureLoader::createTextureFromFile(const char* p_Filename)
{
	//Pick out file extension
	std::vector<char> buffer(strlen(p_Filename)+1);
	strcpy(buffer.data(), p_Filename);
	char *type = nullptr, *tmp;
	tmp = strtok(buffer.data(), ".");
	while(tmp != nullptr)
	{
		type = tmp;
		tmp = strtok(NULL,".");
	}
	char* result = checkCompability(type);

	//If file format is not supported or invalid filename return nullptr.
	if(strcmp(result, "err") == 0)
		return nullptr;

	ID3D11Resource*				textureResource = nullptr;
	ID3D11ShaderResourceView*	textureSRV = nullptr;

	//Convert filename from char pointer to wchar_t 
	//because the texture loader function wants the filename in wchar. 
	std::vector<wchar_t> filename(strlen(p_Filename)+1);
	mbstowcs(filename.data(), p_Filename, strlen(p_Filename)+1);
	filename.end() - 1;

	HRESULT hr = S_OK;
	//All supported file formats except dds uses the WIC TextureLoader.
	if(strcmp(result, "wic") == 0)
	{
		hr = CreateWICTextureFromFile(m_Device, m_DeviceContext, filename.data(), 
																		&textureResource, &textureSRV,0);
		if(FAILED(hr))
		{
			throw TextureLoaderException("WIC Texture load failed: " + std::string(p_Filename), __LINE__, __FILE__);
		}
	}
	else
	{
		//Temporary variable used to save a return value that is not used.
		DirectX::DDS_ALPHA_MODE mode;

		hr = CreateDDSTextureFromFile(m_Device, filename.data(), 
														&textureResource, &textureSRV, 0, &mode);

		if(FAILED(hr))
		{
			throw TextureLoaderException("DDS Texture load failed: " + std::string(p_Filename), __LINE__, __FILE__);
		}
	}
	//Texture resource is used to catch output from the texture creation function but the
	//data saved in it is not used, So we delete it after.
	if(textureResource != nullptr)
		textureResource->Release();
	filename.clear();

	return textureSRV;
}

ID3D11ShaderResourceView* TextureLoader::createTextureFromMemory(const char* p_Data, size_t p_DataLen, const char* p_Type)
{
	//Pick out file extension
	const char* result = checkCompability(p_Type);

	//If file format is not supported or invalid filename return nullptr.
	if(strcmp(result, "err") == 0)
		return nullptr;

	ID3D11Resource*				textureResource = nullptr;
	ID3D11ShaderResourceView*	textureSRV = nullptr;

	HRESULT hr = S_OK;
	//All supported file formats except dds uses the WIC TextureLoader.
	if(strcmp(result, "wic") == 0)
	{
		hr = CreateWICTextureFromMemory(m_Device, m_DeviceContext, (const uint8_t*)p_Data, p_DataLen, 
																		&textureResource, &textureSRV,0);
		if(FAILED(hr))
		{
			throw TextureLoaderException("WIC Texture load from memory failed", __LINE__, __FILE__);
		}
	}
	else
	{
		//Temporary variable used to save a return value that is not used.
		DirectX::DDS_ALPHA_MODE mode;

		hr = CreateDDSTextureFromMemory(m_Device, (const uint8_t*)p_Data, p_DataLen, 
														&textureResource, &textureSRV, 0, &mode);

		if(FAILED(hr))
		{
			throw TextureLoaderException("DDS Texture load from memory failed", __LINE__, __FILE__);
		}
	}
	//Texture resource is used to catch output from the texture creation function but the
	//data saved in it is not used, So we delete it after.
	if(textureResource != nullptr)
		textureResource->Release();

	return textureSRV;
}

char* TextureLoader::checkCompability(const char* p_FileType)
{
	unsigned int size = m_CompabilityList.size();

	if (size == 0)
		return "err";

	//Check if the file format we want to load is supported.
	for(unsigned int i = 0; i < size - 1; i++)
	{
		//Loop through the compability list except for the last item in the list which is .dds
		//because dds files is handled by another Loader function.
		if(strcmp(p_FileType, m_CompabilityList.at(i)) == 0)
		{
			return "wic";
		}
	}
	//Check last against the last element in the list which is dds
	if(strcmp(p_FileType, m_CompabilityList.at(size - 1)) == 0)
	{
		return "dds";
	}
	//if the file format is not supported return err.
	return "err";
}

HRESULT TextureLoader::CreateWICTextureFromFile(ID3D11Device* p_Device, ID3D11DeviceContext* p_Context,
	const wchar_t* p_FileName, ID3D11Resource** p_Texture, ID3D11ShaderResourceView** p_TextureView, size_t p_MaxSize)
{
	return DirectX::CreateWICTextureFromFile(p_Device, p_Context, p_FileName, p_Texture, p_TextureView, p_MaxSize);
}

HRESULT TextureLoader::CreateWICTextureFromMemory(ID3D11Device* p_Device, ID3D11DeviceContext* p_Context,
	const uint8_t* p_Data, size_t p_DataLen, ID3D11Resource** p_Texture, ID3D11ShaderResourceView** p_TextureView, size_t p_MaxSize)
{
	return DirectX::CreateWICTextureFromMemory(p_Device, p_Context, p_Data, p_DataLen, p_Texture, p_TextureView, p_MaxSize);
}

HRESULT TextureLoader::CreateDDSTextureFromFile(ID3D11Device* p_Device, const wchar_t* p_FileName,
	ID3D11Resource** p_Texture, ID3D11ShaderResourceView** p_TextureView, size_t p_MaxSize, DirectX::DDS_ALPHA_MODE* p_AlphaMode)
{
	return DirectX::CreateDDSTextureFromFile(p_Device, p_FileName, p_Texture, p_TextureView, p_MaxSize, p_AlphaMode);
}

HRESULT TextureLoader::CreateDDSTextureFromMemory(ID3D11Device* p_Device, const uint8_t* p_Data, size_t p_DataLen,
	ID3D11Resource** p_Texture, ID3D11ShaderResourceView** p_TextureView, size_t p_MaxSize, DirectX::DDS_ALPHA_MODE* p_AlphaMode)
{
	return DirectX::CreateDDSTextureFromMemory(p_Device, p_Data, p_DataLen, p_Texture, p_TextureView, p_MaxSize, p_AlphaMode);
}
