#include "Drawing.h"
#include "Utils.h"

#include <assert.h>
#include <map>
#include <algorithm>

const COLORREF DefaultModelColor(RGB(0, 0, 255));
std::vector<LightSource> g_lights(10);
std::vector<ShadowVolume> g_ShadowVolumes;


PerspectiveData::PerspectiveData(const MatrixHomogeneous& ms, const MatrixHomogeneous& mp, const ClippingPlane& n, const ClippingPlane& f)
	: ScaleAndMoveToView(ms), PerspectiveWarp(mp), NearPlane(n), FarPlane(f)
{
}

MatrixHomogeneous ScaleAndCenter(const BoundingBox& boundingCube)
{
	const double vright = boundingCube.maxX;
	const double vleft = boundingCube.minX;
	const double vbottom = boundingCube.minY;
	const double vtop = boundingCube.maxY;
	const double vfar = -boundingCube.maxZ;
	const double vnear = -boundingCube.minZ;
	HomogeneousPoint rows[4] = {
		HomogeneousPoint(1 / (vright - vleft), 0, 0, -(vright + vleft) / (vright - vleft)),
		HomogeneousPoint(0, 1 / (vtop - vbottom), 0, -(vtop + vbottom) / (vtop - vbottom)),
		HomogeneousPoint(0, 0, 1 / (vnear - vfar), -(vnear + vfar) / (vfar - vnear)),
		HomogeneousPoint(0, 0, 0, 1)
	};
	return MatrixHomogeneous(rows);
}

MatrixHomogeneous ScaleToCube(const BoundingBox& boundingCube)
{
	const double vright = boundingCube.maxX;
	const double vleft = boundingCube.minX;
	const double vbottom = boundingCube.minY;
	const double vtop = boundingCube.maxY;
	const double vfar = -boundingCube.maxZ;
	const double vnear = -boundingCube.minZ;
	HomogeneousPoint rows[4] = {
		HomogeneousPoint(1 / (vright - vleft), 0, 0, 0),
		HomogeneousPoint(0, 1 / (vtop - vbottom), 0, 0),
		HomogeneousPoint(0, 0, 1 / (vnear - vfar), 0),
		HomogeneousPoint(0, 0, 0, 1)
	};
	return MatrixHomogeneous(rows);
}

MatrixHomogeneous CenterToCube(const BoundingBox& boundingCube, bool minus)
{
	const double vright = boundingCube.maxX;
	const double vleft = boundingCube.minX;
	const double vbottom = boundingCube.minY;
	const double vtop = boundingCube.maxY;
	const double vfar = -boundingCube.maxZ;
	const double vnear = -boundingCube.minZ;
	const double sign = minus ? 1 : -1;
	HomogeneousPoint rows[4] = {
		HomogeneousPoint(1, 0, 0, sign * (vright + vleft) / 2),
		HomogeneousPoint(0, 1, 0, sign * (vtop + vbottom) / 2),
		HomogeneousPoint(0, 0, 1, sign * (vnear + vfar) / 2),
		HomogeneousPoint(0, 0, 0, 1)
	};
	return MatrixHomogeneous(rows);
}

PerspectiveData PerspectiveWarpMatrix(const BoundingBox& boundingCube, double n, double f, int viewSize)
{
	const double near2 = n;
	const double far2 = f;
	const double q2 = far2 / (far2 - near2);

	//return MatrixHomogeneous(rows) * Matrices::Translate(0, 0, boundingCube.minZ + near + depth*0.5);
	HomogeneousPoint rows2[] = {
		HomogeneousPoint(viewSize, 0, 0, 0),
		HomogeneousPoint(0, viewSize, 0, 0),
		HomogeneousPoint(0, 0, q2, 1),
		HomogeneousPoint(0, 0, -q2*near2, 0)
	};

	return PerspectiveData(
		Matrices::Translate(0, 0, 2) * ScaleAndCenter(boundingCube),
		MatrixHomogeneous(rows2),
		ClippingPlane(0, 0, 1, -near2),
		ClippingPlane(0, 0, -1, -far2));
}

PerspectiveData PerspectiveWarpMatrix(const BoundingBox& boundingCube)
{
	const double depth = boundingCube.maxZ - boundingCube.minZ;
	const double clippingMargin = depth * 0.1;
	const double nearPlane = 1 - clippingMargin;
	const double farPlane = nearPlane + depth + clippingMargin;
	return PerspectiveWarpMatrix(boundingCube, nearPlane, farPlane);
}

MatrixHomogeneous OrthographicProjectMatrix(const BoundingBox& boundingCube)
{
	HomogeneousPoint rows[4] = {
		HomogeneousPoint(1, 0, 0, 0),
		HomogeneousPoint(0, 1, 0, 0),
		HomogeneousPoint(0, 0, 0, 0),
		HomogeneousPoint(0, 0, 0, 1)
	};
	return MatrixHomogeneous(rows) * ScaleAndCenter(boundingCube);
}

ZBufferImage::ZBufferImage()
	: _height(0), _width(0), _useBackgroundColor(false), _useBackgroundImg(false), _img(NULL)
{
}

ZBufferImage::ZBufferImage(size_t w, size_t h)
	: _height(h), _width(w), _useBackgroundColor(false), _useBackgroundImg(false)
{
	_img = new ZBufferPixel[_height*_width];
}

ZBufferImage::~ZBufferImage()
{
	delete[] _img;
}

int ZBufferImage::GetHeight() const
{
	return _height;
}

int ZBufferImage::GetWidth() const
{
	return _width;
}

void ZBufferImage::SetSize(size_t w, size_t h)
{
	if (h == _height && w == _width)
	{
		Clear();
		return;
	}

	_height = h;
	_width = w;
	delete[] _img;
	_img = new ZBufferPixel[_height*_width];
}

void ZBufferImage::Clear()
{
	for (size_t i = 0; i < _height*_width; ++i)
	{
		_img[i].Clear();
	}
}

void ZBufferImage::SetBackgroundColor(COLORREF clr)
{
	_backgroundColor = clr;
	_useBackgroundColor = true;
	_useBackgroundImg = false;
}

void ZBufferImage::SetBackgroundImage(CImage& img, BGImageMode imMode)
{
	_bgImageMode = imMode;
	_useBackgroundImg = true;
	_useBackgroundColor = false;
	CopyImage(img, _backgroundImage);
}

void ZBufferImage::PushPixel(int x, int y, double z, COLORREF clr)
{
	if (x < 0 || y < 0 || x >= _width || y >= _height)
	{
		return;
	}
	_img[y*_width + x].PushColor(clr, z, _currOpacity);
}

void ZBufferImage::PushPixel(int x, int y, const Point3D& p0, const Point3D& p1, COLORREF clr)
{
	const double relativeT = ((double)x - p0.x) / (p1.x - p0.x);
	const double z = p0.z + (p1.z - p0.z)*relativeT;

	_img[y*_width + x].PushColor(clr, z, _currOpacity);
}

void ZBufferImage::DrawOnImage(CImage& img) const
{
	if ((!img.IsNull()) &&
		(img.GetHeight() != _height || img.GetWidth() != _width))
	{
		img.Destroy();
		img.Create(_width, _height, 32);
	}
	else if (img.IsNull())
	{
		img.Create(_width, _height, 32);
	}

	if (_useBackgroundColor)
	{
		HDC imgDC = img.GetDC();
		RECT rect;
		rect.top = rect.left = 0;
		rect.bottom = _height;
		rect.right = _width;
		HBRUSH brsh = CreateSolidBrush(_backgroundColor);
		FillRect(imgDC, &rect, brsh);
		DeleteObject(brsh);
		img.ReleaseDC();
	}
	else if (_useBackgroundImg)
	{
		switch (_bgImageMode)
		{
		case CROP:
		{
			HDC imgDC = img.GetDC();
			_backgroundImage.BitBlt(imgDC, 0, 0, _width, _height, 0, 0);
			img.ReleaseDC();
			break;
		}
		case STRECH:
		{
			HDC imgDC = img.GetDC();
			_backgroundImage.StretchBlt(imgDC, 0, 0, _width, _height, 0, 0, _backgroundImage.GetWidth(), _backgroundImage.GetHeight());
			img.ReleaseDC();
			break;
		}
		case REPEAT:
		{
			HDC imgDC = img.GetDC();
			int bgWidth = _backgroundImage.GetWidth(), bgHeight = _backgroundImage.GetHeight();
			for (int i = 0; i < _height; i += bgHeight)
			{
				for (int j = 0; j < _width; j += bgWidth)
				{
					int actualWidth = min(bgWidth, _width - j - 1);
					int actualHeight = min(bgHeight, _height - i - 1);
					_backgroundImage.BitBlt(imgDC, j, i, bgWidth, bgHeight, 0, 0);
				}
			}
			img.ReleaseDC();
			break;
		}
		default:
			break;
		}
	}
	
	for (size_t x = 0; x < _width; ++x)
	{
		for (size_t y = 0; y < _height; ++y)
		{
			if (_img[y*_width + x].IsEmpty())
				continue;
			BYTE* pos = (BYTE*)img.GetPixelAddress(x, y);
			const COLORREF clr = _img[y*_width + x].GetActualColor(RGB(*pos, *(pos + 1), *(pos + 2)));
			*pos = GetBValue(clr);
			*(pos + 1) = GetGValue(clr);
			*(pos + 2) = GetRValue(clr);
		}
	}
}

