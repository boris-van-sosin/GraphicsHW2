#include "Drawing.h"

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
		HomogeneousPoint(2 / (vright - vleft), 0, 0, -(vright + vleft) / (vright - vleft)),
		HomogeneousPoint(0, 2 / (vtop - vbottom), 0, -(vtop + vbottom) / (vtop - vbottom)),
		HomogeneousPoint(0, 0, 2 / (vnear - vfar), -(vnear + vfar) / (vfar - vnear)),
		HomogeneousPoint(0, 0, 0, 1)
	};
	return MatrixHomogeneous(rows);
}

PerspectiveData PerspectiveWarpMatrix(const BoundingBox& boundingCube, double n, double f)
{
	const double near2 = n;
	const double far2 = f;
	const double q2 = far2 / (far2 - near2);

	//return MatrixHomogeneous(rows) * Matrices::Translate(0, 0, boundingCube.minZ + near + depth*0.5);
	HomogeneousPoint rows2[] = {
		HomogeneousPoint(2, 0, 0, 0),
		HomogeneousPoint(0, 2, 0, 0),
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

ZBufferImage::ZBufferImage(size_t h, size_t w)
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

void ZBufferImage::SetSize(size_t h, size_t w)
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
	if (!_backgroundImage.IsNull())
		_backgroundImage.Destroy();
	_backgroundImage.Create(img.GetWidth(), img.GetHeight(), 32);

	// copy image:
	HDC bgdc = _backgroundImage.GetDC();
	img.BitBlt(bgdc, 0, 0, img.GetWidth(), img.GetHeight(), 0, 0);
	_backgroundImage.ReleaseDC();
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
	: zBufImg(NULL), img(NULL)
{
}

DrawingObject::DrawingObject(CImage& cimg, ZBufferImage& zbimg)
	: zBufImg(&zbimg), img(&cimg)
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
	if (active == DRAWING_OBJECT_CIMG && img)
		ImageSetPixel(*img, x, y, clr);
	else if (zBufImg)
		zBufImg->PushPixel(x, y, z, clr);
}

void DrawingObject::SetPixel(int x, int y, const Point3D& p0, const Point3D& p1, COLORREF clr)
{

}
