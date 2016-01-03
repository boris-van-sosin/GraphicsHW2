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
PerspectiveData PerspectiveWarpMatrix(const BoundingBox& boundingCube);
MatrixHomogeneous OrthographicProjectMatrix(const BoundingBox& boundingCube);
PerspectiveData PerspectiveWarpMatrix(const BoundingBox& boundingCube, double n, double f);

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
	ZBufferImage(size_t h, size_t w);
	~ZBufferImage();

	enum BGImageMode { CROP, STRECH, REPEAT };

	int GetHeight() const;
	int GetWidth() const;
	void SetSize(size_t h, size_t w);
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
};

#endif