void ZBufferImage::SetOpacity(double opacity)
{
	_currOpacity = opacity;
}

ZBufferPixel::ZBufferItem::ZBufferItem(COLORREF clr, double z_, double op)
	: z(z_), color(clr), opacity(op)
{
}

void ZBufferPixel::PushColor(COLORREF clr, double z, double opacity)
{
	_buffer.insert(ZBufferItem(clr, z, opacity));
}

void ZBufferPixel::Clear()
{
	_buffer.clear();
}

bool ZBufferPixel::IsEmpty() const
{
	return _buffer.empty();
}

COLORREF ZBufferPixel::GetActualColor(COLORREF bgPixel, bool bgValid) const
{
	std::set<ZBufferPixel::ZBufferItem>::const_iterator it = _buffer.begin();

	if (it->opacity >= 1.0)
	{
		return it->color;
	}

	double red = (double)(GetRValue(it->color));
	double green = (double)(GetGValue(it->color));
	double blue = (double)(GetBValue(it->color));

	double prevOpacity = it->opacity;

	++it;
	for (; it != _buffer.end(); ++it)
	{
		int currRed = GetRValue(it->color);
		int currGreen = GetGValue(it->color);
		int currBlue = GetBValue(it->color);
		red = red * prevOpacity + (1 - prevOpacity)*currRed;
		green = green * prevOpacity + (1 - prevOpacity)*currGreen;
		blue = blue * prevOpacity + (1 - prevOpacity)*currBlue;
		prevOpacity = it->opacity;
		if (it->opacity >= 1.0)
		{
			break;
		}
	}

	if (bgValid && prevOpacity < 1.0)
	{
		int bgRed = GetRValue(bgPixel);
		int bgGreen = GetGValue(bgPixel);
		int bgBlue = GetBValue(bgPixel);
		red = red * prevOpacity + (1 - prevOpacity)*bgRed;
		green = green * prevOpacity + (1 - prevOpacity)*bgGreen;
		blue = blue * prevOpacity + (1 - prevOpacity)*bgBlue;
	}

	int finalRed = (int)red;
	int finalGreen = (int)green;
	int finalBlue = (int)blue;

	return RGB(finalRed, finalGreen, finalBlue);
}

COLORREF ZBufferPixel::GetActualColor() const
{
	return GetActualColor(0, false);
}

COLORREF ZBufferPixel::GetTopColor() const
{
	return _buffer.begin()->color;
}

bool ZBufferPixel::ZBufferItem::operator<(const ZBufferItem& other) const
{
	return z < other.z;
}

void inline ImageSetPixel(CImage& img, int x, int y, COLORREF clr)
{
	int red = GetRValue(clr);
	int green = GetGValue(clr);
	int blue = GetBValue(clr);

	if (x >= img.GetWidth() || y >= img.GetHeight() || x < 0 || y < 0)
	{
		return;
	}

	BYTE* pos = (BYTE*)img.GetPixelAddress(x, y);
	*pos = blue;
	*(pos + 1) = green;
	*(pos + 2) = red;
}

DrawingObject::DrawingObject()
	: zBufImg(NULL), img(NULL), _near(0.0), _far(0.0), _doClip(false)
{
}

DrawingObject::DrawingObject(CImage& cimg, ZBufferImage& zbimg)
	: zBufImg(&zbimg), img(&cimg), _near(0.0), _far(0.0), _doClip(false)
{
}

int DrawingObject::GetHeight() const
{
	if (active == DRAWING_OBJECT_CIMG && img)
		return img->GetHeight();
	else if (active == DRAWING_OBJECT_ZBUF && zBufImg)
		return zBufImg->GetHeight();
	else if (shadowVolume)
		return shadowVolume->GetHeight();
	else
		return 0;
}

int DrawingObject::GetWidth() const
{
	if (active == DRAWING_OBJECT_CIMG && img)
		return img->GetWidth();
	else if (active == DRAWING_OBJECT_ZBUF && zBufImg)
		return zBufImg->GetWidth();
	else if (shadowVolume)
		return shadowVolume->GetWidth();
	else
		return 0;
}

void DrawingObject::SetPixel(int x, int y, double z, COLORREF clr)
{
	/*if (_doClip &&
		z < _near)
	{
		return;
	}*/
	if (active == DRAWING_OBJECT_CIMG && img)
		ImageSetPixel(*img, x, y, clr);
	else if (active == DRAWING_OBJECT_ZBUF && zBufImg)
		zBufImg->PushPixel(x, y, z, clr);
	else if (shadowVolume)
		shadowVolume->SetPixel(x, y, z, clr);
}

void DrawingObject::SetPixel(int x, int y, const Point3D& p0, const Point3D& p1, COLORREF clr)
{
	double z = (p1.x != p0.x) ?
			p0.z
			:
			(((x - p0.x) / (p1.x - p0.x)) * (p1.z - p0.z) + p0.z);
	SetPixel(x, y, z, clr);
}

/////////////////////////////////////////////////////////////////////////////
// CCGWorkView drawing
/////////////////////////////////////////////////////////////////////////////

//COLORREF ApplyLight(const std::vector<LightSource>& lights, const MixedIntPoint& pixel, const ModelAttr& attr, COLORREF clr, const Vector3D& normal, const Point3D& viewPoint, double ambient);

struct MixedIntPoint
{
	MixedIntPoint(int x_, int y_, double z_, COLORREF clr, const Vector3D& n, const Point3D& ospt)
		: x(x_), y(y_), z(z_), color(clr), normal(n), objSpacePt(ospt)
	{}

	MixedIntPoint(int x_, int y_, double z_, const Point3D& ospt)
		: MixedIntPoint(x_, y_, z_, 0, Vector3D(), ospt)
	{}

	MixedIntPoint()
		: x(0), y(0), z(0)
	{}
	MixedIntPoint(int x_, int y_)
		: MixedIntPoint(x_, y_, 0.0, Point3D())
	{}

	MixedIntPoint(int x_, int y_, double z_)
		: MixedIntPoint(x_, y_, z_, Point3D())
	{}


	MixedIntPoint(const MixedIntPoint& other)
		: MixedIntPoint(other.x, other.y, other.z, other.color, other.normal, other.objSpacePt)
	{}

	MixedIntPoint(const Point3D& p, const Point3D& ospt)
		: MixedIntPoint(p.x, p.y, p.z, ospt)
	{
		if (p.colorValid)
			color = p.color;
	}

	MixedIntPoint(const HomogeneousPoint& p, const HomogeneousPoint& ospt)
		: MixedIntPoint(Point3D(p), Point3D(ospt))
	{}

	MixedIntPoint(const Point3D& p)
		: MixedIntPoint(p, Point3D())
	{}

	MixedIntPoint(const HomogeneousPoint& p)
		: MixedIntPoint(Point3D(p), Point3D())
	{}

	MixedIntPoint(const Point3D& p, COLORREF clr, const Vector3D& n, const Point3D& ospt)
		: MixedIntPoint(p.x, p.y, p.z, clr, n, ospt)
	{}

	MixedIntPoint(const HomogeneousPoint& p, COLORREF clr, const Vector3D& n, const HomogeneousPoint& ospt)
		: MixedIntPoint(Point3D(p), clr, n, Point3D(ospt))
	{}

	int x, y;
	double z;
	COLORREF color;
	Vector3D normal;
	Point3D objSpacePt;
};

template <typename T>
void CGSwap(T& x, T& y) {
	T  t = x;
	x = y;
	y = t;
}

template<typename T>
T LinearInterpolate(double x, double minX, double maxX, const T& t0, const T& t1)
{
	if (minX == maxX)
		return t0;
	double b = (x - minX) / (maxX - minX);
	return (1 - b)*t0 + b*t1;
}

COLORREF ColorInterpolate(double x, double minX, double maxX, COLORREF clr0, COLORREF clr1, bool swap = false)
{
	double floatColors[] = { GetRValue(clr0), GetGValue(clr0), GetBValue(clr0), GetRValue(clr1), GetGValue(clr1), GetBValue(clr1) };
	int newColors[3];
	for (int i = 0; i < 3; ++i)
	{
		newColors[i] = LinearInterpolate(x, minX, maxX, floatColors[i], floatColors[i + 3]);
	}
	return RGB(newColors[0], newColors[1], newColors[2]);
}

COLORREF ColorInterpolateN(double x, double minX, double maxX, COLORREF clr0, COLORREF clr1, const Vector3D&, bool swap = false)
{
	return ColorInterpolate(x, minX, maxX, clr0, clr1, swap);
}

COLORREF ColorInterpolateSV(double x, double minX, double maxX, const MixedIntPoint&, COLORREF clr0, COLORREF clr1, const Point3D&, const Vector3D&, bool swap = false)
{
	return ColorInterpolate(x, minX, maxX, clr0, clr1, swap);
}

struct XData
{
	enum PixelType { MIDDLE, UPPER_LIMIT, LOWER_LIMIT, LEFT, RIGHT };
	XData(int x_, double z_, int lid, COLORREF clr, const Vector3D& n, const Point3D& ospt, PixelType pt = MIDDLE)
		: x(x_), z(z_), LineId(lid), Type(pt), color(clr), normal(n), objSpacePt(ospt)
	{}
	XData(int x_, double z_, int lid, const Point3D& ospt, PixelType pt = MIDDLE)
		: x(x_), z(z_), LineId(lid), objSpacePt(ospt), Type(pt)
	{}

