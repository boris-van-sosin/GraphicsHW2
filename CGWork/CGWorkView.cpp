// CGWorkView.cpp : implementation of the CCGWorkView class
//
#include "stdafx.h"
#include "CGWork.h"

#include "CGWorkDoc.h"
#include "CGWorkView.h"

#include <iostream>
#include <algorithm>
using std::cout;
using std::endl;
#include "MaterialDlg.h"
#include "LightDialog.h"

#include "Geometry.h"
#include "GeometricTransformations.h"
#include "Drawing.h"
#include "ChooseColorDlg.h"
#include "ClippingDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "PngWrapper.h"
#include "iritSkel.h"


// For Status Bar access
#include "MainFrm.h"

// Use this macro to display text messages in the status bar.
#define STATUS_BAR_TEXT(str) (((CMainFrame*)GetParentFrame())->getStatusBar().SetWindowText(str))


/////////////////////////////////////////////////////////////////////////////
// CCGWorkView

IMPLEMENT_DYNCREATE(CCGWorkView, CView)

BEGIN_MESSAGE_MAP(CCGWorkView, CView)
	//{{AFX_MSG_MAP(CCGWorkView)
	ON_WM_ERASEBKGND()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_COMMAND(ID_FILE_LOAD, OnFileLoad)
	ON_COMMAND(ID_VIEW_ORTHOGRAPHIC, OnViewOrthographic)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ORTHOGRAPHIC, OnUpdateViewOrthographic)
	ON_COMMAND(ID_VIEW_PERSPECTIVE, OnViewPerspective)
	ON_UPDATE_COMMAND_UI(ID_VIEW_PERSPECTIVE, OnUpdateViewPerspective)
	ON_COMMAND(ID_ACTION_ROTATE, OnActionRotate)
	ON_UPDATE_COMMAND_UI(ID_ACTION_ROTATE, OnUpdateActionRotate)
	ON_COMMAND(ID_ACTION_SCALE, OnActionScale)
	ON_UPDATE_COMMAND_UI(ID_ACTION_SCALE, OnUpdateActionScale)
	ON_COMMAND(ID_ACTION_TRANSLATE, OnActionTranslate)
	ON_UPDATE_COMMAND_UI(ID_ACTION_TRANSLATE, OnUpdateActionTranslate)
	ON_COMMAND(ID_AXIS_X, OnAxisX)
	ON_UPDATE_COMMAND_UI(ID_AXIS_X, OnUpdateAxisX)
	ON_COMMAND(ID_AXIS_Y, OnAxisY)
	ON_UPDATE_COMMAND_UI(ID_AXIS_Y, OnUpdateAxisY)
	ON_COMMAND(ID_AXIS_Z, OnAxisZ)
	ON_UPDATE_COMMAND_UI(ID_AXIS_Z, OnUpdateAxisZ)
	ON_COMMAND(ID_POLYGON_NORMALS, OnTogglePolygonNormals)
	ON_COMMAND(ID_VERTEX_NORMALS, OnToggleVertexNormals)
	ON_COMMAND(ID_TOGGLE_MODEL_BBOX, OnToggleModelBBox)
	ON_COMMAND(ID_TOGGLE_SUB_BBOX, OnToggleSubObjBBox)
	ON_COMMAND(ID_TOGGLE_ALL_MODEL_BBOX, OnToggleAllModelBBox)
	ON_COMMAND(ID_TOGGLE_ALL_SUB_BBOX, OnToggleAllSubObjBBox)
	ON_COMMAND(ID_CHOOSE_COLORS, OnChooseColors)
	ON_COMMAND(ID_GENERAL_SETTINGS, OnSettings)
	ON_COMMAND(ID_LIGHT_SHADING_FLAT, OnLightShadingFlat)
	ON_UPDATE_COMMAND_UI(ID_LIGHT_SHADING_FLAT, OnUpdateLightShadingFlat)
	ON_COMMAND(ID_LIGHT_SHADING_GOURAUD, OnLightShadingGouraud)
	ON_UPDATE_COMMAND_UI(ID_LIGHT_SHADING_GOURAUD, OnUpdateLightShadingGouraud)
	ON_COMMAND(ID_LIGHT_CONSTANTS, OnLightConstants)
	ON_COMMAND(ID_CHANGE_VIEW, OnChangeView)
	ON_UPDATE_COMMAND_UI(ID_CHANGE_VIEW, OnUpdateChangeView)
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONDOWN()
	ON_COMMAND(ID_SAVE, OnFileSave)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


// A patch to fix GLaux disappearance from VS2005 to VS2008
void auxSolidCone(GLdouble radius, GLdouble height) {
	GLUquadric *quad = gluNewQuadric();
	gluQuadricDrawStyle(quad, GLU_FILL);
	gluCylinder(quad, radius, 0.0, height, 20, 20);
	gluDeleteQuadric(quad);
}

/////////////////////////////////////////////////////////////////////////////
// CCGWorkView construction/destruction

CCGWorkView::CCGWorkView()
//	: _bbox(NULL)
: _drawObj()
{
	// Set default values
	m_nAxis = ID_AXIS_X;
	m_nAction = ID_ACTION_ROTATE;
	m_nView = ID_VIEW_ORTHOGRAPHIC;
	m_bIsPerspective = false;

	m_nLightShading = ID_LIGHT_SHADING_FLAT;

	m_lMaterialAmbient = 0.2;
	m_lMaterialDiffuse = 0.8;
	m_lMaterialSpecular = 1.0;
	m_nMaterialCosineFactor = 32;

	//init the first light to be enabled
	m_lights[LIGHT_ID_1].enabled = true;

	_displayPolygonNormals = false;
	_displayVertexNormals = false;
	_dummyDisplayModelBBox = _dummyDisplaySubObjBBox = false;
	_normalsColor = RGB(0, 255, 0);

	_backgroundColor = RGB(0, 0, 0);
	_backgroundBrush = CreateSolidBrush(_backgroundColor);
}

