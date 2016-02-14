#pragma once

#include <vector>
#include "Geometry.h"
#include "GeometricTransformations.h"

enum ShadingMode { SHADING_NONE, SHADING_FLAT, SHADING_GOURAUD, SHADING_PHONG };

class ModelAttr
{ // don't make this struct big
public:
	COLORREF color = RGB(0, 0, 255);
	COLORREF normal_color = RGB(0, 255, 0);
	COLORREF model_bbox_color = RGB(255, 0, 0);
	COLORREF subObj_bbox_color = RGB(255, 0, 0);
	bool forceColor = false;
	unsigned int line_width = 1;
	bool displayBBox = false;
	bool displaySubObjectBBox = false;
	double sensitivity = 0.1;
	bool removeBackFace = false;
	double AmbientCoefficient = 1.0, DiffuseCoefficient = 1.0, SpecularCoefficient = 1.0;
	int SpecularPower = 4;
	ShadingMode Shading = SHADING_PHONG;
	bool silluete = true;
	bool boundry = true;
	double AmbientIntensity = 1;
	bool is_wireframe = false;
};

class LightSource
{
public:

	enum LightSourceType { POINT, PLANE };

	LightSource();
	LightSource(const Point3D& or, const Vector3D& dir, LightSourceType t, double p);
	LightSource(const HomogeneousPoint& or, const HomogeneousPoint& dir, LightSourceType t, double p);
	LightSource(const LightSource& other);

	Vector3D Direction() const;

	HomogeneousPoint _origin, _offset; // offset - origin = direction
	LightSourceType _type;
	double _intensity[3];	// RGB
};

struct ClippingPlane
{
	ClippingPlane(double x_, double y_, double z_, double c_);
	ClippingPlane(const Point3D& p, const Vector3D& n);
	ClippingPlane(const ClippingPlane& other);

	double Apply(const Point3D& p) const;
	double Apply(const HomogeneousPoint& p) const;
	double Apply(double x_, double y_, double z_) const;

	Point3D Intersection(const Point3D& p0, const Point3D& p1) const;

	double x, y, z, c;
};