	int x;
	int LineId;
	PixelType Type;
	double z;
	COLORREF color;
	Vector3D normal;
	Point3D objSpacePt;
};

bool operator<(const XData& a, const XData& b)
{
	if (a.x != b.x)
		return a.x < b.x;
	else
		return a.LineId < b.LineId;
}

struct FakeXYMap
{
	void SetPixel(int x, int y, double z, COLORREF clr, int lineId, const Vector3D& n, const Point3D& objSpPt, XData::PixelType t = XData::MIDDLE)
	{
		minY = min(minY, y);
		maxY = max(maxY, y);
		if (xyMap.find(y) == xyMap.end())
		{
			xyMap[y] = std::vector<XData>();
		}
		xyMap[y].push_back(XData(x, z, lineId, clr, n, objSpPt, t));
	}

	void SetPixel(int x, int y, double z, COLORREF clr)
	{
		SetPixel(x, y, z, 0, clr, Point3D(), Vector3D());
	}


	std::map<int, std::vector<XData>> xyMap;	// y => <x, line id, pixel type>
	int minY = INT_MAX, maxY = -1;
};

template <typename T>
void innerDrawLine(T& img, const MixedIntPoint& p0, const MixedIntPoint& p1, COLORREF clr, unsigned int line_width, bool z = false) {
	int x0 = p0.x, y0 = p0.y,
		x1 = p1.x, y1 = p1.y;
	double z0 = p0.z, z1 = p1.z;

	if (x0 > x1) {
		CGSwap(x0, x1);
		CGSwap(y0, y1);
		CGSwap(z0, z1);
	}

	int dx = x1 - x0;
	int dy = y1 - y0;

	bool swapXY = false;
	if (abs(dy) > dx)
	{
		CGSwap(x0, y0);
		CGSwap(x1, y1);
		if (x0 > x1) {
			CGSwap(x0, x1);
			CGSwap(y0, y1);
			CGSwap(z0, z1);
		}
		dx = x1 - x0;
		dy = y1 - y0;
		swapXY = true;
	}

	bool reverseY = (dy < 0); // was: && ((-dy) < dx)
	if (reverseY)
	{
		dy = -dy;
	}

	if (dx > 0 && dy >= 0 && dy <= dx) {

		int err = 2 * dy - dx;
		int horizontal = 2 * dy, diagonal = 2 * (dy - dx);
		{
			const double currZ = z ? z0 : 0.0;
			if (!swapXY)
			{
				img.SetPixel(x0, y0, currZ, clr);
				for (unsigned int i = 1; i < line_width; i++) {
					img.SetPixel(x0, y0 + i, currZ, clr);
					img.SetPixel(x0, y0 - i, currZ, clr);
				}
			}
			else
			{
				img.SetPixel(y0, x0, currZ, clr);
				for (unsigned int i = 1; i < line_width; i++) {
					img.SetPixel(y0 + i, x0, currZ, clr);
					img.SetPixel(y0 - i, x0, currZ, clr);
				}
			}
		}
		int y = y0;

		for (int x = x0 + 1; x <= x1; x++) {
			if (err < 0) {
				err = err + horizontal;
			}
			else
			{
				err = err + diagonal;
				if (!reverseY)
				{
					++y;
				}
				else
				{
					--y;
				}
			}
			const double currZ = z ? (swapXY ? LinearInterpolate(y, y0, y1, z0, z1) : LinearInterpolate(x, x0, x1, z0, z1)) : 0.0;
			if (!swapXY)
			{
				img.SetPixel(x, y, currZ, clr);
				for (unsigned int i = 1; i < line_width; i++) {
					img.SetPixel(x, y + 1, currZ, clr);
					img.SetPixel(x, y - 1, currZ, clr);
				}
			}
			else
			{
				img.SetPixel(y, x, currZ, clr);
				for (unsigned int i = 1; i < line_width; i++) {
					img.SetPixel(y + i, x, currZ, clr);
					img.SetPixel(y - i, x, currZ, clr);
				}
			}
		}
	}
	else if (dx == 0 && dy == 0)
	{
		img.SetPixel(x0, y0, z0, clr);
		for (unsigned int i = 1; i < line_width; i++) {
			img.SetPixel(x0, y0 + i, z0, clr);
			img.SetPixel(x0, y0 - i, z0, clr);
			img.SetPixel(x0 + i, y0, z0, clr);
			img.SetPixel(x0 - i, y0, z0, clr);
		}
	}
}

template <typename Z_FUNC, typename CLR_FUNC, typename N_FUNC, typename OSPT_FUNC>
void innerDrawLine(FakeXYMap& img, const MixedIntPoint& p0, const MixedIntPoint& p1, int lineId, unsigned int line_width, Z_FUNC zf, CLR_FUNC clrf, N_FUNC nf, OSPT_FUNC osptf) {
	int x0 = p0.x, y0 = p0.y,
		x1 = p1.x, y1 = p1.y;
	double z0 = p0.z, z1 = p1.z;
	COLORREF c0 = p0.color, c1 = p1.color;
	Vector3D n0 = p0.normal, n1 = p1.normal;
	Point3D ospt0 = p0.objSpacePt, ospt1 = p1.objSpacePt;
	bool swapX = false;

	if (x0 > x1) {
		CGSwap(x0, x1);
		CGSwap(y0, y1);
		CGSwap(z0, z1);
		CGSwap(c0, c1);
		CGSwap(n0, n1);
		CGSwap(ospt0, ospt1);
		swapX = !swapX;
	}

	int dx = x1 - x0;
	int dy = y1 - y0;

	bool swapXY = false;
	if (abs(dy) > dx)
	{
		CGSwap(x0, y0);
		CGSwap(x1, y1);
		//CGSwap(ospt0.x, ospt0.y);
		//CGSwap(ospt1.x, ospt1.y);
		if (x0 > x1) {
			CGSwap(x0, x1);
			CGSwap(y0, y1);
			CGSwap(z0, z1);
			CGSwap(c0, c1);
			CGSwap(n0, n1);
			CGSwap(ospt0, ospt1);
			swapX = !swapX;
		}
		dx = x1 - x0;
		dy = y1 - y0;
		swapXY = true;
	}

	bool reverseY = (dy < 0); // was: && ((-dy) < dx)
	if (reverseY)
	{
		dy = -dy;
	}

	if (dx > 0 && dy >= 0 && dy <= dx) {

		int err = 2 * dy - dx;
		int horizontal = 2 * dy, diagonal = 2 * (dy - dx);
		{
			if (!swapXY)
			{
				img.SetPixel(x0, y0, z0, c0, lineId, n0, ospt0);
				for (unsigned int i = 1; i < line_width; i++) {
					img.SetPixel(x0, y0 + i, z0, c0, lineId, n0, ospt0);
					img.SetPixel(x0, y0 - i, z0, c0, lineId, n0, ospt0);
				}
			}
			else
			{
				img.SetPixel(y0, x0, z0, c0, lineId, n0, ospt0);
				for (unsigned int i = 1; i < line_width; i++) {
					img.SetPixel(y0 + i, x0, z0, c0, lineId, n0, ospt0);
					img.SetPixel(y0 - i, x0, z0, c0, lineId, n0, ospt0);
				}
			}
		}
		int y = y0;

		for (int x = x0 + 1; x <= x1; x++) {
			if (err < 0) {
				err = err + horizontal;
			}
			else
			{
				err = err + diagonal;
				if (!reverseY)
				{
					++y;
				}
				else
				{
					--y;
				}
			}
			/*const double currZ = swapXY ?
				(reverseY ? zf(y, y1, y0, z1, z0) : zf(y, y0, y1, z0, z1))
				:
				zf(x, x0, x1, z0, z1);
			const Vector3D currN = swapXY ?
				(reverseY ? nf(y, y1, y0, n1, n0) : nf(y, y0, y1, n0, n1))
				:
				nf(x, x0, x1, n0, n1);
			const COLORREF currClr = swapXY ?
				(reverseY ? clrf(y, y1, y0, c0, c1, currN) : clrf(y, y0, y1, c0, c1, currN))
				:
				clrf(x, x0, x1, c0, c1, currN);*/
			const double currZ = zf(x, x0, x1, z0, z1);
			const Vector3D currN = nf(x, x0, x1, n0, n1);
			const Vector3D currObjSpPt = osptf(x, x0, x1, ospt0, ospt1);
			const COLORREF currClr = (!swapXY) ? clrf(x, x0, x1, MixedIntPoint(x, y, currZ), c0, c1, currObjSpPt, currN, swapX) : clrf(x, x0, x1, MixedIntPoint(y, x, currZ), c0, c1, currObjSpPt, currN, swapX);
			if (!swapXY)
			{
				img.SetPixel(x, y, currZ, currClr, lineId, currN, currObjSpPt);
				for (unsigned int i = 1; i < line_width; i++) {
					img.SetPixel(x, y + 1, currZ, currClr, lineId, currN, currObjSpPt);
					img.SetPixel(x, y - 1, currZ, currClr, lineId, currN, currObjSpPt);
				}
			}
			else
			{
				img.SetPixel(y, x, currZ, currClr, lineId, currN, currObjSpPt);
				for (unsigned int i = 1; i < line_width; i++) {
					img.SetPixel(y + i, x, currZ, currClr, lineId, currN, currObjSpPt);
					img.SetPixel(y - i, x, currZ, currClr, lineId, currN, currObjSpPt);
				}
			}
		}
	}
	else if (dx == 0 && dy == 0)
	{
		img.SetPixel(x0, y0, z0, p0.color, lineId, p0.normal, p0.objSpacePt);
		for (unsigned int i = 1; i < line_width; i++) {
			img.SetPixel(x0, y0 + i, z0, p0.color, lineId, p0.normal, p0.objSpacePt);
			img.SetPixel(x0, y0 - i, z0, p0.color, lineId, p0.normal, p0.objSpacePt);
			img.SetPixel(x0 + i, y0, z0, p0.color, lineId, p0.normal, p0.objSpacePt);
			img.SetPixel(x0 - i, y0, z0, p0.color, lineId, p0.normal, p0.objSpacePt);
		}
	}
}

