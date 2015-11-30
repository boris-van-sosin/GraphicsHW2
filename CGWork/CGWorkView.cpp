// CGWorkView.cpp : implementation of the CCGWorkView class
//
#include "stdafx.h"
#include "CGWork.h"

#include "CGWorkDoc.h"
#include "CGWorkView.h"

#include <iostream>
using std::cout;
using std::endl;
#include "MaterialDlg.h"
#include "LightDialog.h"

#include "Geometry.h"
#include "GeometricTransformations.h"
#include "Drawing.h"

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
	ON_COMMAND(ID_LIGHT_SHADING_FLAT, OnLightShadingFlat)
	ON_UPDATE_COMMAND_UI(ID_LIGHT_SHADING_FLAT, OnUpdateLightShadingFlat)
	ON_COMMAND(ID_LIGHT_SHADING_GOURAUD, OnLightShadingGouraud)
	ON_UPDATE_COMMAND_UI(ID_LIGHT_SHADING_GOURAUD, OnUpdateLightShadingGouraud)
	ON_COMMAND(ID_LIGHT_CONSTANTS, OnLightConstants)
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_WM_KEYDOWN()
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
}

CCGWorkView::~CCGWorkView()
{
	//delete _bbox;
}


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

	applyMat(mat, 0);
	Invalidate();
}

void CCGWorkView::translate(const Axis& axis,double dist) {
	MatrixHomogeneous mat = Matrices::Translate(axis == AXIS_X ? dist : 0, axis == AXIS_Y ? dist : 0, axis == AXIS_Z ? dist : 0);

	applyMat(mat, 0);
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
	
	if (m_nAction == ID_ACTION_ROTATE) {
		double rotate_angle = zdelta / WHEEL_DELTA;
		rotate_angle /= 10;

		rotate(rotate_angle);
	}
	if (m_nAction == ID_ACTION_TRANSLATE) {
		double dist = zdelta / WHEEL_DELTA;
		dist /= 10;
		dist = -dist;
		translate(axis, dist);
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

/////////////////////////////////////////////////////////////////////////////
// Apply matrix on a model
/////////////////////////////////////////////////////////////////////////////
bool CCGWorkView::applyMat(const MatrixHomogeneous& mat, int ibj_idx) {
	if (ibj_idx >= _models.size()) {
		return false;
	}
	/*
	for (auto it = _objects.begin(); it != _objects.end(); ++it) {
	(*it) = mat * (*it);
	}
	*/
	model_t& model = _models[ibj_idx];
	for (auto it = model.begin(); it != model.end(); ++it) {
		(*it) = mat * (*it);
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

void swap(int& x, int& y) {
	int  z = x;
	x = y;
	y = z;
}

void innerDrawLine(CImage& img, int x0, int y0, int x1, int y1, COLORREF clr, unsigned int line_width) {
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
			ImageSetPixel(img, x0, y0, clr);
			for (int i = 1; i < line_width; i++) {
				ImageSetPixel(img, x0, y0 + i, clr);
				ImageSetPixel(img, x0, y0 - i, clr);
			}
		}
		else
		{
			ImageSetPixel(img, y0, x0, clr);
			for (int i = 1; i < line_width; i++) {
				ImageSetPixel(img, y0 + i, x0, clr);
				ImageSetPixel(img, y0 - i, x0, clr);
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
				ImageSetPixel(img, x, y, clr);
				for (int i = 1; i < line_width; i++) {
					ImageSetPixel(img, x, y + i, clr);
					ImageSetPixel(img, x, y - i, clr);
				}
			}
			else
			{
				ImageSetPixel(img, y, x, clr);
				for (int i = 1; i < line_width; i++) {
					ImageSetPixel(img, y + i, x, clr);
					ImageSetPixel(img, y - i, x, clr);
				}
			}
		}
	}
	else if (dx == 0 && dy == 0)
	{
		ImageSetPixel(img, x0, y0, clr);
		for (int i = 1; i < line_width; i++) {
			ImageSetPixel(img, x0, y0 + i, clr);
			ImageSetPixel(img, x0, y0 - i, clr);
			ImageSetPixel(img, x0 + i, y0, clr);
			ImageSetPixel(img, x0 - i, y0, clr);
		}
	}
}
// temp code end

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

		innerDrawLine(img, points[i][0], points[i][1], points[i][2], points[i][3], RGB(255, 0, 0), 1);
	}
}