CCGWorkView::~CCGWorkView()
{
	//delete _bbox;
}

const COLORREF CCGWorkView::DefaultModelColor(RGB(0, 0, 255));

/////////////////////////////////////////////////////////////////////////////
// CCGWorkView diagnostics

#ifdef _DEBUG
void CCGWorkView::AssertValid() const
{
	CView::AssertValid();
}

void CCGWorkView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CCGWorkDoc* CCGWorkView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CCGWorkDoc)));
	return (CCGWorkDoc*)m_pDocument;
}
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
// CCGWorkView Window Creation - Linkage of windows to CGWork

BOOL CCGWorkView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	// An CGWork window must be created with the following
	// flags and must NOT include CS_PARENTDC for the
	// class style.

	cs.style |= WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	return CView::PreCreateWindow(cs);
}



int CCGWorkView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	InitializeCGWork();

	return 0;
}


// This method initialized the CGWork system.
BOOL CCGWorkView::InitializeCGWork()
{
	m_pDC = new CClientDC(this);

	if (NULL == m_pDC) { // failure to get DC
		::AfxMessageBox(_T("Couldn't get a valid DC."));
		return FALSE;
	}

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CCGWorkView message handlers


void CCGWorkView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	if (0 >= cx || 0 >= cy) {
		return;
	}

	// save the width and height of the current window
	m_WindowWidth = cx;
	m_WindowHeight = cy;

	// compute the aspect ratio
	// this will keep all dimension scales equal
	m_AspectRatio = (GLdouble)m_WindowWidth / (GLdouble)m_WindowHeight;
}


BOOL CCGWorkView::SetupViewingFrustum(void)
{
	return TRUE;
}


// This viewing projection gives us a constant aspect ration. This is done by
// increasing the corresponding size of the ortho cube.
BOOL CCGWorkView::SetupViewingOrthoConstAspect(void)
{
	return TRUE;
}





BOOL CCGWorkView::OnEraseBkgnd(CDC* pDC)
{
	// Windows will clear the window with the background color every time your window 
	// is redrawn, and then CGWork will clear the viewport with its own background color.

	// return CView::OnEraseBkgnd(pDC);
	return true;
}

void CCGWorkView::OnMouseMove(UINT nFlags, CPoint point) {
	COLORREF idx = _pxl2obj.GetPixel(point.x, point.y);
	if (idx == 0) {
		// background
		if (glowing_object == -1)
			return;

		// unglow
		_model_attr[glowing_object].line_width = 1;
		glowing_object = -1;
		Invalidate();
		return;
	}

	idx--;

	glowing_object = idx;
	_model_attr[idx].line_width = 3;
	Invalidate();
}

void CCGWorkView::rotate(double rotate_angle) {
	Axis axis;
	if (m_nAxis == ID_AXIS_X)
		axis = AXIS_X;
	if (m_nAxis == ID_AXIS_Y)
		axis = AXIS_Y;
	if (m_nAxis == ID_AXIS_Z)
		axis = AXIS_Z;

	MatrixHomogeneous mat = Matrices::Rotate(axis, rotate_angle);

	applyMat(mat);
	Invalidate();
}

void CCGWorkView::translate(const Axis& axis,double dist) {
	MatrixHomogeneous mat = Matrices::Translate(axis == AXIS_X ? dist : 0, axis == AXIS_Y ? dist : 0, axis == AXIS_Z ? dist : 0);

	applyMat(mat);
	Invalidate();
}

void CCGWorkView::scale(const Axis& axis, double factor) {
	MatrixHomogeneous mat = Matrices::Scale(axis == AXIS_X ? factor : 1, axis == AXIS_Y ? factor : 1, axis == AXIS_Z ? factor : 1);
	applyMat(mat);
	Invalidate();
}

BOOL CCGWorkView::OnMouseWheel(UINT flags, short zdelta, CPoint point) {
	Axis axis;
	if (m_nAxis == ID_AXIS_X)
		axis = AXIS_X;
	if (m_nAxis == ID_AXIS_Y)
		axis = AXIS_Y;
	if (m_nAxis == ID_AXIS_Z)
		axis = AXIS_Z;
	
	if (active_object >= _models.size())
		return true;
	double sensitivity = _model_attr[active_object].sensitivity;

	if (m_nAction == ID_ACTION_ROTATE) {
		double rotate_angle = zdelta / WHEEL_DELTA;
		//rotate_angle /= 10;
		rotate_angle *= sensitivity;

		rotate(rotate_angle);
	}
	if (m_nAction == ID_ACTION_TRANSLATE) {
		double dist = zdelta / WHEEL_DELTA;
		//dist /= 10;
		dist *= sensitivity;
		dist = -dist;
		translate(axis, dist);
	}
	if (m_nAction == ID_ACTION_SCALE) {
		double factor = zdelta / WHEEL_DELTA;
		//factor *= 1.1;
		factor *= (1 + sensitivity);
		factor = factor < 0 ? -1 / factor : factor;
		scale(axis, factor);
	}


	return true;
}