void DrawLineSegment(DrawingObject& img, const Point3D& p0, const Point3D& p1, COLORREF clr, unsigned int line_width, bool clip, const ClippingPlane& cp)
{
	if (isnan(p0.x) || isnan(p0.y) || isnan(p1.x) || isnan(p1.y) ||
		fabs(p0.x) > 1e6 || fabs(p0.y) > 1e6 || fabs(p1.x) > 1e6 || fabs(p1.y) > 1e6)
	{
		return;
	}
	if (clip)
	{
		const double clipValue0 = cp.Apply(p0);
		const double clipValue1 = cp.Apply(p1);

		if (clipValue0 < 0 && clipValue1 < 0)
		{
			return;
		}
		else if (clipValue0 < 0)
		{
			const Point3D clippingPoint = cp.Intersection(p0, p1);
			innerDrawLine(img, clippingPoint, p1, clr, line_width);
		}
		else if (clipValue1 < 0)
		{
			const Point3D clippingPoint = cp.Intersection(p0, p1);
			innerDrawLine(img, p0, clippingPoint, clr, line_width);
		}
		else
		{
			innerDrawLine(img, p0, p1, clr, line_width);
		}
	}
	else
	{
		innerDrawLine(img, p0, p1, clr, line_width);
	}
}

void DrawLineSegment(DrawingObject& img, const HomogeneousPoint& p0, const HomogeneousPoint& p1, COLORREF clr, unsigned int line_width, bool clip, const ClippingPlane& cp)
{
	DrawLineSegment(img, Point3D(p0), Point3D(p1), clr, line_width, clip, cp);
}

void DrawLineSegment(DrawingObject& img, const LineSegment& line, COLORREF clr, unsigned int line_width, bool clip, const ClippingPlane& cp)
{
	DrawLineSegment(img, line.p0, line.p1, clr, line_width, clip, cp);
}

ClippingResult ApplyClipping(const HomogeneousPoint& p0, const HomogeneousPoint& p1, const ClippingPlane& cp)
{
	const double clipValue0 = cp.Apply(p0);
	const double clipValue1 = cp.Apply(p1);

	if (clipValue0 < 0 && clipValue1 < 0)
	{
		return ClippingResult(LineSegment(HomogeneousPoint::Zeros, HomogeneousPoint::Zeros), true, true);
	}
	else if (clipValue0 < 0)
	{
		const HomogeneousPoint clippingPoint = HomogeneousPoint(cp.Intersection(Point3D(p0), Point3D(p1)));
		return ClippingResult(LineSegment(clippingPoint, p1), true, false);
	}
	else if (clipValue1 < 0)
	{
		const HomogeneousPoint clippingPoint = HomogeneousPoint(cp.Intersection(Point3D(p0), Point3D(p1)));
		return ClippingResult(LineSegment(p0, clippingPoint), false, true);
	}
	else
	{
		return ClippingResult(LineSegment(p0, p1), false, false);
	}
}

Polygon3D ApplyClipping(const Polygon3D& poly, const ClippingPlane& cp, const Normals::PolygonNormalData& nd = Normals::PolygonNormalData(LineSegment(HomogeneousPoint::Zeros, HomogeneousPoint::Zeros)), Normals::PolygonNormalData& newNd = Normals::PolygonNormalData(LineSegment(HomogeneousPoint::Zeros, HomogeneousPoint::Zeros)))
{
	if (!nd.VertexNormals.empty())
	{
		newNd.VertexNormals.clear();
		newNd.PolygonNormal = nd.PolygonNormal;
	}

	std::vector<HomogeneousPoint> resPoints;
	resPoints.reserve(poly.points.size() + 2);
	newNd.VertexNormals.reserve(resPoints.size());
	for (size_t i = 0; i < poly.points.size(); ++i)
	{
		const HomogeneousPoint p0 = poly.points[i];
		const HomogeneousPoint p1 = (i + 1 < poly.points.size()) ? poly.points[i+1] : poly.points.front();
		const ClippingResult clipRes = ApplyClipping(p0, p1, cp);
		if ((!clipRes.clippedFirst) && (!clipRes.clippedSecond))
		{
			resPoints.push_back(clipRes.lineSegment.p0);
			if (!nd.VertexNormals.empty())
			{
				newNd.VertexNormals.push_back(nd.VertexNormals[i]);
			}
		}
		else if (clipRes.clippedFirst ^ clipRes.clippedSecond)
		{
			resPoints.push_back(clipRes.lineSegment.p0);
			resPoints.push_back(clipRes.lineSegment.p1);
			if (!nd.VertexNormals.empty())
			{
				if (clipRes.clippedSecond)
				{
					newNd.VertexNormals.push_back(nd.VertexNormals[i]);
					const Point3D endpP0 = Point3D(nd.VertexNormals[i].p1);
					const Point3D endpP1 = Point3D((i + 1 < poly.points.size()) ? nd.VertexNormals[i + 1].p1 : nd.VertexNormals.front().p1);
					const HomogeneousPoint interpEndp(LinearInterpolate(clipRes.lineSegment.p1.z, p0.z, p1.z, endpP0, endpP1));
					newNd.VertexNormals.push_back(LineSegment(clipRes.lineSegment.p1, interpEndp));
				}
				else
				{
					newNd.VertexNormals.push_back((i + 1 < poly.points.size()) ? nd.VertexNormals[i + 1] : nd.VertexNormals.front());
					const Point3D endpP0 = Point3D(nd.VertexNormals[i].p1);
					const Point3D endpP1 = Point3D((i + 1 < poly.points.size()) ? nd.VertexNormals[i + 1].p1 : nd.VertexNormals.front().p1);
					const HomogeneousPoint interpEndp(LinearInterpolate(clipRes.lineSegment.p0.z, p0.z, p1.z, endpP0, endpP1));
					newNd.VertexNormals.push_back(LineSegment(clipRes.lineSegment.p0, interpEndp));
				}
			}
		}
	}
	return Polygon3D(resPoints);
}

void CollapseSequences(std::vector<XData>& row)
{
	for (int i = 0; i < (row.size() - 1);)
	{
		const size_t sz = row.size();
		bool deleted = false;
		for (int j = i + 1; j < min(sz, i + 3); ++j)
		{
			if ((row[i].x + 1) == row[j].x &&
				row[i].LineId == row[j].LineId)
			{
				if (row[i].Type == XData::MIDDLE &&
					row[j].Type == XData::MIDDLE)
				{
					row.erase(row.begin() + i);
					deleted = true;
					break;
				}
				else if (row[i].Type == XData::MIDDLE &&
					row[j].Type != XData::MIDDLE)
				{
					int lastX = row[j].x;
					while (i >= 0)
					{
						if (row[i].Type == XData::MIDDLE &&
							row[i].LineId == row[j--].LineId &&
							row[i].x == lastX - 1)
						{
							row.erase(row.begin() + i);
							--lastX;
						}
						--i;
					}
					if (i < 0)
						i = 0;
					deleted = true;
					break;
				}
				else if (row[i].Type != XData::MIDDLE &&
					row[j].Type == XData::MIDDLE)
				{
					int lastX = row[i].x;
					while (j < row.size()){
						if (row[j].Type == XData::MIDDLE &&
							row[j].LineId == row[i].LineId &&
							row[j].x == lastX + 1)
						{
							row.erase(row.begin() + j);
							++lastX;
						}
						else
							++j;
					}
					deleted = true;
					break;
				}
			}
		}
		if (!deleted)
			++i;
	}
}

std::vector<XData>::const_iterator innerBinarySearchInXDataVector(const std::vector<XData>::const_iterator a, const std::vector<XData>::const_iterator b, const std::vector<XData>::const_iterator end, int x)
{
	if (a != end && a->x == x)
		return a;
	else if (b != end && b->x == x)
		return b;
	else if (a == b)
		return end;
	const std::vector<XData>::const_iterator mid = a + ((b - a) >> 1);
	if (mid != end && mid->x == x)
		return mid;

	const std::vector<XData>::const_iterator leftRes = innerBinarySearchInXDataVector(a, mid, end, x);
	if (leftRes != end)
		return leftRes;

	const std::vector<XData>::const_iterator rightRes = innerBinarySearchInXDataVector(mid + 1, b, end, x);
	if (rightRes != end)
		return rightRes;
	return end;
}

