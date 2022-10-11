#pragma once
#include <cmath>
#include <math.h>

class Vector3 {
  public:
	float x, y, z;

	Vector3(float x, float y, float z) : x(x), y(y), z(z) { }
	Vector3(int x, int y, int z) : x((float) x), y((float) y), z((float) z) { }
	Vector3() : x(0), y(0), z(0) { }
	Vector3(float value) : x(value), y(value), z(value) { }

#pragma region Functions
	float lengthSquared() {
		return x * x + y * y + z * z;
	}

	float length() {
		return sqrtf(this->lengthSquared());
	}

    Vector3 clone() {
        return Vector3(x, y, z);
    }
#pragma endregion Functions

#pragma region Operator overloading
	Vector3 operator+(Vector3 b) {
		return Vector3(this->x + b.x, this->y + b.y, this->z + b.z);
	}

	Vector3 operator-(Vector3 b) {
		return Vector3(this->x - b.x, this->y - b.y, this->z - b.z);
	}

	Vector3 operator*(float scalar) {
		return Vector3(this->x * scalar, this->y * scalar, this->z * scalar);
	}

	Vector3 operator/(float divisor) {
		return Vector3(this->x / divisor, this->y / divisor, this->z / divisor);
	}

    Vector3 operator-() {
        return Vector3(-x, -y, -z);
    }

    bool operator==(Vector3 b) {
		return x == b.x && y == b.y && z == b.z;
	}
#pragma endregion Operator overloading

#pragma region Static operations
	static float dot(Vector3 a, Vector3 b) {
		return a.x * b.x + a.y * b.y + a.z * b.z;
	}

	static Vector3 normalize(Vector3 a) {
		const float length = a.length();
		return Vector3(a.x / length, a.y / length, a.z / length);
	}
#pragma endregion Static operations

#pragma region Static getters
	inline static Vector3 ZERO() {
		return Vector3(0, 0, 0);
	}

	inline static Vector3 ONE() {
		return Vector3(1, 1, 1);
	}
#pragma endregion Static getters
};

class Vector2 {
  public:
	float x, y;

	Vector2(float x, float y) : x(x), y(y) { }
	Vector2(int x, int y) : x((float) x), y((float) y) { }
	Vector2() : x(0), y(0) { }
	Vector2(float value) : x(value), y(value) { }

#pragma region Functions
	float lengthSquared() {
		return x * x + y * y;
	}

	float length() {
		return sqrtf(this->lengthSquared());
	}

    Vector2 clone() {
        return Vector2(x, y);
    }
#pragma endregion Functions

#pragma region Operator overloading
	Vector2 operator+(Vector2 b) {
		return Vector2(this->x + b.x, this->y + b.y);
	}

	Vector2 operator-(Vector2 b) {
		return Vector2(this->x - b.x, this->y - b.y);
	}

	Vector2 operator*(float scalar) {
		return Vector2(this->x * scalar, this->y * scalar);
	}

	Vector2 operator/(float divisor) {
		return Vector2(this->x / divisor, this->y / divisor);
	}

    bool operator==(Vector2 b) {
		return x == b.x && y == b.y;
	}
#pragma endregion Operator overloading

#pragma region Static operations
	static float dot(Vector2 a, Vector2 b) {
		return a.x * b.x + a.y * b.y;
	}

	static Vector2 normalize(Vector2 a) {
		const float length = a.length();
		return Vector2(a.x / length, a.y / length);
	}
#pragma endregion Static operations

#pragma region Static getters
	inline static Vector2 ZERO() {
		return Vector2(0, 0);
	}

	inline static Vector2 ONE() {
		return Vector2(1, 1);
	}
#pragma endregion Static getters
};