afx_msg void CCGWorkView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) {

	double rotate_gran = 1000;
	double move_gran = 5000;

	switch (nChar) {
	case 90: // z
		rotate(-(double)nChar * (double)nRepCnt / rotate_gran);
		break;
	case 88: // x
		rotate((double)nChar * (double)nRepCnt / rotate_gran);
		break;
	case 87: // w
		translate(AXIS_Y, -(double)nChar * (double)nRepCnt / move_gran);
		break;
	case 65: // a
		translate(AXIS_X, -(double)nChar * (double)nRepCnt / move_gran);
		break;
	case 83: // s
		translate(AXIS_Y, (double)nChar * (double)nRepCnt / move_gran);
		break;
	case 68: // d
		translate(AXIS_X, (double)nChar * (double)nRepCnt / move_gran);
		break;
	default:
		break;
	}
}

afx_msg void CCGWorkView::OnLButtonDown(UINT nHitTest, CPoint point) {
	if (glowing_object == -1)
		return;
	active_object = glowing_object;
}

/////////////////////////////////////////////////////////////////////////////
// Apply matrix on a model
/////////////////////////////////////////////////////////////////////////////
bool CCGWorkView::applyMat(const MatrixHomogeneous& mat) {
	if (active_object >= _models.size()) {
		return false;
	}
	/*
	for (auto it = _objects.begin(); it != _objects.end(); ++it) {
	(*it) = mat * (*it);
	}
	*/
	model_t& model = _models[active_object];
	const model_t& clean_model = _clean_models[active_object];
	model = clean_model;
	
	if (_in_object_view)
		_model_space_transformations[active_object] = _model_space_transformations[active_object] * mat;
	else
		_view_space_transformations[active_object] = mat * _view_space_transformations[active_object];
	
	const MatrixHomogeneous& mat1 = _view_space_transformations[active_object] * _model_space_transformations[active_object];

	for (auto it = model.begin(); it != model.end(); ++it) {
		(*it) = mat1 * (*it);
	}
	for (int i = 0; i <_polygonNormals[active_object].size(); ++i)
	{
		//(*it) = mat * (*it);
		_polygonNormals[active_object][i] = mat1 * _clean_polygonNormals[active_object][i];
	}
	for (int i = 0; i < _vertexNormals[active_object].size(); ++i)
	{
		_vertexNormals[active_object][i] = mat1 * _clean_vertexNormals[active_object][i];
	}

	_modelBoundingBoxes[active_object] = mat1 * _clean_modelBoundingBoxes[active_object];
	for (int i = 0; i < _subObjectBoundingBoxes[active_object].size(); ++i)
	{
		_subObjectBoundingBoxes[active_object][i] = mat1 * _clean_subObjectBoundingBoxes[active_object][i];
	}


	return true;
}


/////////////////////////////////////////////////////////////////////////////
// CCGWorkView drawing
/////////////////////////////////////////////////////////////////////////////

// this is temp code
/*
plotLine(x0,y0, x1,y1)
dx=x1-x0
dy=y1-y0

D = 2*dy - dx
plot(x0,y0)
y=y0

for x from x0+1 to x1
plot(x,y)
D = D + (2*dy)
if D > 0
y = y+1
D = D - (2*dx)
*/

void swap(int& x, int& y) {
	int  z = x;
	x = y;
	y = z;
}

void innerDrawLine(DrawingObject& img, int x0, int y0, int x1, int y1, COLORREF clr, unsigned int line_width) {
	if (x0 > x1) {
		swap(x0, x1);
		swap(y0, y1);
	}

	int dx = x1 - x0;
	int dy = y1 - y0;

	bool swapXY = false;
	if (abs(dy) > dx)
	{
		swap(x0, y0);
		swap(x1, y1);
		if (x0 > x1) {
			swap(x0, x1);
			swap(y0, y1);
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
		if (!swapXY)
		{
			img.SetPixel(x0, y0, 0.0, clr);
			for (unsigned int i = 1; i < line_width; i++) {
				img.SetPixel(x0, y0 + i, 0.0, clr);
				img.SetPixel(x0, y0 - i, 0.0, clr);
			}
		}
		else
		{
			img.SetPixel(y0, x0, 0.0, clr);
			for (unsigned int i = 1; i < line_width; i++) {
				img.SetPixel(y0 + i, x0, 0.0, clr);
				img.SetPixel(y0 - i, x0, 0.0, clr);
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
			if (!swapXY)
			{
				img.SetPixel(x, y, 0.0, clr);
				for (unsigned int i = 1; i < line_width; i++) {
					img.SetPixel(x, y + 1, 0.0, clr);
					img.SetPixel(x, y - 1, 0.0, clr);
				}
			}
			else
			{
				img.SetPixel(y, x, 0.0, clr);
				for (unsigned int i = 1; i < line_width; i++) {
					img.SetPixel(y + i, x, 0.0, clr);
					img.SetPixel(y - i, x, 0.0, clr);
				}
			}
		}
	}
	else if (dx == 0 && dy == 0)
	{
		img.SetPixel(x0, y0, 0.0, clr);
		for (unsigned int i = 1; i < line_width; i++) {
			img.SetPixel(x0, y0 + i, 0.0, clr);
			img.SetPixel(x0, y0 - i, 0.0, clr);
			img.SetPixel(x0 + i, y0, 0.0, clr);
			img.SetPixel(x0 - i, y0, 0.0, clr);
		}
	}
}

void DrawTestLines(CDC* pDC, CImage& img)
{
	// some drawing teset

	int points[][4] = {
			{ 0, 0, 700, 50 },
			{ 0, 0, 100, 50 },
			{ 0, 300, 100, 250 },
			{ 0, 300, 800, 250 },
			{ 300, 300, 200, 250 },
			{ 50, 300, 70, 0 },
			{ 100, 50, 150, 400 },
			{ 300, 300, 400, 0 },
			{ 150, 150, 150, 20 },
			{ 70, 60, 70, 80 },
			{ 50, 80, 100, 80 },
			{ 100, 90, 50, 90 },
	};

	int xd = 500, yd = 0;

	for (int i = 0; i < sizeof(points) / sizeof(points[0]); ++i) {
		pDC->MoveTo(xd + points[i][0], yd + points[i][1]);
		LineTo(*pDC, xd + points[i][2], yd + points[i][3]);

		if (points[i][0] == 150)
		{
			int j = 0;
		}

		//innerDrawLine(img, points[i][0], points[i][1], points[i][2], points[i][3], RGB(255, 0, 0), 1);
	}
}

void DrawLineSegment(DrawingObject& img, const Point3D& p0, const Point3D& p1, COLORREF clr, unsigned int line_width, bool clip = false, const ClippingPlane& cp = ClippingPlane(0, 0, 0, 0))
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
			innerDrawLine(img, clippingPoint.x, clippingPoint.y, p1.x, p1.y, clr, line_width);
		}
		else if (clipValue1 < 0)
		{
			const Point3D clippingPoint = cp.Intersection(p0, p1);
			innerDrawLine(img, p0.x, p0.y, clippingPoint.x, clippingPoint.y, clr, line_width);
		}
		else
		{
			innerDrawLine(img, p0.x, p0.y, p1.x, p1.y, clr, line_width);
		}
	}
	else
	{
		innerDrawLine(img, p0.x, p0.y, p1.x, p1.y, clr, line_width);
	}
}

