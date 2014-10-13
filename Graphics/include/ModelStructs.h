#pragma once

struct CMaterial
{
	const char* m_DiffuseMap;
	const char* m_NormalMap;
	const char* m_SpecularMap;
};

struct CMaterialBuffer
{
	int start;
	int length;
};
