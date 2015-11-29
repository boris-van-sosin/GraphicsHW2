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
	m_lights[LIGHT_ID_1].enabled=true;
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

	if ( NULL == m_pDC ) { // failure to get DC
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

	if ( 0 >= cx || 0 >= cy ) {
		return;
	}

	// save the width and height of the current window
	m_WindowWidth = cx;
	m_WindowHeight = cy;

	// compute the aspect ratio
	// this will keep all dimension scales equal
	m_AspectRatio = (GLdouble)m_WindowWidth/(GLdouble)m_WindowHeight;
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
	// this is the handler of mouse move

	////////////////////////////
	//if (!(nFlags & MK_LBUTTON))
	//	return;
	//int a = 8;
	//Invalidate();
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

BOOL CCGWorkView::OnMouseWheel(UINT flags, short zdelta, CPoint point) {
	double rotate_angle = zdelta / WHEEL_DELTA;
	rotate_angle /= 10;
	
	rotate(rotate_angle);

	return true;
}

afx_msg void CCGWorkView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) {

	switch (nChar) {
	case 90: // z
		rotate(-(double)nChar * (double)nRepCnt / 1000.0);
		break;
	case 88: // x
		rotate((double)nChar * (double)nRepCnt / 1000.0);
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

	BYTE* pos = (BYTE*) img.GetPixelAddress(x, y);
	*pos = blue;
	*(pos + 1) = green;
	*(pos + 2) = red;
}

void swap(int& x, int& y) {
	int  z = x;
	x = y;
	y = z;
}

void innerDrawLine(CImage& img, int x0, int y0, int x1, int y1, COLORREF clr) {
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

	if (dx > 0 && dy >= 0 && dy<=dx) {

		int err = 2 * dy - dx;
		int horizontal = 2 * dy, diagonal = 2*(dy - dx);
		if (!swapXY)
		{
			ImageSetPixel(img, x0, y0, clr);
		}
		else
		{
			ImageSetPixel(img, y0, x0, clr);
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
			}
			else
			{
				ImageSetPixel(img, y, x, clr);
			}
		}
	}
	else if (dx==0 && dy==0)
	{
		ImageSetPixel(img, x0, y0, clr);
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

		innerDrawLine(img, points[i][0], points[i][1], points[i][2], points[i][3], RGB(255, 0, 0));
	}
}

void DrawLineSegment(CImage& img, const Point3D& p0, const Point3D& p1, COLORREF clr)
{
	innerDrawLine(img, p0.x, p0.y, p1.x, p1.y, clr);
}

void DrawLineSegment(CImage& img, const LineSegment& line, COLORREF clr)
{
	DrawLineSegment(img, line.p0, line.p1, clr);
}

void DrawPolygon(CImage& img, const Polygon3D& poly, COLORREF clr)
{
	if (poly.points.size() < 2)
	{
		return;
	}
	for (std::vector<Point3D>::const_iterator i = poly.points.begin(); i != poly.points.end(); ++i)
	{
		if (i + 1 != poly.points.end())
		{
			DrawLineSegment(img, *i, *(i + 1), clr);
		}
		else
		{
			DrawLineSegment(img, *i, poly.points.front(), clr);
		}
	}
}

void DrawObject(CImage& img, const PolygonalObject& obj, COLORREF clr)
{
	for (std::vector<Polygon3D>::const_iterator i = obj.polygons.begin(); i != obj.polygons.end(); ++i)
	{
		DrawPolygon(img, *i, clr);
		// test:
		std::pair<double, Point3D> p = i->AreaAndCentroid();
		DrawLineSegment(img, LineSegment(p.second, p.second + ((i->Normal()) * 10)), RGB(0, 255, 0));
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
	if ( m_pDC ) {
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
	TCHAR szFilters[] = _T ("IRIT Data Files (*.itd)|*.itd|All Files (*.*)|*.*||");

	CFileDialog dlg(TRUE, _T("itd"), _T("*.itd"), OFN_FILEMUSTEXIST | OFN_HIDEREADONLY ,szFilters);

	if (dlg.DoModal () == IDOK) {
		m_strItdFileName = dlg.GetPathName();		// Full path and filename
		PngWrapper p;
		_models.push_back(model_t());
		CGSkelProcessIritDataFiles(m_strItdFileName, 1, _models.back());
		FlipYAxis(_models.size() - 1);
		//FitSceneToWindow();
		//delete _bbox;
		//_bbox = new BoundingBox(BoundingBox::OfObjects(_objects));
		_bboxes.push_back(BoundingBox(BoundingBox::OfObjects(_models.back())));
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

	for (int id=LIGHT_ID_1;id<MAX_LIGHT;id++)
	{	    
	    dlg.SetDialogData((LightID)id,m_lights[id]);
	}
	dlg.SetDialogData(LIGHT_ID_AMBIENT,m_ambientLight);

	if (dlg.DoModal() == IDOK) 
	{
	    for (int id=LIGHT_ID_1;id<MAX_LIGHT;id++)
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

void CCGWorkView::FitSceneToWindow()
{
	// I commented it out since it doesn't work, and I am doing something else
	/*
	const int margin = 5;
	RECT rect;
	GetWindowRect(&rect);

	int height = rect.bottom - rect.top - 2*margin;
	int width = rect.right - rect.left - 2 * margin;

	double minWindowDim = min(height, width);

	MatrixHomogeneous r = Matrices::Rotate(AXIS_X, M_PI / 8) * Matrices::Rotate(AXIS_Z, M_PI / 8);
	for (std::vector<PolygonalObject>::iterator i = _objects.begin(); i != _objects.end(); ++i)
	{
		//(*i) = r * (*i);
	}


	BoundingBox bbox = BoundingBox::OfObjects(_objects);
	double maxBBoxDim = max(bbox.maxX - bbox.minX, max(bbox.maxY - bbox.minY, bbox.maxZ - bbox.minZ));

	double resizeRatio = minWindowDim / maxBBoxDim;

	MatrixHomogeneous scale = Matrices::Scale(resizeRatio);
	MatrixHomogeneous moveToTopLeft = Matrices::Translate(-bbox.minX, -bbox.minY, -bbox.minZ);
	MatrixHomogeneous m = (scale * moveToTopLeft);
	for (std::vector<PolygonalObject>::iterator i = _objects.begin(); i != _objects.end(); ++i)
	{
		(*i) = m * (*i);
	}
	*/
}

void CCGWorkView::DrawScene(CImage& img)
{
	const int margin = 5;
	RECT rect;
	GetWindowRect(&rect);

	int height = rect.bottom - rect.top - 2 * margin;
	int width = rect.right - rect.left - 2 * margin;

	for (int i = 0; i < _models.size(); i++) {
		const MatrixHomogeneous mPersp = PerspectiveWarpMatrix(_bboxes[i].BoundingCube());
		//const MatrixHomogeneous mPersp = PerspectiveWarpMatrix(_bbox->BoundingCube());

		const MatrixHomogeneous mOrtho = OrthographicProjectMatrix(_bboxes[i].BoundingCube());
		//const MatrixHomogeneous mOrtho = OrthographicProjectMatrix(_bbox->BoundingCube());

		MatrixHomogeneous m = Matrices::Scale(min(height / 2, width / 2)) * Matrices::Translate(1, 1, 1) * mOrtho;

		model_t& model = _models[i];
		for (std::vector<PolygonalObject>::iterator i = model.begin(); i != model.end(); ++i)
		{
			DrawObject(img, m*(*i), RGB(255, 0, 0));
		}
	}
}