std::vector<XData>::const_iterator BinarySearchInXDataVector(const std::vector<XData>& v, int x)
{
	return innerBinarySearchInXDataVector(v.begin(), v.end(), v.end(), x);
}

COLORREF ApplyLight(const std::vector<LightSource>& lights, const Point3D& pt, const ModelAttr& attr, COLORREF clr, const Vector3D& normal, const Point3D& viewPoint, double ambient);
COLORREF ApplyLight(const std::vector<LightSource>& lights, const std::vector<ShadowVolume>& svs, const MixedIntPoint& viewPt, const Point3D& pt, const ModelAttr& attr, COLORREF clr, const Vector3D& normal, const Point3D& viewPoint, double ambient);

struct ZCalc
{
	double operator()(int x, int x0, int x1, double z0, double z1)
	{
		return LinearInterpolate(x, x0, x1, z0, z1);
	}
}; 

double ZZero(int x, int x0, int x1, double z0, double z1)
{
	return 0.0;
}

struct ColorFlat
{
	ColorFlat(COLORREF c)
		: clr(c)
	{
	}

	COLORREF operator()(int x, int x0, int x1, const MixedIntPoint& pt, COLORREF c0, COLORREF c1, const Vector3D&, bool swap = false)
	{
		return clr;
	}

	COLORREF operator()(int x, int x0, int x1, const MixedIntPoint& pt, COLORREF c0, COLORREF c1, const Point3D&, const Vector3D&, bool swap = false)
	{
		return clr;
	}


	COLORREF clr;
};

Vector3D NormalZero(int x, int x0, int x1, const Vector3D& v0, const Vector3D& v1)
{
	return Point3D::Zero;
}

struct ColorPhong
{
	ColorPhong(const HomogeneousPoint& p0, const HomogeneousPoint& p1, const std::vector<LightSource>& lights, const std::vector<ShadowVolume>& svs, const ModelAttr& attr, COLORREF modelColor, const Point3D& viewPoint, double ambient)
		: _lights(&lights), _shadowVolumes(&svs), _attr(&attr), _viewPoint(viewPoint), _ambient(ambient), _p0(p0), _p1(p1), _modelColor(modelColor)
	{
	}

	ColorPhong()
	{}

	COLORREF operator()(int x, int x0, int x1, const MixedIntPoint& viewPt, COLORREF c0, COLORREF c1, const Vector3D& n, bool swap)
	{
		if (!swap)
			return ApplyLight(*_lights, *_shadowVolumes, viewPt, LinearInterpolate(x, x0, x1, _p0, _p1), *_attr, _modelColor, n, _viewPoint, _ambient);
		else
			return ApplyLight(*_lights, *_shadowVolumes, viewPt, LinearInterpolate(x, x0, x1, _p1, _p0), *_attr, _modelColor, n, _viewPoint, _ambient);
	}

	COLORREF operator()(int x, int x0, int x1, const MixedIntPoint& viewPt, COLORREF c0, COLORREF c1, const Point3D& ospt, const Vector3D& n, bool swap)
	{
		if (!swap)
			return ApplyLight(*_lights, *_shadowVolumes, viewPt, ospt, *_attr, _modelColor, n, _viewPoint, _ambient);
		else
			return ApplyLight(*_lights, *_shadowVolumes, viewPt, ospt, *_attr, _modelColor, n, _viewPoint, _ambient);
	}


	Point3D _p0, _p1;
	const std::vector<LightSource>* _lights;
	const std::vector<ShadowVolume>* _shadowVolumes;
	const ModelAttr* _attr;
	COLORREF _modelColor;
	Point3D _viewPoint;
	double _ambient;
};