void DrawLineSegment(DrawingObject& img, const HomogeneousPoint& p0, const HomogeneousPoint& p1, COLORREF clr, unsigned int line_width, bool clip = false, const ClippingPlane& cp = ClippingPlane(0, 0, 0, 0))
{
	DrawLineSegment(img, Point3D(p0), Point3D(p1), clr, line_width, clip, cp);
}

void DrawLineSegment(DrawingObject& img, const LineSegment& line, COLORREF clr, unsigned int line_width, bool clip = false, const ClippingPlane& cp = ClippingPlane(0, 0, 0, 0))
{
	DrawLineSegment(img, line.p0, line.p1, clr, line_width, clip, cp);
}

inline COLORREF GetActualColor(COLORREF objColor, bool objColorValid, const Polygon3D& poly, const HomogeneousPoint& p, const model_attr_t& attr)
{
	COLORREF actualColor = CCGWorkView::DefaultModelColor;
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


bool CompareEdgesByY(const LineSegment& e0, const LineSegment& e1)
{
	return e0.p0.y < e1.p0.y;
}

void DrawPolygon(DrawingObject& img, const Polygon3D& poly, const model_attr_t& attr, COLORREF objColor, bool objColorValid, const Normals::PolygonNormalData& nd, bool clip = false, const ClippingPlane& cp = ClippingPlane(0, 0, 0, 0))
{
	if (poly.points.size() < 2)
	{
return;
	}
	for (auto i = poly.points.begin(); i != poly.points.end(); ++i)
	{
		COLORREF actualColor = GetActualColor(objColor, objColorValid, poly, *i, attr);

		if (i + 1 != poly.points.end())
		{
			DrawLineSegment(img, *i, *(i + 1), actualColor, attr.line_width, clip, cp);
		}
		else
		{
			DrawLineSegment(img, *i, poly.points.front(), actualColor, attr.line_width, clip, cp);
		}
	}
}

LineSegment ApplyClippingAndViewMatrix(const HomogeneousPoint& p0, const HomogeneousPoint& p1, const MatrixHomogeneous& mFirst, const MatrixHomogeneous& mSecond, const MatrixHomogeneous& mTotal, const ClippingPlane& cp)
{
	const double clipValue0 = cp.Apply(mFirst * p0);
	const double clipValue1 = cp.Apply(mFirst * p1);

	if (clipValue0 < 0 && clipValue1 < 0)
	{
		return LineSegment(HomogeneousPoint::Zeros, HomogeneousPoint::Zeros);
	}
	else if (clipValue0 < 0)
	{
		const HomogeneousPoint clippingPoint = HomogeneousPoint(cp.Intersection(Point3D(mFirst * p0), Point3D(mFirst * p1)));
		return LineSegment(mSecond*clippingPoint, mTotal*p1);
	}
	else if (clipValue1 < 0)
	{
		const HomogeneousPoint clippingPoint = HomogeneousPoint(cp.Intersection(Point3D(mFirst * p0), Point3D(mFirst * p1)));
		return LineSegment(mTotal*p0, mSecond*clippingPoint);
	}
	else
	{
		return LineSegment(mTotal*p0, mTotal*p1);
	}
}

void DrawPolygon(DrawingObject& img, const Polygon3D& poly, const MatrixHomogeneous& mFirst, const MatrixHomogeneous& mSecond, const MatrixHomogeneous& mTotal, const model_attr_t& attr, COLORREF objColor, bool objColorValid, const Normals::PolygonNormalData& nd, bool clip = false, const ClippingPlane& cp = ClippingPlane(0, 0, 0, 0))
{
	if (poly.points.size() < 2)
	{
		return;
	}

	for (auto i = poly.points.begin(); i != poly.points.end(); ++i)
	{
		COLORREF actualColor = GetActualColor(objColor, objColorValid, poly, *i, attr);

		if (i + 1 != poly.points.end())
		{
			if (clip)
			{
				DrawLineSegment(img, ApplyClippingAndViewMatrix(*i, *(i + 1), mFirst, mSecond, mTotal, cp), actualColor, attr.line_width);
			}
			else
			{
				DrawLineSegment(img, mTotal*(*i), mTotal*(*(i + 1)), actualColor, attr.line_width, clip, cp);
			}
		}
		else
		{
			if (clip)
			{
				DrawLineSegment(img, ApplyClippingAndViewMatrix(*i, poly.points.front(), mFirst, mSecond, mTotal, cp), actualColor, attr.line_width);
			}
			else
			{
				DrawLineSegment(img, mTotal*(*i), mTotal*poly.points.front(), actualColor, attr.line_width, clip, cp);
			}
		}
	}

	std::vector<LineSegment> edges = (mTotal * poly).Edges();
	for (auto e = edges.begin(); e != edges.end(); ++e) {
		if (e->p0.y > e->p1.y) {
			*e = LineSegment(e->p1, e->p0);
		}
	}

	std::sort(edges.begin(), edges.end(), CompareEdgesByY);


}

void DrawObject(DrawingObject& img, const PolygonalObject& obj, const model_attr_t& attr, const std::vector<Normals::PolygonNormalData>& normals, bool clip = false, const ClippingPlane& cp = ClippingPlane(0, 0, 0, 0))
{
	for (size_t i = 0; i != obj.polygons.size(); ++i)
	{
		if ((!attr.removeBackFace) ||
			((Vector3D(normals[i].PolygonNormal.p1) - Vector3D(normals[i].PolygonNormal.p0)) * Vector3D(0, 0, 1) < 0))
		{
			DrawPolygon(img, obj.polygons[i], attr, obj.color, obj.colorValid, normals[i], clip, cp);
		}
	}
}

void DrawObject(DrawingObject& img, const PolygonalObject& obj, const MatrixHomogeneous& mFirst, const MatrixHomogeneous& mSecond, const MatrixHomogeneous& mTotal, const model_attr_t& attr, const std::vector<Normals::PolygonNormalData>& normals, bool clip = false, const ClippingPlane& cp = ClippingPlane(0, 0, 0, 0))
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
			DrawPolygon(img, obj.polygons[i], mFirst, mSecond, mTotal, attr, obj.color, obj.colorValid, normals[i], clip, cp);
		}
	}
}

