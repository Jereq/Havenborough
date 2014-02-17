#include "ResourceManager.h"
#include "CommonExceptions.h"
#include "Logger.h"

using std::string;
using std::vector;

void ResourceType::setType(string p_Type)
{
	m_Type = p_Type;
}
string ResourceType::getType()
{
	return m_Type;
}


ResourceManager::ResourceManager()
{
	m_ProjectDirectory = boost::filesystem::current_path();
	
	m_NextID = 0;
}

ResourceManager::ResourceManager(const boost::filesystem::path& p_RootPath)
	:	m_ProjectDirectory(p_RootPath),
		m_NextID(0)
{
}

ResourceManager::~ResourceManager()
{
	std::string unreleasedResources;

	for (auto& type : m_ResourceList)
	{
		for (auto& res : type.m_LoadedResources)
		{
			type.m_Release(res.m_Name.c_str());
			unreleasedResources += res.m_Name + ", ";
		}
	}

	if (!unreleasedResources.empty())
	{
		Logger::log(Logger::Level::WARNING, "Resource not released before shutdown: " + unreleasedResources);
	}
}

bool ResourceManager::registerFunction(string p_Type, std::function<bool(const char*, const char*)> p_CreateFunc,
	std::function<bool(const char*)> p_ReleaseFunc)
{
	for(auto &rl : m_ResourceList)
	{
		if(rl.getType() == p_Type)
		{
			return false;
		}
	}

	ResourceType temp;
	temp.setType(p_Type);
	temp.m_Create = p_CreateFunc;
	temp.m_Release = p_ReleaseFunc;
	m_ResourceList.push_back(temp);
	return true;
}


int ResourceManager::loadResource(string p_ResourceType, string p_ResourceName)
{
	boost::filesystem::path filePath(m_ProjectDirectory / m_ResourceTranslator.translate(p_ResourceType, p_ResourceName));

	for(auto &rl : m_ResourceList)
	{
		if(rl.getType() == p_ResourceType)
		{

			for(auto &r : rl.m_LoadedResources)
			{
				if(filePath.string() == r.m_Path)
				{
					r.m_Count++;

					return r.m_ID;
				}
			}

			if( rl.m_Create(p_ResourceName.c_str(), filePath.string().c_str()) )
			{
				ResourceType::Resource newRes;
				newRes.m_Name = p_ResourceName;
				newRes.m_ID = m_NextID++;
				newRes.m_Count = 1;
				newRes.m_Path = filePath.string().c_str();
				
				rl.m_LoadedResources.push_back(newRes);
				return newRes.m_ID;
			}
			else
			{
				throw ResourceManagerException("Error when loading resource: '" + p_ResourceType + ":" + p_ResourceName + "' (" + filePath.string() + ")", __LINE__, __FILE__);
			}
		}
	}
#ifdef DEBUG
	throw ResourceManagerException(std::string("Error when loading resource! create function for ") + p_ResourceType + "s not registered!", __LINE__, __FILE__);
#endif

	return -1;
}

void  ResourceManager::loadModelTexture(const char *p_ResourceName, const char *p_FilePath, void* p_Userdata)
{
	((ResourceManager*)p_Userdata)->loadModelTextureImpl(p_ResourceName, p_FilePath);
}

int ResourceManager::loadModelTextureImpl(const char *p_ResourceName, const char *p_FilePath)
{
	for(auto &rl : m_ResourceList)
	{
		if(rl.getType() == "texture")
		{

			for(auto &r : rl.m_LoadedResources)
			{
				if(p_FilePath == r.m_Path)
				{
					r.m_Count++;

					return r.m_ID;
				}
			}

			if( rl.m_Create(p_ResourceName, p_FilePath) )
			{
				ResourceType::Resource newRes;
				newRes.m_Name = p_ResourceName;
				newRes.m_ID = m_NextID++;
				newRes.m_Count = 1;
				newRes.m_Path = p_FilePath;
				
				rl.m_LoadedResources.push_back(newRes);
				return newRes.m_ID;
			}
			else
			{
				throw ResourceManagerException(std::string("Error when loading model texture resource: ") + p_FilePath + " (" + p_ResourceName + ")", __LINE__, __FILE__);
			}
		}
	}
#ifdef DEBUG
	throw ResourceManagerException(std::string("Error when loading model texture ") + p_FilePath + " (" + p_ResourceName + "). create function for textures not registered!", __LINE__, __FILE__);
#endif
	return -1;
}

bool ResourceManager::releaseResource(int p_ID)
{
	for(auto &rl : m_ResourceList)
	{
		for(auto it = rl.m_LoadedResources.begin(); it != rl.m_LoadedResources.end(); ++it)
		{
			auto& r = *it;

			if(r.m_ID == p_ID)
			{
				r.m_Count--;

				if (r.m_Count <= 0)
				{
					rl.m_Release(r.m_Name.c_str());
					rl.m_LoadedResources.erase(it);
				}

				return true;
			}
		}
	}

#ifdef DEBUG
	throw ResourceManagerException("Releasing a resource that does not exist?", __LINE__, __FILE__);
#endif
	return false;
}

void ResourceManager::releaseModelTexture(const char *p_ResourceName, void *p_Userdata)
{
	((ResourceManager*)p_Userdata)->releaseModelTextureImpl(p_ResourceName);
}

void ResourceManager::releaseModelTextureImpl(const char *p_ResourceName)
{
	for(auto &rl : m_ResourceList)
	{
		for(auto it = rl.m_LoadedResources.begin(); it != rl.m_LoadedResources.end(); ++it)
		{
			auto& r = *it;

			if(strcmp(r.m_Name.c_str(), p_ResourceName) == 0)
			{
				r.m_Count--;

				if (r.m_Count <= 0)
				{
					rl.m_Release(r.m_Name.c_str());
					rl.m_LoadedResources.erase(it);
				}

				break;
			}
		}
	}
}