void DrawPolygon(DrawingObject& img, const Polygon3D& poly0, const MatrixHomogeneous& mTotal, const ModelAttr& attr, COLORREF objColor, bool objColorValid, const Normals::PolygonNormalData& nd, bool fillPolygons, const std::vector<LightSource>& lights, const std::vector<ShadowVolume>& svs, bool clip = false, const ClippingPlane& cp = ClippingPlane(0, 0, 0, 0))
{
	if (poly0.points.size() < 2)
	{
		return;
	}

	FakeXYMap xyMap;

	const int width = fillPolygons ? 1 : attr.line_width;

	Normals::PolygonNormalData clipND = clip ? Normals::PolygonNormalData(nd.PolygonNormal) : nd;
	const Polygon3D clipPoly = clip ? ApplyClipping(poly0, cp, nd ,clipND) : poly0;
	const Polygon3D poly = mTotal * clipPoly;

	COLORREF actualColor = 0;
	if (attr.Shading == SHADING_FLAT)
	{
		actualColor = GetActualColor(objColor, objColorValid, poly, HomogeneousPoint::Zeros, attr);
		actualColor = ApplyLight(lights, Point3D(nd.PolygonNormal.p0), attr, actualColor, Point3D(nd.PolygonNormal.p0) - Point3D(nd.PolygonNormal.p1), Point3D::Zero, attr.AmbientIntensity);
	}	

	MixedIntPoint p0, p1;
	bool getP0 = true;
	for (size_t i = 0; i < poly.points.size(); ++i)
	{
		if (attr.Shading != SHADING_FLAT)
		{
			actualColor = GetActualColor(objColor, objColorValid, poly, poly.points[i], attr);
		}
		const ColorFlat clrFlat(actualColor);

		const LineSegment l = ((i + 1) < poly.points.size()) ?
			LineSegment(poly.points[i], poly.points[i + 1]) :
			LineSegment(poly.points[i], poly.points.front());

		const LineSegment objSpaceLn = ((i + 1) < clipPoly.points.size()) ?
			LineSegment(clipPoly.points[i], clipPoly.points[i + 1]) :
			LineSegment(clipPoly.points[i], clipPoly.points.front());

		if (getP0)
		{
			p0 = MixedIntPoint(l.p0, objSpaceLn.p0);
			//objSpP0 = MixedIntPoint(objSpaceLn.p0);
			getP0 = false;
		}

		p1 = MixedIntPoint(l.p1, objSpaceLn.p1);

		if (attr.Shading == SHADING_GOURAUD || attr.Shading == SHADING_PHONG)
		{
			p0.normal = Vector3D(clipND.VertexNormals[i].p1) - Vector3D(clipND.VertexNormals[i].p0);
			p1.normal = ((i + 1) < clipND.VertexNormals.size()) ?
					(Vector3D(clipND.VertexNormals[i + 1].p1) - Vector3D(clipND.VertexNormals[i + 1].p0))
					:
					(Vector3D(clipND.VertexNormals.front().p1) - Vector3D(clipND.VertexNormals.front().p0));
		}

		ColorPhong cPhong = attr.Shading == SHADING_PHONG ? ColorPhong(objSpaceLn.p0, objSpaceLn.p1, lights, svs, attr, actualColor, Point3D::Zero, attr.AmbientIntensity) : ColorPhong();

		/*if (attr.line_width > 1)
		{
			innerDrawLine(img, p0, p1, actualColor, attr.line_width, img.active == DrawingObject::DRAWING_OBJECT_ZBUF);
		}*/

		if (p0.y != p1.y)
		{
			switch (attr.Shading)
			{
			case SHADING_FLAT:
				p0.color = p1.color = actualColor;
				if (img.active == DrawingObject::DRAWING_OBJECT_ZBUF)
					innerDrawLine(xyMap, p0, p1, i, width, LinearInterpolate<double>, clrFlat, NormalZero, NormalZero);
				else
					innerDrawLine(xyMap, p0, p1, i, width, ZZero, clrFlat, NormalZero, NormalZero);
				break;
			case SHADING_GOURAUD:
			{
				p0.color = ApplyLight(lights, svs, p0, Point3D(objSpaceLn.p0), attr, actualColor, p0.normal, Point3D::Zero, attr.AmbientIntensity);
				p1.color = ApplyLight(lights, svs, p1, Point3D(objSpaceLn.p1), attr, actualColor, p1.normal, Point3D::Zero, attr.AmbientIntensity);
				if (img.active == DrawingObject::DRAWING_OBJECT_ZBUF)
					innerDrawLine(xyMap, p0, p1, i, width, LinearInterpolate<double>, ColorInterpolateSV, NormalZero, NormalZero);
				else
					innerDrawLine(xyMap, p0, p1, i, width, ZZero, ColorInterpolateSV, NormalZero, NormalZero);
				break;
			}
			case SHADING_PHONG:
			{
				p0.color = ApplyLight(lights, svs, p0, Point3D(objSpaceLn.p0), attr, actualColor, p0.normal, Point3D::Zero, attr.AmbientIntensity);
				p1.color = ApplyLight(lights, svs, p1, Point3D(objSpaceLn.p1), attr, actualColor, p1.normal, Point3D::Zero, attr.AmbientIntensity);
				if (img.active == DrawingObject::DRAWING_OBJECT_ZBUF || img.active == DrawingObject::DRAWING_OBJECT_SV)
					innerDrawLine(xyMap, p0, p1, i, width, LinearInterpolate<double>, cPhong, LinearInterpolate<Vector3D>, LinearInterpolate<Point3D>);
				else
					innerDrawLine(xyMap, p0, p1, i, width, ZZero, cPhong, LinearInterpolate<Vector3D>, LinearInterpolate<Point3D>);
				break;
			}
			default:
				p0.color = p1.color = actualColor;
				if (img.active == DrawingObject::DRAWING_OBJECT_ZBUF || img.active == DrawingObject::DRAWING_OBJECT_SV)
					innerDrawLine(xyMap, p0, p1, i, width, LinearInterpolate<double>, clrFlat, NormalZero, NormalZero);
				else
					innerDrawLine(xyMap, p0, p1, i, width, ZZero, clrFlat, NormalZero, NormalZero);
				break;
			}

			for (auto pIt = xyMap.xyMap[p0.y].begin(); pIt != xyMap.xyMap[p0.y].end(); ++pIt)
			{
				if (pIt->x == p0.x && pIt->LineId == i)
				{
					if (p0.y < p1.y)
						pIt->Type = XData::UPPER_LIMIT;
					else
						pIt->Type = XData::LOWER_LIMIT;
					break;
				}
			}
			for (auto pIt = xyMap.xyMap[p1.y].begin(); pIt != xyMap.xyMap[p1.y].end(); ++pIt)
			{
				if (pIt->x == p1.x && pIt->LineId == i)
				{
					if (p1.y < p0.y)
						pIt->Type = XData::UPPER_LIMIT;
					else
						pIt->Type = XData::LOWER_LIMIT;
					break;
				}
			}
		}
		else
		{
			switch (attr.Shading)
			{
			case SHADING_FLAT:
				p0.color = p1.color = actualColor;
				break;
			case SHADING_GOURAUD:
			{
				p0.color = ApplyLight(lights, svs, p0, Point3D(objSpaceLn.p0), attr, actualColor, p0.normal, Point3D::Zero, attr.AmbientIntensity);
				p1.color = ApplyLight(lights, svs, p1, Point3D(objSpaceLn.p1), attr, actualColor, p1.normal, Point3D::Zero, attr.AmbientIntensity);
				break;
			}
			case SHADING_PHONG:
				p0.color = ApplyLight(lights, svs, p0, Point3D(objSpaceLn.p0), attr, actualColor, p0.normal, Point3D::Zero, attr.AmbientIntensity);
				p1.color = ApplyLight(lights, svs, p1, Point3D(objSpaceLn.p1), attr, actualColor, p1.normal, Point3D::Zero, attr.AmbientIntensity);
				break;
			default:
				p0.color = p1.color = actualColor;
				break;
			}
			
			if (fillPolygons)
			{
				if (p0.x < p1.x)
				{
					xyMap.SetPixel(p0.x, p0.y, p0.z, p0.color, i, p0.normal, p0.objSpacePt, XData::LEFT);
					xyMap.SetPixel(p1.x, p1.y, p1.z, p1.color, i, p1.normal, p0.objSpacePt, XData::RIGHT);
				}
				else
				{
					xyMap.SetPixel(p0.x, p0.y, p0.z, p0.color, i, p0.normal, p0.objSpacePt, XData::RIGHT);
					xyMap.SetPixel(p1.x, p1.y, p1.z, p1.color, i, p1.normal, p0.objSpacePt, XData::LEFT);
				}
			}
			else
			{
				if (img.active == DrawingObject::DRAWING_OBJECT_ZBUF || img.active == DrawingObject::DRAWING_OBJECT_SV)
					innerDrawLine(xyMap, p0, p1, i, width, LinearInterpolate<double>, clrFlat, NormalZero, NormalZero);
				else
					innerDrawLine(xyMap, p0, p1, i, width, ZZero, clrFlat, NormalZero, NormalZero);
			}
		}

		p0 = p1;
	}

	for (int y = xyMap.minY; y <= xyMap.maxY; ++y)
	{
		if (y < 0)
			continue;
		std::vector<XData> currRow = xyMap.xyMap[y];

		for (auto it = currRow.begin(); it != currRow.end(); ++it)
		{
			img.SetPixel(it->x, y, it->z, it->color);
		}
		if (!fillPolygons)
			continue;

		std::sort(currRow.begin(), currRow.end());
		std::vector<XData> origRow = currRow;

		// collapse sequences of pixels
		CollapseSequences(currRow);

		bool draw = false;
		bool lowerLeft = false, prevDraw;
		int idx = 0;
		std::vector<XData>::const_iterator interpolationIter0 = origRow.begin();
		for (int x = currRow.front().x; x != currRow.back().x; ++x)
		{
			if (x == currRow[idx].x)
			{
				int counters[5] = { 0, 0, 0, 0, 0 };

				while (idx < (currRow.size() - 1) && (currRow[idx].x == currRow[idx + 1].x))
				{
					++(counters[currRow[idx].Type]);
					++idx;
				}
				++(counters[currRow[idx].Type]);

				++idx;

				if (counters[XData::MIDDLE] > 0 && (counters[XData::UPPER_LIMIT] == 0 || counters[XData::LOWER_LIMIT] == 0))
				{
					draw = !draw;
				}
				else if (counters[XData::UPPER_LIMIT] > 0 && counters[XData::LOWER_LIMIT] > 0)
				{
					draw = !draw;
				}
				else if (counters[XData::LEFT] > 0)
				{
					prevDraw = draw;
					draw = true;
					lowerLeft = counters[XData::LOWER_LIMIT] > 0;
				}
				else if (counters[XData::RIGHT] > 0)
				{
					if (lowerLeft)
					{
						draw = prevDraw ^ (counters[XData::UPPER_LIMIT] > 0);
					}
					else
					{
						draw = prevDraw ^ (counters[XData::LOWER_LIMIT] > 0);
					}
					lowerLeft = false;
				}

			}

			if (draw && (BinarySearchInXDataVector(origRow, x) == origRow.end()))
			{
				COLORREF fillColor = GetActualColor(objColor, objColorValid, poly, HomogeneousPoint::Zeros, attr);
				bool needsIterpolation = (img.active == DrawingObject::DRAWING_OBJECT_ZBUF || img.active == DrawingObject::DRAWING_OBJECT_SV);
				switch (attr.Shading)
				{
				case SHADING_FLAT:
					fillColor = actualColor;
					break;
				case SHADING_GOURAUD:
					needsIterpolation = true;
					break;
				case SHADING_PHONG:
					needsIterpolation = true;
					break;
				default:
					break;
				}
				if (needsIterpolation)
				{
					while (x > (interpolationIter0 + 1)->x)
					{
						++interpolationIter0;
					}
					const int x0 = (interpolationIter0)->x, x1 = (interpolationIter0 + 1)->x;
					const double z0 = (interpolationIter0)->z, z1 = (interpolationIter0 + 1)->z;
					const double currZ = LinearInterpolate(x, x0, x1, z0, z1);
					if (attr.Shading == SHADING_GOURAUD)
					{
						const COLORREF c1 = interpolationIter0->color, c2 = (interpolationIter0 + 1)->color;
						fillColor = ColorInterpolate(x, x0, x1, c1, c2);
					}
					else if (attr.Shading == SHADING_PHONG)
					{
						const Vector3D n = LinearInterpolate(x, x0, x1, interpolationIter0->normal, (interpolationIter0 + 1)->normal);
						const Point3D currObjSpPt = LinearInterpolate(x, 0, x1, interpolationIter0->objSpacePt, (interpolationIter0 + 1)->objSpacePt);

						// v_texture
						//actualColor = RGB(abs(sin(x) * 255), abs(sin(x) * 255), abs(sin(x) * 255));
						fillColor = ApplyLight(lights, svs, MixedIntPoint(x, y, currZ), currObjSpPt, attr, actualColor, n, Point3D::Zero, attr.AmbientIntensity); /*FIX THIS*/
					}
					img.SetPixel(x, y, currZ, fillColor);
				}
				else
				{
					img.SetPixel(x, y, 0.0, fillColor);
				}
			}
		}
	}
}

void DrawObject(DrawingObject& img, const PolygonalObject& obj, const MatrixHomogeneous& mTotal, const ModelAttr& attr, const std::vector<Normals::PolygonNormalData>& normals, size_t normalsOffset, bool fillPolygons, bool clip, const ClippingPlane& cp)
{
	if (img.active == DrawingObject::DRAWING_OBJECT_ZBUF && img.zBufImg)
	{
		if (attr.forceOpacity)
			img.zBufImg->SetOpacity(attr.opacity);
		else if (obj.opacityValid)
			img.zBufImg->SetOpacity(obj.opacity);
	}
	for (size_t i = 0; i != obj.polygons.size(); ++i)
	{
		bool draw = false;
		if (attr.removeBackFace == BACKFACE_SHOW)
		{
			draw = true;
		}
		else
		{
			Normals::PolygonNormalData n = mTotal * normals[i + normalsOffset];;
			Vector3D normalDir = (Vector3D(n.PolygonNormal.p1) - Vector3D(n.PolygonNormal.p0));
			if ((attr.removeBackFace == BACKFACE_REMOVE_BACK && normalDir * Vector3D(0, 0, 1) < 0) ||
				(attr.removeBackFace == BACKFACE_REMOVE_FRONT && normalDir * Vector3D(0, 0, 1) > 0))
			{
				draw = true;
			}
		}
		if (draw)
		{
			DrawPolygon(img, obj.polygons[i], mTotal, attr, obj.color, obj.colorValid, normals[i + normalsOffset], fillPolygons, g_lights, g_ShadowVolumes, clip, cp);
		}
	}
	if (img.active == DrawingObject::DRAWING_OBJECT_ZBUF && img.zBufImg)
	{
		img.zBufImg->SetOpacity();
	}
}