void CCGWorkView::OnDraw(CDC* pDC)
{
	CCGWorkDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	RECT rect;
	GetClientRect(&rect);

	int h = rect.bottom - rect.top;
	int w = rect.right - rect.left;

	if (!_pxl2obj.IsNull()) {
		_pxl2obj.Destroy();
	}
	_pxl2obj.Create(w, h, 32);

	CImage img;
	img.Create(w, h, 32);

	HDC imgDC = img.GetDC();
	FillRect(imgDC, &rect, _backgroundBrush);
	img.ReleaseDC();

	_drawObj.img = &img;
	_drawObj.active = DrawingObject::DRAWING_OBJECT_CIMG;

	DrawScene(_drawObj);
	img.BitBlt(*pDC, 0, 0, w, h, 0, 0);
	//temporary:
	img.Destroy();
}


/////////////////////////////////////////////////////////////////////////////
// CCGWorkView CGWork Finishing and clearing...

void CCGWorkView::OnDestroy()
{
	CView::OnDestroy();

	// delete the DC
	if (m_pDC) {
		delete m_pDC;
	}
}



/////////////////////////////////////////////////////////////////////////////
// User Defined Functions

void CCGWorkView::RenderScene() {
	// do nothing. This is supposed to be overriden...

	return;
}


void CCGWorkView::OnFileLoad()
{
	TCHAR szFilters[] = _T("IRIT Data Files (*.itd)|*.itd|All Files (*.*)|*.*||");

	CFileDialog dlg(TRUE, _T("itd"), _T("*.itd"), OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, szFilters);

	if (dlg.DoModal() == IDOK) {
		m_strItdFileName = dlg.GetPathName();		// Full path and filename
		PngWrapper p;
		_models.push_back(model_t());

		_model_space_transformations.push_back(Matrices::UnitMatrixHomogeneous);
		_view_space_transformations.push_back(Matrices::UnitMatrixHomogeneous);
		CGSkelProcessIritDataFiles(m_strItdFileName, 1, _models.back(), _polygonFineness);
		_clean_models.push_back(_models.back());
		
		_polygonNormals.push_back(std::vector<Normals::PolygonNormalData>());
		_vertexNormals.push_back(Normals::NormalList());
		_polygonAdjacencies.push_back(PolygonAdjacencyGraph());
		Normals::ComputeNormals(_models.back(), _polygonNormals.back(), _vertexNormals.back(), _polygonAdjacencies.back());

		//FlipYAxis(_models.size() - 1);

		_bboxes.push_back(BoundingBox(BoundingBox::OfObjects(_models.back())));

		const BoundingBox bcube = _bboxes.back().BoundingCube();

		_model_attr.push_back(model_attr_t());

		double gran_size = fmax(bcube.maxX - bcube.minX, fmax(bcube.maxY - bcube.minY, bcube.maxZ - bcube.minZ));
		_model_attr.back().sensitivity = 1.0 / gran_size;
		
		active_object = _models.size() - 1;

		_modelBoundingBoxes.push_back(BoundingBox::OfObjects(_models.back()).ToObject());
		
		_subObjectBoundingBoxes.push_back(BoundingBox::BoundingBoxObjectsOfSubObjects(_models.back()));

		_clean_polygonNormals.push_back(_polygonNormals.back());
		_clean_vertexNormals.push_back(_vertexNormals.back());
		_clean_modelBoundingBoxes.push_back(_modelBoundingBoxes.back());
		_clean_subObjectBoundingBoxes.push_back(_subObjectBoundingBoxes.back());

		assert(_models.size() == _polygonNormals.size());
		assert(_polygonNormals.size() == _vertexNormals.size());
		assert(_vertexNormals.size() == _bboxes.size());
		assert(_bboxes.size() == _model_attr.size());
		assert(_model_attr.size() == _models.size());
		assert(_model_attr.size() == _clean_models.size());
		assert(_model_attr.size() == _model_space_transformations.size());
		assert(_model_attr.size() == _view_space_transformations.size());

		assert(_model_attr.size() == _clean_polygonNormals.size());
		assert(_model_attr.size() == _clean_vertexNormals.size());
		assert(_model_attr.size() == _clean_modelBoundingBoxes.size());
		assert(_model_attr.size() == _clean_subObjectBoundingBoxes.size());

		// Open the file and read it.
		// Your code here...

		Invalidate();	// force a WM_PAINT for drawing.
	}
}





