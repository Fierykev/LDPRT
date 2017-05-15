#ifndef _HLSL_CPU_
#define _HLSL_CPU_

#include <limits>
#include "vec.h"

#define MAX_UTYPE SIZE_MAX

typedef size_t uint;

struct int2
{
	int x, y;

	int2(int px, int py)
	{
		x = px;
		y = py;
	}

	int2()
	{

	}

	int2(const int2 &data)
	{
		x = data.x;
		y = data.y;
	}
};

struct uint2
{
	uint x, y;

	uint2(uint px, uint py)
	{
		x = px;
		y = py;
	}

	uint2()
	{

	}

	uint2(const uint2 &data)
	{
		x = data.x;
		y = data.y;
	}
};

struct uint3
{
	uint x, y, z;

	uint3(uint px, uint py, uint pz)
	{
		x = px;
		y = py;
		z = pz;
	}

	uint3()
	{

	}

	uint3(const uint3 &data)
	{
		x = data.x;
		y = data.y;
		z = data.z;
	}

	uint& operator[](size_t index)
	{
		switch (index)
		{
		case 0:
			return x;
		case 1:
			return y;
		case 2:
			return z;
		}

		return x;
	}
};

struct float2
{
	float x, y;
	// TODO: CHECK ORDER
	float2(float px, float py)
	{
		x = px;
		y = py;
	}

	float2()
	{

	}

	float2(const float2 &data)
	{
		x = data.x;
		y = data.y;
	}
};

struct float3
{
	float x, y, z;

	float3(float px, float py, float pz)
	{
		x = px;
		y = py;
		z = pz;
	}

	float3()
	{

	}

	float3(const float3 &data)
	{
		x = data.x;
		y = data.y;
		z = data.z;
	}

	float& operator[](size_t index)
	{
		switch (index)
		{
		case 0:
			return x;
		case 1:
			return y;
		case 2:
			return z;
		}

		return x;
	}

	operator Vec3f() { return Vec3f(x, y, z); }
};

inline float3 vecToFloat(Vec3f in)
{
	return float3(in[0], in[1], in[2]);
}

struct float4
{
	float x, y, z, w;
	// TODO: CHECK ORDER
	float4(float px, float py, float pz, float pw)
	{
		x = px;
		y = py;
		z = pz;
		w = pw;
	}

	float4(float3 p, float pw)
	{
		x = p.x;
		y = p.y;
		z = p.z;
		w = pw;
	}

	float4()
	{

	}

	float4(const float4 &data)
	{
		x = data.x;
		y = data.y;
		z = data.z;
		w = data.w;
	}

	float& operator[](size_t index)
	{
		switch (index)
		{
		case 0:
			return x;
		case 1:
			return y;
		case 2:
			return z;
		case 3:
			return w;
		}

		return x;
	}
};

inline float2 floor(float2 n)
{
	return float2(floor(n.x), floor(n.y));
}

inline float2 operator-(const float2& a, const float2& b) {
	return float2(a.x - b.x, a.y - b.y);
}

inline float2 operator*(const float2& a, const float2& b) {
	return float2(a.x * b.x, a.y * b.y);
}

inline float2 operator*(const float2& a, const float& b) {
	return float2(a.x * b, a.y * b);
}

inline float3 floor(float3 n)
{
	return float3(floor(n.x), floor(n.y), floor(n.z));
}

inline float3 operator-(const float3& a, const float3& b) {
	return float3(a.x - b.x, a.y - b.y, a.z - b.z);
}

inline float3 operator*(const float3& a, const float3& b) {
	return float3(a.x * b.x, a.y * b.y, a.z * b.z);
}

inline float3 operator*(const float3& a, const float& b) {
	return float3(a.x * b, a.y * b, a.z * b);
}

inline float3 operator/(const float& a, const float3& b) {
	return float3(a / b.x, a / b.y, a / b.z);
}

inline float3 operator/(const float3& a, const float3& b) {
	return float3(a.x / b.x, a.y / b.y, a.z / b.z);
}

inline float3 operator/=(const float3& b, const float& a) {
	return float3(b.x / a, b.y / a, b.z / a);
}

inline float3 operator+=(const float3& a, const float3& b) {
	return float3(a.x + b.x, a.y + b.y, a.z + b.z);
}

inline uint3 operator+=(const uint3& a, const uint3& b) {
	return uint3(a.x + b.x, a.y + b.y, a.z + b.z);
}

inline float3 cross(const float3& a, const float3& b)
{
	return float3(a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}

inline float dot(const float3& a, const float3& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline float4 floor(float4 n)
{
	return float4(floor(n.x), floor(n.y), floor(n.z), floor(n.w));
}

inline float4 operator*(const float4& a, const float4& b) {
	return float4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
}

inline float4 operator*(const float4& a, const float& b) {
	return float4(a.x * b, a.y * b, a.z * b, a.w * b);
}

inline float clamp(const float& x, const uint& minVal, const uint& maxVal) {
	if (x < minVal)
		return minVal;
	else if (maxVal < x)
		return maxVal;

	return x;
}

#endif