COLORREF ApplyLight(const std::vector<LightSource>& lights, const Point3D& pt, const ModelAttr& attr, COLORREF clr, const Vector3D& normal, const Point3D& viewPoint, double ambient)
{
	return ApplyLight(lights, std::vector<ShadowVolume>(), MixedIntPoint(), pt, attr, clr, normal, viewPoint, ambient);
}

inline int mod(int a, int b) {
	int r = a % b;
	return r < 0 ? r + b : r;
}

const int nr_random = 200;
const int noise_margin = 0;
double noise[nr_random + noise_margin + 1][nr_random + noise_margin +1][nr_random + noise_margin + 1]; // in [0,1)
bool inited = false;

double noise_at_xy(const Point3D& pt1, const BoundingBox& bbox1) {

	if (!inited) {
		for (int i = 0; i < nr_random + noise_margin + 1; i++) {
			for (int j = 0; j < nr_random + noise_margin + 1; j++) {
				for (int k = 0; k < nr_random + noise_margin + 1; k++) {
					noise[i][j][k] = (rand() % 32768) / 32768.0;
					//noise[i][j][k] = (rand() % 100) / 100.0;
				}
			}
		}
		inited = true;
	}
	double diffs[3];
	diffs[0] = (bbox1.maxX - bbox1.minX) / (double)nr_random;
	diffs[1] = (bbox1.maxY - bbox1.minY) / (double)nr_random;
	diffs[2] = (bbox1.maxZ - bbox1.minZ) / (double)nr_random;

	int nearestXidx = noise_margin / 2 + int((pt1.x) / diffs[0]);// / nr_random;
	int nearestYidx = noise_margin / 2 + int((pt1.y) / diffs[1]);// / nr_random;
	int nearestZidx = noise_margin / 2 + int((pt1.z) / diffs[2]);// / nr_random;
	
	//assert(nearestXidx <= nr_random);
	//assert(nearestXidx >= 0);

	nearestXidx = mod(nearestXidx, nr_random + noise_margin);
	nearestYidx = mod(nearestYidx, nr_random + noise_margin);
	nearestZidx = mod(nearestZidx, nr_random + noise_margin);

	//if (nearestXidx < 0) nearestXidx = 0;
	//if (nearestYidx < 0) nearestYidx = 0;
	//if (nearestZidx < 0) nearestZidx = 0;

	/*double w1x = fabs(pt1.x - (nearestXidx * diffs[0] + (double)bbox1.minX));
	double w2x = fabs(pt1.x - ((nearestXidx + 1) * (diffs[0]) + (double)bbox1.minX));

	double w1y = fabs(pt1.y - (nearestYidx * (diffs[1]) + (double)bbox1.minY));
	double w2y = fabs(pt1.y - ((nearestYidx + 1) * (diffs[1]) + (double)bbox1.minY));

	double w1z = fabs(pt1.z - (nearestZidx * (diffs[2]) + (double)bbox1.minZ));
	double w2z = fabs(pt1.z - ((nearestZidx + 1) * (diffs[2]) + (double)bbox1.minZ));*/

	double w1x = fabs(pt1.x / diffs[0] - nearestXidx + noise_margin/2);
	double w2x = 1 - w1x;
	//double w2x = fabs(pt1.x - ((nearestXidx + 1) * (diffs[0])));

	double w1y = fabs(pt1.y / diffs[1] - nearestYidx + noise_margin / 2);
	double w2y = 1 - w1y;
	//double w2y = fabs(pt1.y - ((nearestYidx + 1) * (diffs[1])));

	double w1z = fabs(pt1.z / diffs[2] - nearestZidx + noise_margin / 2);
	double w2z = 1 - w1z;
	//double w2z = fabs(pt1.z - ((nearestZidx + 1) * (diffs[2])));

	double local_noise = 0;
	
	local_noise += ((double)noise[nearestXidx][nearestYidx][nearestZidx]) * w1x * w1y * w1z;
	local_noise += ((double)noise[nearestXidx][nearestYidx][nearestZidx + 1]) * w1x * w1y * w2z;
	local_noise += ((double)noise[nearestXidx][nearestYidx + 1][nearestZidx]) * w1x * w2y * w1z;
	local_noise += ((double)noise[nearestXidx][nearestYidx + 1][nearestZidx + 1]) * w1x * w2y * w2z;
	local_noise += ((double)noise[nearestXidx + 1][nearestYidx][nearestZidx]) * w2x * w1y * w1z;
	local_noise += ((double)noise[nearestXidx + 1][nearestYidx][nearestZidx + 1]) * w2x * w1y * w2z;
	local_noise += ((double)noise[nearestXidx + 1][nearestYidx + 1][nearestZidx]) * w2x * w2y * w1z;
	local_noise += ((double)noise[nearestXidx + 1][nearestYidx + 1][nearestZidx + 1]) * w2x * w2y * w2z;

	/*local_noise += ((double)noise[nearestXidx][nearestYidx][nearestZidx] * sqrt(w1x*w1x + w1y*w1y + w1z*w1z));
	local_noise += ((double)noise[nearestXidx][nearestYidx][nearestZidx + 1] * sqrt(w1x*w1x + w1y*w1y + (w2z)*(w2z)));
	local_noise += ((double)noise[nearestXidx][nearestYidx + 1][nearestZidx] * sqrt(w1x*w1x + (w2y)*(w2y) + w1z*w1z));
	local_noise += ((double)noise[nearestXidx + 1][nearestYidx][nearestZidx] * sqrt((w2x)*(w2x) + w1y*w1y + w1z*w1z));
	local_noise += ((double)noise[nearestXidx + 1][nearestYidx + 1][nearestZidx] * sqrt((w2x)*(w2x) + (w2y)*(w2y) + w1z*w1z));
	local_noise += ((double)noise[nearestXidx][nearestYidx + 1][nearestZidx + 1] * sqrt(w1x*w1x + (w2y)*(w2y) + (w2z)*(w2z)));
	local_noise += ((double)noise[nearestXidx + 1][nearestYidx][nearestZidx + 1] * sqrt((w2x)*(w2x) + w1y*w1y + (w2z)*(w2z)));
	local_noise += ((double)noise[nearestXidx + 1][nearestYidx + 1][nearestZidx + 1] * sqrt((w2x)*(w2x) + (w2y)*(w2y) + (w2z)*(w2z)));

	local_noise /= sqrt(w1x*w1x + w1y*w1y + w1z*w1z)
		+ sqrt(w1x*w1x + w1y*w1y + (w2z)*(w2z))
		+ sqrt(w1x*w1x + (w2y)*(w2y) + w1z*w1z)
		+ sqrt((w2x)*(w2x) + w1y*w1y + w1z*w1z)
		+ sqrt((w2x)*(w2x) + (w2y)*(w2y) + w1z*w1z)
		+ sqrt(w1x*w1x + (w2y)*(w2y) + (w2z)*(w2z))
		+ sqrt((w2x)*(w2x) + w1y*w1y + (w2z)*(w2z))
		+ sqrt((w2x)*(w2x) + (w2y)*(w2y) + (w2z)*(w2z));*/


	/*
	
	local_noise += ((double)noise[nearestXidx][nearestYidx][nearestZidx] * sqrt(w1x*w1x + w1y*w1y + w1z*w1z));
	local_noise += ((double)noise[nearestXidx][nearestYidx][nearestZidx + 1] * sqrt(w1x*w1x + w1y*w1y + (w1z + 1)*(w1z + 1)));
	local_noise += ((double)noise[nearestXidx][nearestYidx + 1][nearestZidx] * sqrt(w1x*w1x + (w1y + 1)*(w1y + 1) + w1z*w1z));
	local_noise += ((double)noise[nearestXidx + 1][nearestYidx][nearestZidx] * sqrt((w1x + 1)*(w1x + 1) + w1y*w1y + w1z*w1z));
	local_noise += ((double)noise[nearestXidx + 1][nearestYidx + 1][nearestZidx] * sqrt((w1x + 1)*(w1x + 1) + (w1y + 1)*(w1y + 1) + w1z*w1z));
	local_noise += ((double)noise[nearestXidx][nearestYidx + 1][nearestZidx + 1] * sqrt(w1x*w1x + (w1y + 1)*(w1y + 1) + (w1z + 1)*(w1z + 1)));
	local_noise += ((double)noise[nearestXidx + 1][nearestYidx][nearestZidx + 1] * sqrt((w1x + 1)*(w1x + 1) + w1y*w1y + (w1z + 1)*(w1z + 1)));
	local_noise += ((double)noise[nearestXidx + 1][nearestYidx + 1][nearestZidx + 1] * sqrt((w1x + 1)*(w1x + 1) + (w1y + 1)*(w1y + 1) + (w1z + 1)*(w1z + 1)));

	local_noise /= sqrt(w1x*w1x + w1y*w1y + w1z*w1z)
		+ sqrt(w1x*w1x + w1y*w1y + (w1z + 1)*(w1z + 1))
		+ sqrt(w1x*w1x + (w1y + 1)*(w1y + 1) + w1z*w1z)
		+ sqrt((w1x + 1)*(w1x + 1) + w1y*w1y + w1z*w1z)
		+ sqrt((w1x + 1)*(w1x + 1) + (w1y + 1)*(w1y + 1) + w1z*w1z)
		+ sqrt(w1x*w1x + (w1y + 1)*(w1y + 1) + (w1z + 1)*(w1z + 1))
		+ sqrt((w1x + 1)*(w1x + 1) + w1y*w1y + (w1z + 1)*(w1z + 1))
		+ sqrt((w1x + 1)*(w1x + 1) + (w1y + 1)*(w1y + 1) + (w1z + 1)*(w1z + 1));

	*/

	return local_noise;
}

