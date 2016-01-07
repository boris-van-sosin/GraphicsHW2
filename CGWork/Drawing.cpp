#include "Drawing.h"
#include "Utils.h"

#include <map>
#include <algorithm>

const COLORREF DefaultModelColor(RGB(0, 0, 255));

ClippingPlane::ClippingPlane(double x_, double y_, double z_, double c_)
	: x(x_), y(y_), z(z_), c(c_)
{
}

ClippingPlane::ClippingPlane(const Point3D& p, const Vector3D& n)
	: x(n.x), y(n.y), z(n.z), c(-(p * n))
{
}

ClippingPlane::ClippingPlane(const ClippingPlane& other)
	: ClippingPlane(other.x, other.y, other.z, other.c)
{
}

double ClippingPlane::Apply(const Point3D& p) const
{
	return Apply(p.x, p.y, p.z);
}

double ClippingPlane::Apply(const HomogeneousPoint& p) const
{
	return Apply(Point3D(p));
}

double ClippingPlane::Apply(double x_, double y_, double z_) const
{
	return x_ * x + y_ * y + z_ * z + c;
}

Point3D ClippingPlane::Intersection(const Point3D& p0, const Point3D& p1) const
{
	Point3D pb;
	if (z != 0)
	{
		pb = Point3D(0, 0, -c/z);
	}
	else if (y != 0)
	{
		pb = Point3D(0, 0, -c/y);
	}
	else if (x != 0)
	{
		pb = Point3D(0, 0, -c/x);
	}

	const Point3D normal(x, y, z);
	const double a = (p1 - p0)*normal;
	if (fabs(a) < GEOMETRIC_COMPUTATION_EPSILON)
	{
		return Point3D(NAN, NAN, NAN);
	}
	double d = ((pb - p0) * normal) / a;
	return (d * (p1 - p0)) + p0;
}

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
	_img[y*_width + x].PushColor(clr, z);
}

void ZBufferImage::PushPixel(int x, int y, const Point3D& p0, const Point3D& p1, COLORREF clr)
{
	const double relativeT = ((double)x - p0.x) / (p1.x - p0.x);
	const double z = p0.z + (p1.z - p0.z)*relativeT;

	_img[y*_width + x].PushColor(clr, z);
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
			const COLORREF clr = _img[y*_width + x].GetActualColor();
			*pos = GetBValue(clr);
			*(pos + 1) = GetGValue(clr);
			*(pos + 2) = GetRValue(clr);
		}
	}
}

ZBufferPixel::ZBufferItem::ZBufferItem(COLORREF clr, double z_, double op)
	: z(z_), color(clr), opacity(op)
{
}

void ZBufferPixel::PushColor(COLORREF clr, double z)
{
	_buffer.insert(ZBufferItem(clr, z));
}

void ZBufferPixel::Clear()
{
	_buffer.clear();
}

bool ZBufferPixel::IsEmpty() const
{
	return _buffer.empty();
}

COLORREF ZBufferPixel::GetActualColor() const
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
		if (it->opacity >= 1.0)
		{
			break;
		}
		prevOpacity = it->opacity;
	}

	int finalRed = (int)red;
	int finalGreen = (int)green;
	int finalBlue = (int)blue;

	return RGB(finalRed, finalGreen, finalBlue);
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
	else if (zBufImg)
		return zBufImg->GetHeight();
	else
		return 0;
}

