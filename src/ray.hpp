#pragma once
#include "vector.hpp"

class Ray {
  public:
	Vector3 origin;
	Vector3 direction;

	Ray(Vector3 origin, Vector3 direction) : origin(origin), direction(direction) { }
};