double turbulance(const Point3D& pt, const BoundingBox& bbox, double size) {
	double value = 0;
	double initialSize = size;

	while (size >= 1) {
		Point3D pt2 = pt / size;
		 value += noise_at_xy(pt2, bbox) * size;
		//value += ((rand() % 32768) / 32768.0) * size;
		size /= 2;
	}

	return (128.0 * value) / initialSize;
}

double smoothNoise(double x, double y, double z)
{
	if (!inited) {
		for (int i = 0; i < nr_random + noise_margin + 1; i++) {
			for (int j = 0; j < nr_random + noise_margin + 1; j++) {
				for (int k = 0; k < nr_random + noise_margin + 1; k++) {
					noise[i][j][k] = (rand() % 32768) / 32768.0;
					//noise[i][j][k] = (rand() % 100) / 100.0;
				}
			}
		}
		inited = true;
	}
	//get fractional part of x and y
	double fractX = x - int(x);
	double fractY = y - int(y);
	double fractZ = z - int(z);

	//wrap around
	int x1 = (int(x) + nr_random) % nr_random;
	int y1 = (int(y) + nr_random) % nr_random;
	int z1 = (int(z) + nr_random) % nr_random;

	//neighbor values
	int x2 = (x1 + nr_random - 1) % nr_random;
	int y2 = (y1 + nr_random - 1) % nr_random;
	int z2 = (z1 + nr_random - 1) % nr_random;

	//smooth the noise with bilinear interpolation
	double value = 0.0;
	value += fractX     * fractY     * fractZ     * noise[z1][y1][x1];
	value += fractX     * (1 - fractY) * fractZ     * noise[z1][y2][x1];
	value += (1 - fractX) * fractY     * fractZ     * noise[z1][y1][x2];
	value += (1 - fractX) * (1 - fractY) * fractZ     * noise[z1][y2][x2];

	value += fractX     * fractY     * (1 - fractZ) * noise[z2][y1][x1];
	value += fractX     * (1 - fractY) * (1 - fractZ) * noise[z2][y2][x1];
	value += (1 - fractX) * fractY     * (1 - fractZ) * noise[z2][y1][x2];
	value += (1 - fractX) * (1 - fractY) * (1 - fractZ) * noise[z2][y2][x2];

	return value;
}

double turbulance2(const Point3D& pt, double size) {
	double value = 0;
	double initialSize = size;

	while (size >= 1) {
		Point3D pt2 = pt / size;
		value += smoothNoise(pt2.x, pt2.y, pt.z) * size;
		//value += ((rand() % 32768) / 32768.0) * size;
		size /= 2;
	}

	return (128.0 * value) / initialSize;
}

COLORREF ApplyLight(const std::vector<LightSource>& lights, const std::vector<ShadowVolume>& svs, const MixedIntPoint& viewPt, const Point3D& pt, const ModelAttr& attr, COLORREF clr, const Vector3D& normal, const Point3D& viewPoint, double ambient)
{
	//const Point3D pt(pixel.x, pixel.y, pixel.z);

	double normACoefficient = attr.AmbientCoefficient / (attr.AmbientCoefficient + attr.DiffuseCoefficient + attr.SpecularCoefficient);
	double normDCoefficient = attr.DiffuseCoefficient / (attr.AmbientCoefficient + attr.DiffuseCoefficient + attr.SpecularCoefficient);
	double normSCoefficient = attr.SpecularCoefficient / (attr.AmbientCoefficient + attr.DiffuseCoefficient + attr.SpecularCoefficient);

	//double rgbValues[] = { (double)(GetRValue(clr)), (double)(GetGValue(clr)), (double)(GetBValue(clr)) };
	// v_texture

	Point3D pt1(attr.inv * HomogeneousPoint(pt));
	BoundingBox bbox1 = attr.GetBoundingBox();

	Point3D ptOffset(pt1.x - bbox1.minX, pt1.y - bbox1.minY, pt1.z - bbox1.minZ);

	double local_noise = 0;// noise_at_xy(pt1, bbox1);

	double diffs[3];
	diffs[0] = (bbox1.maxX - bbox1.minX) / (double)nr_random;
	diffs[1] = (bbox1.maxY - bbox1.minY) / (double)nr_random;
	diffs[2] = (bbox1.maxZ - bbox1.minZ) / (double)nr_random;

	Point3D pt01(100.0 * ptOffset.x / (bbox1.maxX - bbox1.minX), 100.0 * ptOffset.y / (bbox1.maxY - bbox1.minY), 100.0 * ptOffset.z / (bbox1.maxZ - bbox1.minZ));
	//Point3D pt01(100 * ptOffset.x , 100 * ptOffset.y , 100* ptOffset.z );


	const double pi = 3.141592;
	//local_noise = (local_noise / 5.0);

	double a = 0.001, turbPower = 2;
	//double turbulance1 = turbPower * turbulance2(pt01, 8) / 256.0;
	double turbulance1 = turbPower * turbulance2(pt01, 8) / 256.0;
	/*double xyValue = (ptOffset.x) * a / (bbox1.maxX - bbox1.minX)
		//+ (ptOffset.y) * a / (bbox1.maxY - bbox1.minY)
		//+ (ptOffset.z) * a / (bbox1.maxZ - bbox1.minZ)
		+ turbulance1;*/
	double xyValue =a* sin(             ((ptOffset.x)  / (bbox1.maxX - bbox1.minX)) * 2 * pi        )
		+ a*sin (        (ptOffset.y) / (bbox1.maxY - bbox1.minY)      * 2 * pi)
		+ a*sin (        (ptOffset.z) / (bbox1.maxZ - bbox1.minZ)      * 2 * pi)
		+ turbulance1;


	//double num = abs(sin((pt1.x + pt1.y + pt1.z) / 3 * 100)) * 255;
	double num = abs(sin(xyValue * pi));

	//num = abs(turbulance1) * 255;

	//num *= ((local_noise_x + local_noise_y + local_noise_z) / 3.0);
	//num = ((local_noise_x + local_noise_y + local_noise_z) / 3.0) * 255.0;
	//num = (local_noise) * 255.0;
	//num *= local_noise;

	//num = noise_at_xy(pt1, bbox1) * 256;

	//num = smoothNoise(pt01.x, pt01.y, pt01.z) * 256.0;
	//num = ptOffset.x / (bbox1.maxX - bbox1.minX);

	double rgbValues[3];// = { num, num, num };
	rgbValues[0] = num * 255;
	rgbValues[1] = num * 255;// num;
	rgbValues[2] = num * 255;// num;
	//rgbValues[0] += ((int)((double)rand() * 0.005));
	//rgbValues[1] += ((int)((double)rand() * 0.005));
	//rgbValues[2] += ((int)((double)rand() * 0.005));

	//rgbValues[0] = GetRValue(clr);
	//rgbValues[1] = GetGValue(clr);
	//rgbValues[2] = GetBValue(clr);

	const Vector3D n = normal.Normalized();
	const Vector3D viewVec = (viewPoint - pt).Normalized();

	double currLightPart[] = { 0, 0, 0 };
	for (size_t j = 0; j < lights.size(); ++j)
	{
		if (svs.size() > j)
		{
			if (!svs[j].IsPixelLit(viewPt.x, viewPt.y, viewPt.z, pt))
				continue;
		}
		const Vector3D lightVec = lights[j]._type == LightSource::PLANE ?
			lights[j].Direction()
			:
			(pt - Point3D(lights[j]._origin)).Normalized();

		if (lights[j]._type == LightSource::SPOT)
		{
			double dotFromSpotAxis = lightVec * lights[j].Direction();
			if (lights[j]._minDot > dotFromSpotAxis)
				continue;
		}

		const Vector3D reflectVec = lightVec - 2 * (lightVec*n)*n;
		double viewProd = fabs(-(reflectVec * viewVec));
		double dotProd = -(n * lightVec);
		if (dotProd < 0)
			dotProd = 0;

		for (int i = 0; i < 3; ++i)
		{
			currLightPart[i] += (lights[j]._intensity[i] * (normDCoefficient*dotProd + normSCoefficient*pow(viewProd, attr.SpecularPower)));
		}
	}

	for (int i = 0; i < 3; ++i)
	{
		rgbValues[i] = rgbValues[i] * (ambient*normACoefficient + currLightPart[i]);
		rgbValues[i] = min(rgbValues[i], 255.0);
	}

	int rgbInts[] = { rgbValues[0], rgbValues[1], rgbValues[2] };

	return RGB(rgbInts[0], rgbInts[1], rgbInts[2]);
}
