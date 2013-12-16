#pragma once
#include <DirectXMath.h>

struct Vector2
{
	float x;
	float y;

	Vector2() : x(0.0f), y(0.0f)
	{}

	Vector2(float _x, float _y) : x(_x), y(_y)
	{}
};

struct Vector3 : Vector2
{
	float z;

	Vector3() : Vector2(0.0f, 0.0f), z(0.0f)
	{}

	Vector3(float _x, float _y, float _z) : Vector2(_x, _y), z(_z)
	{}
};

struct Vector4 : Vector3
{
	float w;

	Vector4() : Vector3(0.0f, 0.0f, 0.0f), w(0.0f)
	{}

	Vector4(float _x, float _y, float _z, float _w) : Vector3(_x, _y, _z), w(_w)
	{}

	inline Vector4 operator-(Vector4 p_vec)
	{
		this->x -= p_vec.x;
		this->y -= p_vec.y;
		this->z -= p_vec.z;
		this->w -= p_vec.w;

		return *this;
	}
};

inline Vector2 XMFLOAT2ToVector2(const DirectX::XMFLOAT2 *p_vector)
{
	return Vector2(p_vector->x, p_vector->y);
};

inline Vector3 XMFLOAT3ToVector3(const DirectX::XMFLOAT3 *p_vector)
{
	return Vector3(p_vector->x, p_vector->y, p_vector->z);
};

inline Vector4 XMFLOAT4ToVector4(const DirectX::XMFLOAT4 *p_vector)
{
	return Vector4(p_vector->x, p_vector->y, p_vector->z, p_vector->w);
};

inline Vector4 XMVECTORToVector4(const DirectX::XMVECTOR *p_vector)
{
	return Vector4(p_vector->m128_f32[0], p_vector->m128_f32[1], p_vector->m128_f32[2], p_vector->m128_f32[3]);
};

inline DirectX::XMFLOAT2 Vector2ToXMFLOAT2(const Vector2 *_p)
{
	return DirectX::XMFLOAT2(_p->x, _p->y);
}

inline DirectX::XMFLOAT3 Vector3ToXMFLOAT3(const Vector3 *_p)
{
	return DirectX::XMFLOAT3(_p->x, _p->y, _p->z);
}

inline DirectX::XMFLOAT4 Vector3ToXMFLOAT4(const Vector3 *_p, float _w)
{
	return DirectX::XMFLOAT4(_p->x, _p->y, _p->z, _w);
}

inline DirectX::XMFLOAT4 Vector4ToXMFLOAT4(const Vector4 *_p)
{
	return DirectX::XMFLOAT4(_p->x, _p->y, _p->z, _p->w);
}

inline DirectX::XMVECTOR Vector4ToXMVECTOR(const Vector4 *p_vector)
{
	return DirectX::XMVectorSet(p_vector->x, p_vector->y, p_vector->z, p_vector->w);
};