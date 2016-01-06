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

struct XData
{
	enum PixelType { MIDDLE, UPPER_LIMIT, LOWER_LIMIT };
	XData(int x_, double z_, int lid, PixelType pt = MIDDLE)
		: x(x_), z(z_), LineId(lid), Type(pt)
	{}

	int x;
	int LineId;
	PixelType Type;
	double z;
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
	void SetPixel(int x, int y, double z, COLORREF clr)
	{
		minY = min(minY, y);
		maxY = max(maxY, y);
		if (xyMap.find(y) == xyMap.end())
		{
			xyMap[y] = std::vector<XData>();
		}
		xyMap[y].push_back(XData(x, z, clr));
	}

	std::map<int, std::vector<XData>> xyMap;	// y => <x, line id, pixel type>
	int minY = INT_MAX, maxY = -1;
};

struct MixedIntPoint
{
	MixedIntPoint(int x_, int y_, double z_)
		: x(x_), y(y_), z(z_)
	{}

	MixedIntPoint()
		: x(0), y(0), z(0)
	{}
	MixedIntPoint(int x_, int y_)
		: MixedIntPoint(x, y, 0.0)
	{}

	MixedIntPoint(const MixedIntPoint& other)
		: MixedIntPoint(other.x, other.y, other.z)
	{}

	MixedIntPoint(const Point3D& p)
		: MixedIntPoint(p.x, p.y, p.z)
	{}

	MixedIntPoint(const HomogeneousPoint& p)
		: MixedIntPoint(Point3D(p))
	{}