// VIEW HANDLERS ///////////////////////////////////////////

// Note: that all the following Message Handlers act in a similar way.
// Each control or command has two functions associated with it.

void CCGWorkView::OnViewOrthographic()
{
	m_nView = ID_VIEW_ORTHOGRAPHIC;
	m_bIsPerspective = false;
	Invalidate();		// redraw using the new view.
}

void CCGWorkView::OnUpdateViewOrthographic(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_nView == ID_VIEW_ORTHOGRAPHIC);
}

void CCGWorkView::OnViewPerspective()
{
	m_nView = ID_VIEW_PERSPECTIVE;
	m_bIsPerspective = true;
	Invalidate();
}

void CCGWorkView::OnUpdateViewPerspective(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_nView == ID_VIEW_PERSPECTIVE);
}




// ACTION HANDLERS ///////////////////////////////////////////

void CCGWorkView::OnActionRotate()
{
	m_nAction = ID_ACTION_ROTATE;
}

void CCGWorkView::OnUpdateActionRotate(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_nAction == ID_ACTION_ROTATE);
}

void CCGWorkView::OnActionTranslate()
{
	m_nAction = ID_ACTION_TRANSLATE;
}

void CCGWorkView::OnUpdateActionTranslate(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_nAction == ID_ACTION_TRANSLATE);
}

void CCGWorkView::OnActionScale()
{
	m_nAction = ID_ACTION_SCALE;
}

void CCGWorkView::OnUpdateActionScale(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_nAction == ID_ACTION_SCALE);
}




// AXIS HANDLERS ///////////////////////////////////////////


// Gets calles when the X button is pressed or when the Axis->X menu is selected.
// The only thing we do here is set the ChildView member variable m_nAxis to the 
// selected axis.
void CCGWorkView::OnAxisX()
{
	m_nAxis = ID_AXIS_X;
}

// Gets called when windows has to repaint either the X button or the Axis pop up menu.
// The control is responsible for its redrawing.
// It sets itself disabled when the action is a Scale action.
// It sets itself Checked if the current axis is the X axis.
void CCGWorkView::OnUpdateAxisX(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_nAxis == ID_AXIS_X);
}


void CCGWorkView::OnAxisY()
{
	m_nAxis = ID_AXIS_Y;
}

void CCGWorkView::OnUpdateAxisY(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_nAxis == ID_AXIS_Y);
}


void CCGWorkView::OnAxisZ()
{
	m_nAxis = ID_AXIS_Z;
}

void CCGWorkView::OnUpdateAxisZ(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_nAxis == ID_AXIS_Z);
}

void CCGWorkView::OnChangeView()
{
	//_in_object_view == ID_CHANGE_VIEW;
	if (_in_object_view)
		_in_object_view = false;
	else
		_in_object_view = true;
}

void CCGWorkView::OnUpdateChangeView(CCmdUI* pCmdUI)
{
	if (_in_object_view)
		pCmdUI->SetCheck(1);
	else
		pCmdUI->SetCheck(0);
}

void CCGWorkView::OnTogglePolygonNormals()
{
	_displayPolygonNormals = !_displayPolygonNormals;
	Invalidate();
}

void CCGWorkView::OnToggleVertexNormals()
{
	_displayVertexNormals = !_displayVertexNormals;
	Invalidate();
}

void CCGWorkView::OnChooseColors()
{
	Choose_color_param_t param;
	if (active_object >= _models.size())
		return;
	param.model_color = _model_attr[active_object].color;
	param.model_force_color = _model_attr[active_object].forceColor;
	param.normal_color = _model_attr[active_object].normal_color;
	param.background_color = _backgroundColor;
	CChooseColorDlg dlg(&param);
	if (dlg.DoModal() == IDOK)
	{
		_model_attr[active_object].forceColor = param.model_force_color;
		_model_attr[active_object].color = param.model_color;
		_model_attr[active_object].normal_color = param.normal_color;
		_model_attr[active_object].model_bbox_color = param.model_bbox_color;
		_model_attr[active_object].subObj_bbox_color = param.subObj_bbox_color;

		DeleteObject(_backgroundBrush);
		_backgroundBrush = CreateSolidBrush(_backgroundColor = param.background_color);

		Invalidate();
	}
}