void DrawLineSegment(CImage& img, const Point3D& p0, const Point3D& p1, COLORREF clr, unsigned int line_width)
{
	innerDrawLine(img, p0.x, p0.y, p1.x, p1.y, clr, line_width);
}

void DrawLineSegment(CImage& img, const LineSegment& line, COLORREF clr, unsigned int line_width)
{
	DrawLineSegment(img, line.p0, line.p1, clr, line_width);
}

void DrawPolygon(CImage& img, const Polygon3D& poly, const model_attr_t& attr)
{
	if (poly.points.size() < 2)
	{
		return;
	}
	for (std::vector<Point3D>::const_iterator i = poly.points.begin(); i != poly.points.end(); ++i)
	{
		if (i + 1 != poly.points.end())
		{
			DrawLineSegment(img, *i, *(i + 1), attr.color, attr.line_width);
		}
		else
		{
			DrawLineSegment(img, *i, poly.points.front(), attr.color, attr.line_width);
		}
	}
}

void DrawObject(CImage& img, const PolygonalObject& obj, const model_attr_t& attr)
{
	for (std::vector<Polygon3D>::const_iterator i = obj.polygons.begin(); i != obj.polygons.end(); ++i)
	{
		DrawPolygon(img, *i, attr);
		// test:
		//std::pair<double, Point3D> p = i->AreaAndCentroid();
		//DrawLineSegment(img, LineSegment(p.second, p.second + ((i->Normal()) * 10)), RGB(0, 255, 0), 1);
		//
	}
}

void CCGWorkView::OnDraw(CDC* pDC)
{
	CCGWorkDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	RECT rect;
	GetWindowRect(&rect);

	int h = rect.bottom - rect.top;
	int w = rect.right - rect.left;

	if (!_pxl2obj.IsNull()) {
		_pxl2obj.Destroy();
	}
	_pxl2obj.Create(w, h, 32);

	CImage img;
	img.Create(w, h, 32);
	DrawScene(img);
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
		CGSkelProcessIritDataFiles(m_strItdFileName, 1, _models.back());
		FlipYAxis(_models.size() - 1);
		//delete _bbox;
		//_bbox = new BoundingBox(BoundingBox::OfObjects(_objects));
		_bboxes.push_back(BoundingBox(BoundingBox::OfObjects(_models.back())));
		_model_attr.push_back(model_attr_t());
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

void CCGWorkView::DrawScene(CImage& img)
{
	const int margin = 5;
	RECT rect;
	GetWindowRect(&rect);

	int height = rect.bottom - rect.top - 2 * margin;
	int width = rect.right - rect.left - 2 * margin;

	for (int i = 0; i < _models.size(); i++) {
		const BoundingBox bCube = _bboxes[i].BoundingCube();
		const MatrixHomogeneous mPersp = PerspectiveWarpMatrix(bCube);

		MatrixHomogeneous m = m_bIsPerspective ?
			(Matrices::Translate(width*0.5, height*0.5, 0) * Matrices::Scale(min(width, height)) * mPersp)
			: (Matrices::Translate(width*0.5, height*0.5, 0) * Matrices::Scale(0.5*min(width, height)) * ScaleAndCenter(bCube));

		model_t& model = _models[i];
		const model_attr_t attr = _model_attr[i];
		model_attr_t shadow_attr = attr;
		for (std::vector<PolygonalObject>::iterator it = model.begin(); it != model.end(); ++it)
		{
			DrawObject(img, m*(*it), attr);

			shadow_attr.color = i + 1;
			DrawObject(_pxl2obj, m*(*it), shadow_attr);
		}
	}
}