int DrawingObject::GetWidth() const
{
	if (active == DRAWING_OBJECT_CIMG && img)
		return img->GetWidth();
	else if (zBufImg)
		return zBufImg->GetWidth();
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
	else if (zBufImg)
		zBufImg->PushPixel(x, y, z, clr);
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

template <typename T>
void CGSwap(T& x, T& y) {
	T  t = x;
	x = y;
	y = t;
}

template<typename T>
T LinearInterpolate(double x, double minX, double maxX, const T& t0, const T& t1)
{
	double b = (x - minX) / (maxX - minX);
	return (1 - b)*t0 + b*t1;
}

COLORREF ColorInterpolate(double x, double minX, double maxX, COLORREF clr0, COLORREF clr1)
{
	double floatColors[] = { GetRValue(clr0), GetGValue(clr0), GetBValue(clr0), GetRValue(clr1), GetGValue(clr1), GetBValue(clr1) };
	int newColors[3];
	for (int i = 0; i < 3; ++i)
	{
		newColors[i] = LinearInterpolate(x, minX, maxX, floatColors[i], floatColors[i + 3]);
	}
	return RGB(newColors[0], newColors[1], newColors[3]);
}

struct XData
{
	enum PixelType { MIDDLE, UPPER_LIMIT, LOWER_LIMIT };
	XData(int x_, double z_, int lid, COLORREF clr, const Vector3D& n, PixelType pt = MIDDLE)
		: x(x_), z(z_), LineId(lid), Type(pt), color(clr), normal(n)
	{}
	XData(int x_, double z_, int lid, PixelType pt = MIDDLE)
		: x(x_), z(z_), LineId(lid), Type(pt)
	{}

	int x;
	int LineId;
	PixelType Type;
	double z;
	COLORREF color;
	Vector3D normal;
};

bool operator<(const XData& a, const XData& b)
{
	if (a.x != b.x)
		return a.x < b.x;
	else
		return a.LineId < b.LineId;
}

struct MixedIntPoint
{
	MixedIntPoint(int x_, int y_, double z_, COLORREF clr, const Vector3D& n)
		: x(x_), y(y_), z(z_), color(clr), normal(n)
	{}

	MixedIntPoint(int x_, int y_, double z_)
		: MixedIntPoint(x_, y_, z_, 0, Vector3D())
	{}

	MixedIntPoint()
		: x(0), y(0), z(0)
	{}
	MixedIntPoint(int x_, int y_)
		: MixedIntPoint(x, y, 0.0)
	{}

	MixedIntPoint(const MixedIntPoint& other)
		: MixedIntPoint(other.x, other.y, other.z, other.color, other.normal)
	{}

	MixedIntPoint(const Point3D& p)
		: MixedIntPoint(p.x, p.y, p.z)
	{
		if (p.colorValid)
			color = p.color;
	}

	MixedIntPoint(const HomogeneousPoint& p)
		: MixedIntPoint(Point3D(p))
	{}

	MixedIntPoint(const Point3D& p, COLORREF clr, const Vector3D& n)
		: MixedIntPoint(p.x, p.y, p.z, clr, n)
	{}

	MixedIntPoint(const HomogeneousPoint& p, COLORREF clr, const Vector3D& n)
		: MixedIntPoint(Point3D(p), clr, n)
	{}

	int x, y;
	double z;
	COLORREF color;
	Vector3D normal;
};

struct FakeXYMap
{
	void SetPixel(int x, int y, double z, COLORREF clr, int lineId, const Vector3D& n)
	{
		minY = min(minY, y);
		maxY = max(maxY, y);
		if (xyMap.find(y) == xyMap.end())
		{
			xyMap[y] = std::vector<XData>();
		}
		xyMap[y].push_back(XData(x, z, lineId, clr, n));
	}

	void SetPixel(int x, int y, double z, COLORREF clr)
	{
		SetPixel(x, y, z, 0, clr, Vector3D());
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

template <typename Z_FUNC, typename CLR_FUNC, typename N_FUNC>
void innerDrawLine(FakeXYMap& img, const MixedIntPoint& p0, const MixedIntPoint& p1, int lineId, unsigned int line_width, Z_FUNC zf, CLR_FUNC clrf, N_FUNC nf) {
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
	
	const MixedIntPoint newP0(x0, y0, z0, p0.color, p0.normal);
	const MixedIntPoint newP1(x1, y1, z1, p1.color, p1.normal);

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
				img.SetPixel(x0, y0, newP0.z, newP0.color, lineId, newP0.normal);
				for (unsigned int i = 1; i < line_width; i++) {
					img.SetPixel(x0, y0 + i, newP0.z, newP0.color, lineId, newP0.normal);
					img.SetPixel(x0, y0 - i, newP0.z, newP0.color, lineId, newP0.normal);
				}
			}
			else
			{
				img.SetPixel(y0, x0, newP0.z, newP0.color, lineId, newP0.normal);
				for (unsigned int i = 1; i < line_width; i++) {
					img.SetPixel(y0 + i, x0, newP0.z, newP0.color, lineId, newP0.normal);
					img.SetPixel(y0 - i, x0, newP0.z, newP0.color, lineId, newP0.normal);
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
			const double currZ = swapXY ? zf(y, y0, y1, z0, z1) : zf(x, x0, x1, z0, z1);
			const COLORREF currClr = swapXY ? clrf(y, y0, y1, p0.color, p1.color) : clrf(x, x0, x1, p0.color, p1.color);
			const Vector3D currN = swapXY ? nf(y, y0, y1, p0.normal, p1.normal) : nf(x, x0, x1, p0.normal, p1.normal);
			if (!swapXY)
			{
				img.SetPixel(x, y, currZ, currClr, lineId, currN);
				for (unsigned int i = 1; i < line_width; i++) {
					img.SetPixel(x, y + 1, currZ, currClr, lineId, currN);
					img.SetPixel(x, y - 1, currZ, currClr, lineId, currN);
				}
			}
			else
			{
				img.SetPixel(y, x, currZ, currClr, lineId, currN);
				for (unsigned int i = 1; i < line_width; i++) {
					img.SetPixel(y + i, x, currZ, currClr, lineId, currN);
					img.SetPixel(y - i, x, currZ, currClr, lineId, currN);
				}
			}
		}
	}
	else if (dx == 0 && dy == 0)
	{
		img.SetPixel(x0, y0, z0, p0.color, lineId, p0.normal);
		for (unsigned int i = 1; i < line_width; i++) {
			img.SetPixel(x0, y0 + i, z0, p0.color, lineId, p0.normal);
			img.SetPixel(x0, y0 - i, z0, p0.color, lineId, p0.normal);
			img.SetPixel(x0 + i, y0, z0, p0.color, lineId, p0.normal);
			img.SetPixel(x0 - i, y0, z0, p0.color, lineId, p0.normal);
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

Polygon3D ApplyClipping(const Polygon3D& poly, const ClippingPlane& cp)
{
	std::vector<HomogeneousPoint> resPoints;
	resPoints.reserve(poly.points.size());
	for (auto i = poly.points.begin(); i != poly.points.end(); ++i)
	{
		const HomogeneousPoint p0 = *i;
		const HomogeneousPoint p1 = (i + 1 != poly.points.end()) ? *(i + 1) : poly.points.front();
		const ClippingResult clipRes = ApplyClipping(p0, p1, cp);
		if ((!clipRes.clippedFirst) && (!clipRes.clippedSecond))
		{
			resPoints.push_back(clipRes.lineSegment.p0);
		}
		else if (clipRes.clippedFirst ^ clipRes.clippedSecond)
		{
			resPoints.push_back(clipRes.lineSegment.p0);
			resPoints.push_back(clipRes.lineSegment.p1);
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

COLORREF ApplyLight(const std::vector<LightSource>& lights, const MixedIntPoint& pixel, const ModelAttr& attr, COLORREF clr, const Vector3D& normal, const Point3D& viewPoint, double ambient);

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

	COLORREF operator()(int x, int x0, int x1, COLORREF c0, COLORREF c1)
	{
		return clr;
	}

	COLORREF clr;
};

Vector3D NormalZero(int x, int x0, int x1, const Vector3D& v0, const Vector3D& v1)
{
	return Point3D::Zero;
}

void DrawPolygon(DrawingObject& img, const Polygon3D& poly0, const MatrixHomogeneous& mTotal, const ModelAttr& attr, COLORREF objColor, bool objColorValid, const Normals::PolygonNormalData& nd, bool fillPolygons, const std::vector<LightSource>& lights, bool clip = false, const ClippingPlane& cp = ClippingPlane(0, 0, 0, 0))
{
	if (poly0.points.size() < 2)
	{
		return;
	}

	FakeXYMap xyMap;

	const Polygon3D clipPoly = clip ? ApplyClipping(poly0, cp) : poly0;
	const Polygon3D poly = mTotal * clipPoly;

	COLORREF actualColor = 0;
	if (attr.Shading == SHADING_FLAT)
	{
		actualColor = GetActualColor(objColor, objColorValid, poly, HomogeneousPoint::Zeros, attr);
		actualColor = ApplyLight(lights, nd.PolygonNormal.p0, attr, actualColor, Point3D(nd.PolygonNormal.p0) - Point3D(nd.PolygonNormal.p1), Point3D::Zero, 1);
	}	

	MixedIntPoint p0, p1, objSpP0, objSpP1;
	bool getP0 = true;
	for (size_t i = 0; i < poly.points.size(); ++i)
	{
		if (attr.Shading == SHADING_NONE)
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
			p0 = MixedIntPoint(l.p0);
			objSpP0 = MixedIntPoint(objSpaceLn.p0);
			getP0 = false;
		}

		p1 = MixedIntPoint(l.p1);
		objSpP1 = MixedIntPoint(objSpaceLn.p1);

		if (attr.Shading == SHADING_GOURAUD || attr.Shading == SHADING_PHONG)
		{
			objSpP0.normal = Vector3D(nd.VertexNormals[i].p0) - Vector3D(nd.VertexNormals[i].p1);
			objSpP1.normal = ((i + 1) < clipPoly.points.size()) ?
					(Vector3D(nd.VertexNormals[i+1].p0) - Vector3D(nd.VertexNormals[i+1].p1))
					:
					(Vector3D(nd.VertexNormals.back().p0) - Vector3D(nd.VertexNormals.back().p1));
		}

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
					innerDrawLine(xyMap, p0, p1, i, 1, LinearInterpolate<double>, clrFlat, NormalZero);
				else
					innerDrawLine(xyMap, p0, p1, i, 1, ZZero, clrFlat, NormalZero);
				break;
			case SHADING_GOURAUD:
			{
				p0.color = ApplyLight(lights, objSpP0, attr, actualColor, objSpP0.normal, Point3D::Zero, 1);
				p1.color = ApplyLight(lights, objSpP1, attr, actualColor, objSpP1.normal, Point3D::Zero, 1);
				if (img.active == DrawingObject::DRAWING_OBJECT_ZBUF)
					innerDrawLine(xyMap, p0, p1, i, 1, LinearInterpolate<double>, ColorInterpolate, NormalZero);
				else
					innerDrawLine(xyMap, p0, p1, i, 1, ZZero, ColorInterpolate, NormalZero);
				break;
			}
			case SHADING_PHONG:
				innerDrawLine(xyMap, p0, p1, i, 1, img.active == DrawingObject::DRAWING_OBJECT_ZBUF);
				break;
			default:
				if (img.active == DrawingObject::DRAWING_OBJECT_ZBUF)
					innerDrawLine(xyMap, p0, p1, i, 1, LinearInterpolate<double>, clrFlat, NormalZero);
				else
					innerDrawLine(xyMap, p0, p1, i, 1, ZZero, clrFlat, NormalZero);
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
			xyMap.SetPixel(p0.x, p0.y, p0.z, objSpP0.color, i, objSpP0.normal);
			xyMap.SetPixel(p1.x, p1.y, p1.z, objSpP0.color, i, objSpP0.normal);
			for (auto pIt = xyMap.xyMap[p0.y].begin(); pIt != xyMap.xyMap[p0.y].end(); ++pIt)
			{
				if (pIt->LineId == i)
					if (pIt->x == p0.x)
					{
					if (p0.x < p1.x)
						pIt->Type = XData::UPPER_LIMIT;
					else
						pIt->Type = XData::LOWER_LIMIT;
					}
					else if (pIt->x == p0.x)
					{
						if (p0.x < p1.x)
							pIt->Type = XData::LOWER_LIMIT;
						else
							pIt->Type = XData::UPPER_LIMIT;
					}
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
		int idx = 0;
		std::vector<XData>::const_iterator interpolationIter0 = origRow.begin();
		for (int x = currRow.front().x; x != currRow.back().x; ++x)
		{
			if (x == currRow[idx].x)
			{
				int counters[3] = { 0, 0, 0 };

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

			}

			if (draw && (BinarySearchInXDataVector(origRow, x) == origRow.end()))
			{
				COLORREF fillColor = GetActualColor(objColor, objColorValid, poly, HomogeneousPoint::Zeros, attr);
				bool needsIterpolation = img.active == DrawingObject::DRAWING_OBJECT_ZBUF;
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

void DrawObject(DrawingObject& img, const PolygonalObject& obj, const MatrixHomogeneous& mTotal, const ModelAttr& attr, const std::vector<Normals::PolygonNormalData>& normals, bool fillPolygons, bool clip, const ClippingPlane& cp)
{
	std::vector<LightSource> lights;
	lights.push_back(LightSource(Point3D(0.5, 0.5, 0.5), Point3D(0, 0, 1), LightSource::POINT, 1.0));
	for (size_t i = 0; i != obj.polygons.size(); ++i)
	{
		bool draw = false;
		if (!attr.removeBackFace)
		{
			draw = true;
		}
		else
		{
			Normals::PolygonNormalData n = mTotal * normals[i];
			Vector3D normalDir = (Vector3D(n.PolygonNormal.p1) - Vector3D(n.PolygonNormal.p0));
			if (normalDir * Vector3D(0, 0, 1) > 0)
			{
				draw = true;
			}
		}
		if (draw)
		{
			DrawPolygon(img, obj.polygons[i], mTotal, attr, obj.color, obj.colorValid, normals[i], fillPolygons, lights, clip, cp);
		}
	}
}

LightSource::LightSource()
	: LightSource(HomogeneousPoint::Zeros, HomogeneousPoint::Zeros, LightSource::POINT, 0.0)
{
}

LightSource::LightSource(const Point3D& or, const Vector3D& dir, LightSourceType t, double p)
	: LightSource(HomogeneousPoint(or), HomogeneousPoint(dir), t, p)
{
}

LightSource::LightSource(const HomogeneousPoint& or, const HomogeneousPoint& dir, LightSourceType t, double p)
	: _origin(or), _offset(Point3D(or) + (Point3D(dir).Normalized())), _type(t), _intensity(p)
{
}

LightSource::LightSource(const LightSource& other)
	: _origin(other._origin), _offset(other._offset), _type(other._type), _intensity(other._intensity)
{
}

Vector3D LightSource::Direction() const
{
	return (Vector3D(_origin) - Vector3D(_offset)).Normalized();
}

COLORREF ApplyLight(const std::vector<LightSource>& lights, const MixedIntPoint& pixel, const ModelAttr& attr, COLORREF clr, const Vector3D& normal, const Point3D& viewPoint, double ambient)
{
	const Point3D pt(pixel.x, pixel.y, pixel.z);

	double normACoefficient = attr.AmbientCoefficient / (attr.AmbientCoefficient + attr.DiffuseCoefficient + attr.SpecularCoefficient);
	double normDCoefficient = attr.DiffuseCoefficient / (attr.AmbientCoefficient + attr.DiffuseCoefficient + attr.SpecularCoefficient);
	double normSCoefficient = attr.SpecularCoefficient / (attr.AmbientCoefficient + attr.DiffuseCoefficient + attr.SpecularCoefficient);

	double rgbValues[] = { (double)(GetRValue(clr)), (double)(GetGValue(clr)), (double)(GetBValue(clr)) };

	const Vector3D n = normal.Normalized();
	const Vector3D viewVec = (viewPoint - pt).Normalized();

	for (int i = 0; i < 3; ++i)
	{
		double currLightPart = 0.0;
		for (auto j = lights.begin(); j != lights.end(); ++j)
		{
			const Vector3D lightVec = j->_type == LightSource::POINT ?
				(Point3D(j->_origin) - pt).Normalized()
				:
				j->Direction();
			const Vector3D reflectVec = lightVec - 2 * (lightVec*n)*n;
			double viewProd = -(reflectVec * viewVec);
			double dotProd = n * lightVec;
			currLightPart += j->_intensity * (normDCoefficient*dotProd + normSCoefficient*pow(viewProd, attr.SpecularPower));
		}
		rgbValues[i] = rgbValues[i] * (ambient*normACoefficient + currLightPart);
		rgbValues[i] = min(rgbValues[i], 255.0);
	}

	int rgbInts[] = { rgbValues[0], rgbValues[1], rgbValues[2] };

	return RGB(rgbInts[0], rgbInts[1], rgbInts[2]);
}