void CCGWorkView::OnToggleModelBBox()
{
	if (active_object >= 0)
	{
		_model_attr[active_object].displayBBox = !_model_attr[active_object].displayBBox;
		Invalidate();
	}
}

void CCGWorkView::OnToggleSubObjBBox()
{
	if (active_object >= 0)
	{
		_model_attr[active_object].displaySubObjectBBox = !_model_attr[active_object].displaySubObjectBBox;
		Invalidate();
	}
}

void CCGWorkView::OnToggleAllModelBBox()
{
	_dummyDisplayModelBBox = !_dummyDisplayModelBBox;
	for (auto i = _model_attr.begin(); i != _model_attr.end(); ++i)
	{
		i->displayBBox = _dummyDisplayModelBBox;
	}
	Invalidate();
}

void CCGWorkView::OnToggleAllSubObjBBox()
{
	_dummyDisplaySubObjBBox = !_dummyDisplaySubObjBBox;
	for (auto i = _model_attr.begin(); i != _model_attr.end(); ++i)
	{
		i->displaySubObjectBBox = _dummyDisplaySubObjBBox;
	}
	Invalidate();
}

void CCGWorkView::OnSettings()
{
	GeneralSettings s;
	s._polygonFineness = _polygonFineness;
	s._nearClippingPlane = _nearClippingPlane;
	s._farClippingPlane = _farClippingPlane;
	s._sensitivity = 0;
	if (active_object < _model_attr.size())
		s._sensitivity = _model_attr[active_object].sensitivity;
	CClippingDlg dlg(s);
	if (dlg.DoModal() == IDOK)
	{
		_polygonFineness = s._polygonFineness;
		_nearClippingPlane = s._nearClippingPlane;
		_farClippingPlane = s._farClippingPlane;
		if (active_object < _model_attr.size())
			_model_attr[active_object].sensitivity = s._sensitivity;
		Invalidate();
	}
}

// OPTIONS HANDLERS ///////////////////////////////////////////




// LIGHT SHADING HANDLERS ///////////////////////////////////////////

void CCGWorkView::OnLightShadingFlat()
{
	m_nLightShading = ID_LIGHT_SHADING_FLAT;
}

void CCGWorkView::OnUpdateLightShadingFlat(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_nLightShading == ID_LIGHT_SHADING_FLAT);
}


void CCGWorkView::OnLightShadingGouraud()
{
	m_nLightShading = ID_LIGHT_SHADING_GOURAUD;
}

void CCGWorkView::OnUpdateLightShadingGouraud(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_nLightShading == ID_LIGHT_SHADING_GOURAUD);
}

// LIGHT SETUP HANDLER ///////////////////////////////////////////

void CCGWorkView::OnLightConstants()
{
	CLightDialog dlg;

	for (int id = LIGHT_ID_1; id<MAX_LIGHT; id++)
	{
		dlg.SetDialogData((LightID)id, m_lights[id]);
	}
	dlg.SetDialogData(LIGHT_ID_AMBIENT, m_ambientLight);

	if (dlg.DoModal() == IDOK)
	{
		for (int id = LIGHT_ID_1; id<MAX_LIGHT; id++)
		{
			m_lights[id] = dlg.GetDialogData((LightID)id);
		}
		m_ambientLight = dlg.GetDialogData(LIGHT_ID_AMBIENT);
	}
	Invalidate();
}


void CCGWorkView::FlipYAxis(int obj_idx)
{
	MatrixHomogeneous flipY = Matrices::Flip(AXIS_Y);
	for (std::vector<PolygonalObject>::iterator i = _models[obj_idx].begin(); i != _models[obj_idx].end(); ++i)
	{
		(*i) = flipY * (*i);
	}
}

