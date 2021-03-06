#include <boost\test\unit_test.hpp>
#include <ResourceManager.h>
#include "..\..\Client\Source\ClientExceptions.h"

BOOST_AUTO_TEST_SUITE(ResourceManagerTest)

	class TestResource
	{
	public:
		bool create(const char *p_ResourceType, const char *p_ResourceName)
		{
			return true;
		}
		bool release(const char *p_Resource)
		{
			return true;
		}
	};


	BOOST_AUTO_TEST_CASE(LoadResource)
	{
		ResourceManager rm;
		TestResource tr;
		rm.loadDataFromFile("..\\Source\\Common\\Resources.xml");
		using namespace std::placeholders;
		rm.registerFunction("model", std::bind(&TestResource::create, tr, _1, _2), std::bind(&TestResource::release, tr, _1));

		int id;
		BOOST_CHECK_NO_THROW(id = rm.loadResource("model", "Dzala"));
		BOOST_CHECK_GE(id, 0);
		rm.releaseResource(id);
	}
	BOOST_AUTO_TEST_CASE(LoadAndReleaseResource)
	{
		ResourceManager rm;
		TestResource tr;
		rm.loadDataFromFile("..\\Source\\Common\\Resources.xml");
		using namespace std::placeholders;
		rm.registerFunction("model", std::bind(&TestResource::create, tr, _1, _2), std::bind(&TestResource::release, tr, _1));

		int id;
		BOOST_CHECK_NO_THROW(id = rm.loadResource("model", "Dzala"));
		BOOST_CHECK_GE(id, 0);
		BOOST_CHECK_EQUAL(rm.releaseResource(0), true);
	}
	BOOST_AUTO_TEST_CASE(LoadAndReleaseResources)
	{
		ResourceManager rm;
		TestResource tr;
		rm.loadDataFromFile("..\\Source\\Common\\Resources.xml");
		using namespace std::placeholders;
		rm.registerFunction("model", std::bind(&TestResource::create, tr, _1, _2), std::bind(&TestResource::release, tr, _1));
		rm.setReleaseImmediately(true);

		int id1, id2, id3;
		BOOST_CHECK_NO_THROW(id1 = rm.loadResource("model", "Dzala"));
		BOOST_CHECK_GE(id1, 0);
		BOOST_CHECK_NO_THROW(id2 = rm.loadResource("model", "House1"));
		BOOST_CHECK_GE(id2, 0);
		BOOST_CHECK_NO_THROW(id3 = rm.loadResource("model", "Dzala"));
		BOOST_CHECK_GE(id3, 0);

		BOOST_CHECK_EQUAL(rm.releaseResource(id2), true);
		BOOST_CHECK_EQUAL(rm.releaseResource(id3), true);
		BOOST_CHECK_EQUAL(rm.releaseResource(id1), true);
		rm.unregisterResourceType("model");

		rm.registerFunction("texture", std::bind(&TestResource::create, tr, _1, _2), std::bind(&TestResource::release, tr, _1));

		BOOST_CHECK_NO_THROW(id1 = rm.loadModelTextureImpl("MANA_BAR", "assets/textures/manabar_changebar.png"));
		BOOST_CHECK_GE(id1, 0);
		BOOST_CHECK_NO_THROW(id2 = rm.loadModelTextureImpl("MANA_BARCHANGE", "assets/textures/manabar.png"));
		BOOST_CHECK_GE(id2, 0);
		BOOST_CHECK_NO_THROW(id3 = rm.loadModelTextureImpl("MANA_BAR", "assets/textures/manabar_changebar.png"));
		BOOST_CHECK_GE(id3, 0);

		rm.releaseModelTextureImpl("MANA_BAR");
		rm.releaseModelTextureImpl("MANA_BARCHANGE");
		rm.releaseModelTextureImpl("MANA_BAR");

		unsigned int zero = 0;

		BOOST_CHECK_GE(rm.getResourceList().size(), zero);

		rm.unregisterResourceType("texture");
	}

	BOOST_AUTO_TEST_CASE(ReleaseNonExistentResources)
	{
		ResourceManager rm;
		TestResource tr;
		rm.loadDataFromFile("..\\Source\\Common\\Resources.xml");
		using namespace std::placeholders;
		rm.registerFunction("model", std::bind(&TestResource::create, tr, _1, _2), std::bind(&TestResource::release, tr, _1));
#ifdef DEBUG
		BOOST_CHECK_THROW(rm.releaseResource(0), ResourceManagerException);
#else
		BOOST_CHECK_EQUAL(rm.releaseResource(0), false);
#endif
	}
	BOOST_AUTO_TEST_CASE(LoadResourceDataFromFile)
	{
		ResourceManager rm;
		BOOST_CHECK_THROW(rm.loadDataFromFile(""), ResourceManagerException);
	}

BOOST_AUTO_TEST_SUITE_END()