#pragma once

#include <cstdint>

typedef unsigned int Package;

enum class PackageType : uint16_t
{
	RESERVED = 0,
	PLAYER_READY,
	ADD_OBJECT,
	REMOVE_OBJECT,
	UPDATE_OBJECTS,
	GAME_RESULT,
};

struct AddObjectData
{
	float m_Position[3];
};

enum class Result
{
	SUCCESS,
	FAILURE,
};
typedef void (*actionDoneCallback)(Result, void* p_UserData);