void CCGWorkView::DrawScene(DrawingObject& img)
{
	const int margin = 5;
	RECT rect;
	GetWindowRect(&rect);
	
	int height = img.GetHeight();
	int width = img.GetWidth();

	for (size_t i = 0; i < _models.size(); i++) {
		const BoundingBox bCube = _bboxes[i].BoundingCube();
		const PerspectiveData perspData = PerspectiveWarpMatrix(bCube, _nearClippingPlane, _farClippingPlane);

		const COLORREF normalsColor = _model_attr[i].normal_color;

		MatrixHomogeneous mMoveToView = 
			(m_bIsPerspective ?
			(Matrices::Flip(AXIS_X)*perspData.ScaleAndMoveToView) :
			(Matrices::Flip(AXIS_Y)*ScaleAndCenter(bCube)));

		MatrixHomogeneous mProj = 
			(m_bIsPerspective ?
			perspData.PerspectiveWarp : Matrices::UnitMatrixHomogeneous);

		const BoundingBox displayBox = ((mProj*mMoveToView) * _bboxes[i]).BoundingCube();

		const double scalingFactor = 0.25 * min(width, height) /
			((displayBox.maxX - displayBox.minX));

		const MatrixHomogeneous mScale = Matrices::Translate(width*0.5, height*0.5, 0)*Matrices::Scale(scalingFactor);

		const MatrixHomogeneous mFirst = mMoveToView;
		const MatrixHomogeneous mSecond = mScale * mProj;
		const MatrixHomogeneous mTotal = mSecond * mFirst;

		model_t& model = _models[i];
		const model_attr_t attr = _model_attr[i];
		model_attr_t shadow_attr = attr;
		DrawingObject tmpDrawingObj;
		tmpDrawingObj.img = &_pxl2obj;
		tmpDrawingObj.active = DrawingObject::DRAWING_OBJECT_CIMG;
		for (std::vector<PolygonalObject>::iterator it = model.begin(); it != model.end(); ++it)
		{
			DrawObject(img, *it, mFirst, mSecond, mTotal, attr, _polygonNormals[i], m_bIsPerspective, perspData.NearPlane);
			shadow_attr.color = i + 1;
			shadow_attr.forceColor = true;
			DrawObject(tmpDrawingObj, *it, mFirst, mSecond, mTotal, shadow_attr, _polygonNormals[i], m_bIsPerspective, perspData.NearPlane);
		}

		if (true)
		{
			for (auto j = _polygonAdjacencies[i].begin(); j != _polygonAdjacencies[i].end(); ++j)
			{
				const Polygon3D& currPoly = _models[i][j->objIdx].polygons[j->polygonInObjIdx];
				const LineSegment edge(currPoly.points[j->vertexIdx], currPoly.points[(j->vertexIdx+1) % currPoly.points.size()]);
				COLORREF boundaryColor = ShiftColor(GetActualColor(_models[i][j->objIdx].color, _models[i][j->objIdx].colorValid, currPoly, edge.p0, attr), -100);
				if (j->polygonIdxs.size() == 1)
				{
					// boundary
					if (m_bIsPerspective)
					{
						DrawLineSegment(img, ApplyClippingAndViewMatrix(edge.p0, edge.p1, mFirst, mSecond, mTotal, perspData.NearPlane), boundaryColor, 1);
					}
					else
					{
						DrawLineSegment(img, mTotal*(edge.p0), mTotal*(edge.p1), boundaryColor, 1);
					}
				}
				else
				{
					const LineSegment n0 = TransformNormal(mTotal, _polygonNormals[i][j->polygonIdxs[0]].PolygonNormal);
					const LineSegment n1 = TransformNormal(mTotal, _polygonNormals[i][j->polygonIdxs[1]].PolygonNormal);
					const Vector3D optVector(0, 0, 1);
					const Vector3D n0Vec = Vector3D(n0.p1) - Vector3D(n0.p0);
					const Vector3D n1Vec = Vector3D(n1.p1) - Vector3D(n1.p0);
					if ((n0Vec * optVector) * (n1Vec * optVector) < 0)
					{
						// silhouette
						if (m_bIsPerspective)
						{
							DrawLineSegment(img, ApplyClippingAndViewMatrix(edge.p0, edge.p1, mFirst, mSecond, mTotal, perspData.NearPlane), boundaryColor, 1);
						}
						else
						{
							DrawLineSegment(img, mTotal*(edge.p0), mTotal*(edge.p1), boundaryColor, 1);
						}
					}
				}
			}
		}

		if (_displayPolygonNormals)
		{
			for (auto j = _polygonNormals[i].begin(); j != _polygonNormals[i].end(); ++j)
			{
				DrawLineSegment(img, TransformNormal(mTotal, j->PolygonNormal, 10), normalsColor, 1);
			}
		}
		if (_displayVertexNormals)
		{
			for (auto j = _vertexNormals[i].begin(); j != _vertexNormals[i].end(); ++j)
			{
				DrawLineSegment(img, TransformNormal(mTotal, *j, 10), normalsColor, 1);
			}
		}
		if (attr.displayBBox)
		{
			model_attr_t bboxAttr;
			bboxAttr.color = _model_attr[i].model_bbox_color;
			bboxAttr.forceColor = true;
			DrawObject(img, _modelBoundingBoxes[i], mFirst, mSecond, mTotal, bboxAttr, _polygonNormals[i], m_bIsPerspective, perspData.NearPlane);
		}
		if (attr.displaySubObjectBBox)
		{
			model_attr_t bboxAttr;
			bboxAttr.color = _model_attr[i].subObj_bbox_color;
			bboxAttr.forceColor = true;
			for (auto j = _subObjectBoundingBoxes[i].begin(); j != _subObjectBoundingBoxes[i].end(); ++j)
			{
				DrawObject(img, *j, mFirst, mSecond, mTotal, bboxAttr, _polygonNormals[i], m_bIsPerspective, perspData.NearPlane);
			}
		}
	}
}

void CCGWorkView::OnFileSave()
{
	TCHAR szFilters[] = _T("PNG image (*.png)|*.png|All Files (*.*)|*.*||");
	CFileDialog dlg(FALSE, _T("png"), _T("*.png"), OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, szFilters);
	if (dlg.DoModal() == IDOK)
	{
		const CString fileName = dlg.GetPathName();
		CImage img;
		img.Create(2000, 2000, 32);

		RECT r;
		r.top = r.left = 0;
		r.bottom = r.right = 2000;

		HDC imgDC = img.GetDC();
		FillRect(imgDC, &r, _backgroundBrush);
		img.ReleaseDC();

		DrawingObject tmpDrawingObj;
		ZBufferImage zbimg(2000, 2000);
		tmpDrawingObj.zBufImg = &zbimg;
		tmpDrawingObj.img = &_pxl2obj;
		tmpDrawingObj.active = DrawingObject::DRAWING_OBJECT_ZBUF;

		DrawScene(tmpDrawingObj);

		zbimg.DrawOnImage(img);

		img.Save(fileName, Gdiplus::ImageFormatPNG);
		img.Destroy();
	}
}
