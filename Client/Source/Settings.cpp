#include "Settings.h"
#include "ClientExceptions.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

Settings::Settings(void)
{
	m_Resolution = Vector2(1080,720);
}


Settings::~Settings(void)
{
}

void Settings::initialize(std::string p_FilePath)
{
	tinyxml2::XMLDocument doc;
	tinyxml2::XMLError res = doc.LoadFile(p_FilePath.c_str());
	if(res != tinyxml2::XML_NO_ERROR)
		throw ClientException("Settings file was not found.", __LINE__, __FILE__);

	tinyxml2::XMLElement *root = doc.FirstChildElement();
	if(root == nullptr)
		throw ClientException("Failed to load settings file. Root element not found.", __LINE__, __FILE__);

	tinyxml2::XMLElement *element = nullptr;
	for(element = root->FirstChildElement(); element != nullptr; element = element->NextSiblingElement())
	{
		std::string elementName = element->Value();
		if(elementName == "Settings")
		{
			loadSettings(element);
		}
		else if(elementName == "Controls")
		{
			loadControls(element);
		}
	}
}

void Settings::loadControls(tinyxml2::XMLElement *p_Element)
{
	tinyxml2::XMLElement *element = nullptr;
	for(element = p_Element->FirstChildElement(); element != nullptr; element = element->NextSiblingElement())
	{
		std::string elementName = element->Value();
		const char* commandValue = element->Attribute("command");

		if(elementName == "KeyMap")
		{
			const char* value = element->Attribute("key");
			std::string sValue(value);
			unsigned short keyValue;
			if(sValue == "Escape")
				keyValue = VK_ESCAPE;
			else if(sValue == "Space")
				keyValue = VK_SPACE;
			else if(sValue == "Return")
				keyValue = VK_RETURN;
			else			
				keyValue = (unsigned short)*value;

			m_KeyMap.insert(make_pair(std::string(commandValue), keyValue));
		}
		else if(elementName == "MouseMap")
		{
			MouseStruct m;
			m.movement = std::string(commandValue);
			m.position = (element->Attribute("position"));
			std::string keyValue(element->Attribute("movement"));
			Axis value;
			if(keyValue == "Vertical")
				value = Axis::VERTICAL;
			else if(keyValue == "Horizontal")
				value = Axis::HORIZONTAL;

			m.axis = value;
			m_MouseMap.push_back(m);
		}
		else if(elementName == "MouseButtonMap")
		{
			std::string keyValue(element->Attribute("button"));
			MouseButton value;
			if(keyValue == "Left")
				value = MouseButton::LEFT;
			else if(keyValue == "Right")
				value = MouseButton::RIGHT;
			else if(keyValue == "Middle")
				value = MouseButton::MIDDLE;
			else if(keyValue == "Extra_1")
				value = MouseButton::EXTRA_1;
			else if(keyValue == "Extra_2")
				value = MouseButton::EXTRA_2;

			m_MouseButtonMap.insert(make_pair(std::string(commandValue), value));
		}
	}
}

void Settings::loadSettings(tinyxml2::XMLElement *p_Element)
{
	tinyxml2::XMLElement *element = nullptr;
	for(element = p_Element->FirstChildElement(); element != nullptr; element = element->NextSiblingElement())
	{
		std::string elementName = element->Value();

		if(elementName == "Resolution")
		{
			m_Resolution.x = element->FloatAttribute("width");
			m_Resolution.y = element->FloatAttribute("height");
		}
		else
		{
			m_SettingsEnabled[elementName] = element->BoolAttribute("enabled");
		}
	}
}

const std::map<std::string, unsigned short> &Settings::getKeyMap() const
{
	return m_KeyMap;
}

const std::vector<Settings::MouseStruct> &Settings::getMouseMap() const
{
	return m_MouseMap;
}

const std::map<std::string, MouseButton> &Settings::getMouseButtonMap() const
{
	return m_MouseButtonMap;
}

const bool Settings::getIsSettingEnabled(std::string p_SettingName) const
{
	if (m_SettingsEnabled.count(p_SettingName) > 0)
	{
		return m_SettingsEnabled.at(p_SettingName);
	} 
	else
	{
		throw ClientException("The setting with name: " + p_SettingName + ", was not found.", __LINE__, __FILE__);
	}
}

const Vector2 Settings::getResolution() const
{
	return m_Resolution;
}