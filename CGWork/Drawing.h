#ifndef _DRAWING_H
#define _DRAWING_H

#include <atlimage.h>
#include <set>

#include "Geometry.h"
#include "GeometricTransformations.h"
#include "DrawingSupport.h"
#include "ShadowVolume.h"

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
	void PushColor(COLORREF clr, double z, double opacity = 1.0);
	void Clear();
	COLORREF GetActualColor() const;
	COLORREF GetActualColor(COLORREF bgPixel, bool bgValid = true) const;
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
	void SetOpacity(double opacity = 1.0);
private:
	size_t _height, _width;
	ZBufferPixel* _img;
	bool _useBackgroundColor, _useBackgroundImg;
	COLORREF _backgroundColor;
	CImage _backgroundImage;
	BGImageMode _bgImageMode;
	double _currOpacity = 1.0;
};

class DrawingObject
{
public:
	DrawingObject();
	DrawingObject(CImage& cimg, ZBufferImage& zbimg);

	enum ActiveDrawingObject { DRAWING_OBJECT_ZBUF, DRAWING_OBJECT_CIMG, DRAWING_OBJECT_SV };

	ZBufferImage* zBufImg;
	CImage* img;
	ShadowVolume* shadowVolume;
	ActiveDrawingObject active = DRAWING_OBJECT_CIMG;
	int GetHeight() const;
	int GetWidth() const;
	void SetPixel(int x, int y, double z, COLORREF clr);
	void SetPixel(int x, int y, const Point3D& p0, const Point3D& p1, COLORREF clr);
private:
	double _near, _far;
	bool _doClip;
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
void DrawObject(DrawingObject& img, const PolygonalObject& obj, const MatrixHomogeneous& m, const ModelAttr& attr, const std::vector<Normals::PolygonNormalData>& normals, size_t normalsIdx, bool fillPolygons, bool clip = false, const ClippingPlane& cp = ClippingPlane(0, 0, 0, 0));

#endif