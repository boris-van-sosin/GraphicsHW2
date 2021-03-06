#pragma once

#include <vector>
#include "Geometry.h"
#include "GeometricTransformations.h"

enum ShadingMode { SHADING_NONE, SHADING_FLAT, SHADING_GOURAUD, SHADING_PHONG };

enum BackFaceBehavior { BACKFACE_SHOW, BACKFACE_REMOVE_BACK, BACKFACE_REMOVE_FRONT };

class ModelAttr
{ // don't make this struct big
public:
	ModelAttr()
	{}

	ModelAttr(const BoundingBox& b)
		: minX(b.minX), maxX(b.maxX), minY(b.minY), maxY(b.maxY), minZ(b.minZ), maxZ(b.maxZ)
	{}

	BoundingBox GetBoundingBox() const
	{
		return BoundingBox(minX, maxX, minY, maxY, minZ, maxZ);
	}

	COLORREF color = RGB(0, 0, 255);
	COLORREF normal_color = RGB(0, 255, 0);
	COLORREF model_bbox_color = RGB(255, 0, 0);
	COLORREF subObj_bbox_color = RGB(255, 0, 0);
	bool forceColor = false;
	unsigned int line_width = 1;
	bool displayBBox = false;
	bool displaySubObjectBBox = false;
	double sensitivity = 0.1;
	BackFaceBehavior removeBackFace = BACKFACE_SHOW;
	double AmbientCoefficient = 1.0, DiffuseCoefficient = 1.0, SpecularCoefficient = 1.0;
	int SpecularPower = 4;
	ShadingMode Shading = SHADING_PHONG;
	bool silluete = true;
	bool boundry = true;
	double AmbientIntensity = 1;
	bool is_wireframe = false;
	bool castShadow = false;
	int shadowVolumeWireframe = -1;
	double opacity = 1.0;
	bool forceOpacity = false;
	// v_texture
	MatrixHomogeneous inv = Matrices::UnitMatrixHomogeneous; // inverse transformation matrix
	double minX, maxX, minY, maxY, minZ, maxZ;
	int v_texture = 0;
	double a = 0.001, turbPower = 2;
};

class LightSource
{
public:

	enum LightSourceType { POINT, PLANE, SPOT };

	LightSource();
	LightSource(const Point3D& or, const Vector3D& dir, LightSourceType t, double p);
	LightSource(const HomogeneousPoint& or, const HomogeneousPoint& dir, LightSourceType t, double p);
	LightSource(const LightSource& other);

	Vector3D Direction() const;

	HomogeneousPoint _origin, _offset; // offset - origin = direction
	LightSourceType _type;
	double _intensity[3];	// RGB
	double _minDot;
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
