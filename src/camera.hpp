#pragma once
#include "vector.hpp"

class Camera {
  public:
	Vector3 origin;

	Camera() : origin(Vector3(0, 0, 0)) { }
	Camera(Vector3 origin) : origin(origin) { }
};