	int x, y;
	double z;
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

ClippingResult ApplyClippingAndViewMatrix(const HomogeneousPoint& p0, const HomogeneousPoint& p1, const MatrixHomogeneous& mFirst, const MatrixHomogeneous& mSecond, const MatrixHomogeneous& mTotal, const ClippingPlane& cp)
{
	const double clipValue0 = cp.Apply(mFirst * p0);
	const double clipValue1 = cp.Apply(mFirst * p1);

	if (clipValue0 < 0 && clipValue1 < 0)
	{
		return ClippingResult(LineSegment(HomogeneousPoint::Zeros, HomogeneousPoint::Zeros), true, true);
	}
	else if (clipValue0 < 0)
	{
		const HomogeneousPoint clippingPoint = HomogeneousPoint(cp.Intersection(Point3D(mFirst * p0), Point3D(mFirst * p1)));
		return ClippingResult(LineSegment(mSecond*clippingPoint, mTotal*p1), true, false);
	}
	else if (clipValue1 < 0)
	{
		const HomogeneousPoint clippingPoint = HomogeneousPoint(cp.Intersection(Point3D(mFirst * p0), Point3D(mFirst * p1)));
		return ClippingResult(LineSegment(mTotal*p0, mSecond*clippingPoint), false, true);
	}
	else
	{
		return ClippingResult(LineSegment(mTotal*p0, mTotal*p1), false, false);
	}
}

Polygon3D ApplyClippingAndViewMatrix(const Polygon3D& poly, const MatrixHomogeneous& mFirst, const MatrixHomogeneous& mSecond, const MatrixHomogeneous& mTotal, const ClippingPlane& cp)
{
	std::vector<HomogeneousPoint> resPoints;
	resPoints.reserve(poly.points.size());
	for (auto i = poly.points.begin(); i != poly.points.end(); ++i)
	{
		const HomogeneousPoint p0 = *i;
		const HomogeneousPoint p1 = (i + 1 != poly.points.end()) ? *(i + 1) : poly.points.front();
		const ClippingResult clipRes = ApplyClippingAndViewMatrix(p0, p1, mFirst, mSecond, mTotal, cp);
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

bool innerBinarySearchInXDataVector(const std::vector<XData>::const_iterator a, const std::vector<XData>::const_iterator b, const std::vector<XData>::const_iterator end, int x)
{
	if ((a != end && a->x == x) || (b != end && b->x == x))
		return true;
	else if (a == b)
		return false;
	const std::vector<XData>::const_iterator mid = a + ((b - a) >> 1);
	if (mid != end && mid->x == x)
		return true;
	return innerBinarySearchInXDataVector(a, mid, end, x) || innerBinarySearchInXDataVector(mid + 1, b, end, x);
}

bool BinarySearchInXDataVector(const std::vector<XData>& v, int x)
{
	return innerBinarySearchInXDataVector(v.begin(), v.end(), v.end(), x);
}

void DrawPolygon(DrawingObject& img, const Polygon3D& poly0, const MatrixHomogeneous& mFirst, const MatrixHomogeneous& mSecond, const MatrixHomogeneous& mTotal, const ModelAttr& attr, COLORREF objColor, bool objColorValid, const Normals::PolygonNormalData& nd, bool fillPolygons, bool clip = false, const ClippingPlane& cp = ClippingPlane(0, 0, 0, 0))
{
	if (poly0.points.size() < 2)
	{
		return;
	}

	FakeXYMap xyMap;

	const Polygon3D poly = clip ? ApplyClippingAndViewMatrix(poly0, mFirst, mSecond, mTotal, cp) : (mTotal * poly0);

	MixedIntPoint p0, p1;
	bool getP0 = true;
	for (auto i = poly.points.begin(); i != poly.points.end(); ++i)
	{
		COLORREF actualColor = GetActualColor(objColor, objColorValid, poly, *i, attr);

		const LineSegment l = ((i + 1) != poly.points.end()) ?
			LineSegment(*i, *(i + 1)) :
			LineSegment(*i, poly.points.front());

		if (getP0)
		{
			p0 = MixedIntPoint(l.p0);
			getP0 = false;
		}

		if (l.p0.w == 0 || l.p1.w == 0)
		{
			getP0 = true;
			continue;
		}

		p1 = MixedIntPoint(l.p1);
		innerDrawLine(img, p0, p1, actualColor, attr.line_width, img.active == DrawingObject::DRAWING_OBJECT_ZBUF);

		if (p0.y != p1.y)
		{
			innerDrawLine(xyMap, p0, p1, i - poly.points.begin(), 1, img.active == DrawingObject::DRAWING_OBJECT_ZBUF);

			for (auto pIt = xyMap.xyMap[p0.y].begin(); pIt != xyMap.xyMap[p0.y].end(); ++pIt)
			{
				if (pIt->x == p0.x && pIt->LineId == (i - poly.points.begin()))
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
				if (pIt->x == p1.x && pIt->LineId == (i - poly.points.begin()))
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
			xyMap.SetPixel(p0.x, p0.y, p0.z, i - poly.points.begin());
			xyMap.SetPixel(p1.x, p1.y, p1.z, i - poly.points.begin());
			for (auto pIt = xyMap.xyMap[p0.y].begin(); pIt != xyMap.xyMap[p0.y].end(); ++pIt)
			{
				if (pIt->LineId == (i - poly.points.begin()))
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

	if (!fillPolygons)
	{
		return;
	}
	for (int y = xyMap.minY; y <= xyMap.maxY; ++y) {
		if (y < 0)
			continue;
		std::vector<XData> currRow = xyMap.xyMap[y];
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

			if (draw && !BinarySearchInXDataVector(origRow, x))
			{
				if (img.active == DrawingObject::DRAWING_OBJECT_ZBUF)
				{
					while (x > (interpolationIter0 + 1)->x)
					{
						++interpolationIter0;
					}
					const int x0 = (interpolationIter0)->x, x1 = (interpolationIter0 + 1)->x;
					const double z0 = (interpolationIter0)->z, z1 = (interpolationIter0 + 1)->z;
					const double currZ = LinearInterpolate(x, x0, x1, z0, z1);
					img.SetPixel(x, y, currZ, RGB(0, 0, 255));
				}
				else
				{
					img.SetPixel(x, y, 0.0, RGB(0, 0, 255));
				}
			}
		}
	}
}

void DrawObject(DrawingObject& img, const PolygonalObject& obj, const MatrixHomogeneous& mFirst, const MatrixHomogeneous& mSecond, const MatrixHomogeneous& mTotal, const ModelAttr& attr, const std::vector<Normals::PolygonNormalData>& normals, bool fillPolygons, bool clip, const ClippingPlane& cp)
{
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
			if (normalDir * Vector3D(0, 0, 1) < 0)
			{
				draw = true;
			}
		}
		if (draw)
		{
			DrawPolygon(img, obj.polygons[i], mFirst, mSecond, mTotal, attr, obj.color, obj.colorValid, normals[i], fillPolygons, clip, cp);
		}
	}
}
