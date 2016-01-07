#ifndef _DRAWING_H
#define _DRAWING_H

#include "Geometry.h"
#include "GeometricTransformations.h"
#include <atlimage.h>
#include <set>

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

struct PerspectiveData
{
	PerspectiveData(const MatrixHomogeneous& ms, const MatrixHomogeneous& mp, const ClippingPlane& n, const ClippingPlane& f);

	MatrixHomogeneous ScaleAndMoveToView, PerspectiveWarp;
	ClippingPlane NearPlane, FarPlane;
};

MatrixHomogeneous ScaleAndCenter(const BoundingBox& boundingCube);
MatrixHomogeneous ScaleToCube(const BoundingBox& boundingCube);
MatrixHomogeneous CenterToCube(const BoundingBox& boundingCube, bool minus = false);
PerspectiveData PerspectiveWarpMatrix(const BoundingBox& boundingCube);
MatrixHomogeneous OrthographicProjectMatrix(const BoundingBox& boundingCube);
PerspectiveData PerspectiveWarpMatrix(const BoundingBox& boundingCube, double n, double f, int viewSize = 2);

class ZBufferPixel
{
public:
	void PushColor(COLORREF clr, double z);
	void Clear();
	COLORREF GetActualColor() const;
	COLORREF GetTopColor() const;
	bool IsEmpty() const;

	class ZBufferItem
	{
	public:
		ZBufferItem(COLORREF clr, double z_, double opacity = 1.0);

		double z;
		COLORREF color;
		double opacity;

		bool operator<(const ZBufferItem& other) const;
	};
private:
	std::set<ZBufferItem> _buffer;
};

class ZBufferImage
{
public:
	ZBufferImage();
	ZBufferImage(size_t w, size_t h);
	~ZBufferImage();

	enum BGImageMode { CROP, STRECH, REPEAT };

	int GetHeight() const;
	int GetWidth() const;
	void SetSize(size_t w, size_t h);
	void PushPixel(int x, int y, double z, COLORREF clr);
	void PushPixel(int x, int y, const Point3D& p0, const Point3D& p1, COLORREF clr);
	void Clear();
	void SetBackgroundColor(COLORREF clr);
	void SetBackgroundImage(CImage& img, BGImageMode imMode);
	void DrawOnImage(CImage& img) const;
private:
	size_t _height, _width;
	ZBufferPixel* _img;
	bool _useBackgroundColor, _useBackgroundImg;
	COLORREF _backgroundColor;
	CImage _backgroundImage;
	BGImageMode _bgImageMode;
};

class DrawingObject
{
public:
	DrawingObject();
	DrawingObject(CImage& cimg, ZBufferImage& zbimg);

	enum ActiveDrawingObject { DRAWING_OBJECT_ZBUF, DRAWING_OBJECT_CIMG };

	ZBufferImage* zBufImg;
	CImage* img;
	ActiveDrawingObject active = DRAWING_OBJECT_CIMG;
	int GetHeight() const;
	int GetWidth() const;
	void SetPixel(int x, int y, double z, COLORREF clr);
	void SetPixel(int x, int y, const Point3D& p0, const Point3D& p1, COLORREF clr);
private:
	double _near, _far;
	bool _doClip;
};

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
	ShadingMode Shading = SHADING_GOURAUD;
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

	HomogeneousPoint _origin, _offset;
	LightSourceType _type;
	double _intensity;
};

inline COLORREF ShiftColor(COLORREF c, int shift)
{
	int red = GetRValue(c) + shift;
	int green = GetGValue(c) + shift;
	int blue = GetBValue(c) + shift;
	if (red > 255)
		red = 255;
	else if (red < 0)
		red = 0;

	if (green > 255)
		green = 255;
	else if (green < 0)
		green = 0;

	if (blue > 255)
		blue = 255;
	else if (blue < 0)
		blue = 0;

	return RGB(red, green, blue);
}

extern const COLORREF DefaultModelColor;

inline COLORREF GetActualColor(COLORREF objColor, bool objColorValid, const Polygon3D& poly, const HomogeneousPoint& p, const ModelAttr& attr)
{
	COLORREF actualColor = DefaultModelColor;
	if (attr.forceColor)
	{
		actualColor = attr.color;
	}
	else if (p.colorValid)
	{
		actualColor = p.color;
	}
	else if (poly.colorValid)
	{
		actualColor = poly.color;
	}
	else if (objColorValid)
	{
		actualColor = objColor;
	}
	return actualColor;
}

struct ClippingResult
{
	ClippingResult(const LineSegment& ls, bool c0, bool c1)
		: lineSegment(ls), clippedFirst(c0), clippedSecond(c1)
	{
	}

	LineSegment lineSegment;
	bool clippedFirst, clippedSecond;
};

ClippingResult ApplyClipping(const HomogeneousPoint& p0, const HomogeneousPoint& p1, const ClippingPlane& cp);
void DrawLineSegment(DrawingObject& img, const Point3D& p0, const Point3D& p1, COLORREF clr, unsigned int line_width, bool clip = false, const ClippingPlane& cp = ClippingPlane(0, 0, 0, 0));
void DrawLineSegment(DrawingObject& img, const HomogeneousPoint& p0, const HomogeneousPoint& p1, COLORREF clr, unsigned int line_width, bool clip = false, const ClippingPlane& cp = ClippingPlane(0, 0, 0, 0));
void DrawLineSegment(DrawingObject& img, const LineSegment& line, COLORREF clr, unsigned int line_width, bool clip = false, const ClippingPlane& cp = ClippingPlane(0, 0, 0, 0));
void DrawObject(DrawingObject& img, const PolygonalObject& obj, const MatrixHomogeneous& m, const ModelAttr& attr, const std::vector<Normals::PolygonNormalData>& normals, bool fillPolygons, bool clip = false, const ClippingPlane& cp = ClippingPlane(0, 0, 0, 0));

#endif