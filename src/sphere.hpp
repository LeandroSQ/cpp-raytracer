#pragma once
#include "vector.hpp"
#include "color.hpp"

class Sphere {
  public:
	Vector3 position;
	float radius;
    Color color;

	Sphere(Vector3 position, float radius, Color color) : position(position), radius(radius), color(color) { }
};