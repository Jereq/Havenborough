#pragma once

#include <cstdint>

class ResourceProxy
{
public:
	struct Buff
	{
		const char* data;
		size_t size;
	};

	typedef uint32_t ResId;

	typedef Buff (*ResourceDataCallback)(ResId p_Res, void* p_UserData);
	typedef ResId (*FindResourceIdCallback)(const char* p_ResourceName, void* p_UserData);

private:
	ResourceDataCallback m_DataCallback;
	void* m_DataUserData;
	FindResourceIdCallback m_FindCallback;
	void* m_FindUserData;

public:
	ResourceProxy()
	{
	}

	ResourceProxy(ResourceDataCallback p_DataCallback, void* p_DataUserData,
		FindResourceIdCallback p_FindCallback, void* p_FindUserData)
		: m_DataCallback(p_DataCallback),
		m_DataUserData(p_DataUserData),
		m_FindCallback(p_FindCallback),
		m_FindUserData(p_FindUserData)
	{
	}

	Buff getData(ResId p_Res)
	{
		return m_DataCallback(p_Res, m_DataUserData);
	}

	Buff getData(const char* p_ResourceName)
	{
		ResId id = findResourceId(p_ResourceName);
		return getData(id);
	}

	ResId findResourceId(const char* p_ResourceName)
	{
		return m_FindCallback(p_ResourceName, m_FindUserData);
	}
};
