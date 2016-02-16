// CGWorkView.cpp : implementation of the CCGWorkView class
//
#include "stdafx.h"
#include "CGWork.h"

#include "CGWorkDoc.h"
#include "CGWorkView.h"

#include <iostream>
#include <algorithm>
#include <list>
#include <map>
using std::cout;
using std::endl;
#include "MaterialDlg.h"
#include "LightDialog.h"
#include "PerModel.h"

#include "Geometry.h"
#include "GeometricTransformations.h"
#include "Drawing.h"
#include "ChooseColorDlg.h"
#include "ClippingDlg.h"
#include "PerModel.h"
#include "Utils.h"

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
	ON_COMMAND(ID_BTN_INV_NORMALS, OnInverseNormals)
	ON_COMMAND(ID_TOGGLE_ALL_SUB_BBOX, OnToggleAllSubObjBBox)
	ON_COMMAND(ID_CHOOSE_COLORS, OnChooseColors)
	ON_COMMAND(ID_GENERAL_SETTINGS, OnSettings)
	ON_COMMAND(ID_PER_MODEL, OnPerModel)
	ON_COMMAND(ID_LIGHT_SHADING_FLAT, OnLightShadingFlat)
	ON_UPDATE_COMMAND_UI(ID_LIGHT_SHADING_FLAT, OnUpdateLightShadingFlat)
	ON_COMMAND(ID_LIGHT_SHADING_GOURAUD, OnLightShadingGouraud)
	ON_UPDATE_COMMAND_UI(ID_LIGHT_SHADING_GOURAUD, OnUpdateLightShadingGouraud)
	ON_COMMAND(ID_LIGHT_CONSTANTS, OnLightConstants)
	ON_COMMAND(ID_CHANGE_VIEW, OnChangeView)
	ON_UPDATE_COMMAND_UI(ID_CHANGE_VIEW, OnUpdateChangeView)
	ON_COMMAND(ID_Z_BTN, OnZBtn)
	ON_UPDATE_COMMAND_UI(ID_Z_BTN, OnUpdateZBtn)
	ON_COMMAND(ID_G_SCALE, OnGlobalScaleBtn)
	ON_UPDATE_COMMAND_UI(ID_G_SCALE, OnUpdateGlobalScaleBtn)
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

	translate_light_menu();

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
	if (m_z_buf)
		_zBufferImg.SetSize(m_WindowWidth, m_WindowHeight);
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
	if (global_scale_on) {	// override normal behaviour
		mat = Matrices::Scale(factor, factor, factor);
	}
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
	case VK_DELETE:
		deleteModel();
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

	const BoundingBox& bcube = _bboxes[active_object];
	const MatrixHomogeneous recenter = Matrices::Translate(0, 0, -(bcube.maxZ + bcube.minZ) / 2);
	const MatrixHomogeneous rerurnToPos = Matrices::Translate(0, 0, (bcube.maxZ + bcube.minZ) / 2);
	const MatrixHomogeneous mat0 = rerurnToPos * mat * recenter;
	/*
	for (auto it = _objects.begin(); it != _objects.end(); ++it) {
	(*it) = mat * (*it);
	}
	*/
	PolygonalModel& model = _models[active_object];
	const PolygonalModel& clean_model = _clean_models[active_object];
	model = clean_model;
	
	if (_in_object_view)
		_model_space_transformations[active_object] = _model_space_transformations[active_object] * mat0;
	else
		_view_space_transformations[active_object] = mat0 * _view_space_transformations[active_object];
	
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

void CCGWorkView::deleteModel()
{
	if (active_object >= _models.size())
		return;

	_models.erase(_models.begin() + active_object);
	_clean_models.erase(_clean_models.begin() + active_object);
	_model_space_transformations.erase(_model_space_transformations.begin() + active_object);
	_view_space_transformations.erase(_view_space_transformations.begin() + active_object);

	_polygonNormals.erase(_polygonNormals.begin() + active_object);
	_clean_polygonNormals.erase(_clean_polygonNormals.begin() + active_object);

	_vertexNormals.erase(_vertexNormals.begin() + active_object);
	_clean_vertexNormals.erase(_clean_vertexNormals.begin() + active_object);

	_polygonAdjacencies.erase(_polygonAdjacencies.begin() + active_object);

	_modelBoundingBoxes.erase(_modelBoundingBoxes.begin() + active_object);
	_clean_modelBoundingBoxes.erase(_clean_modelBoundingBoxes.begin() + active_object);
	
	_subObjectBoundingBoxes.erase(_subObjectBoundingBoxes.begin() + active_object);
	_clean_subObjectBoundingBoxes.erase(_clean_subObjectBoundingBoxes.begin() + active_object);

	_model_attr.erase(_model_attr.begin() + active_object);

	std::vector<BoundingBox> oldBBoxes(_bboxes.begin(), _bboxes.end());
	_bboxes.clear();
	for (auto i = oldBBoxes.begin(); i != oldBBoxes.end(); ++i)
	{
		if (i != oldBBoxes.begin() + active_object)
			_bboxes.push_back(*i);
	}

	active_object = _models.size() - 1;

	Invalidate();
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

	if (m_z_buf && (_zBufferImg.GetHeight() == 0 || _zBufferImg.GetWidth() == 0))
	{
		_zBufferImg.SetSize(w, h);
	}

	//HDC imgDC = img.GetDC();
	//FillRect(imgDC, &rect, _backgroundBrush);
	//img.ReleaseDC();

	if (_useBackgroundImage)
	{
		_zBufferImg.SetBackgroundImage(_backgrounImage, m_bgimage_mode);
	}
	else
	{
		_zBufferImg.SetBackgroundColor(_backgroundColor);
	}


	_drawObj.img = &img;
	_drawObj.zBufImg = &_zBufferImg;
	
	if (!m_z_buf)
		_drawObj.active = DrawingObject::DRAWING_OBJECT_CIMG;
	else {
		_drawObj.active = DrawingObject::DRAWING_OBJECT_ZBUF;
		_zBufferImg.Clear();
	}
		

	DrawScene(_drawObj);

	if (m_z_buf)
		_zBufferImg.DrawOnImage(img);
	

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

void CCGWorkView::ScaleAndCenterAll(PolygonalModel& model) const
{
	const BoundingBox bcube = BoundingBox::OfObjects(model).BoundingCube();
	const MatrixHomogeneous sc = Matrices::Translate(0, 0, (_initFar + _initNear) / 2) * ScaleToCube(bcube) * CenterToCube(bcube);
	for (auto i = model.begin(); i != model.end(); ++i)
	{
		for (auto j = i->polygons.begin(); j != i->polygons.end(); ++j)
		{
			std::vector<Vector3D> tmpN = j->tmpNormals;
			*j = sc * (*j);
			j->tmpNormals = tmpN;
		}
	}
}

void CCGWorkView::RemoveTmpNormals(PolygonalModel& model)
{
	for (auto i = model.begin(); i != model.end(); ++i)
	{
		for (auto j = i->polygons.begin(); j != i->polygons.end(); ++j)
		{
			j->tmpNormals.clear();
		}
	}
}

void CCGWorkView::OnFileLoad()
{
	TCHAR szFilters[] = _T("IRIT Data Files (*.itd)|*.itd|Obj Files (*.obj)|*.obj|All Files (*.*)|*.*||");

	CFileDialog dlg(TRUE, _T("itd"), _T("*.itd"), OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, szFilters);

	if (dlg.DoModal() == IDOK) {
		m_strItdFileName = dlg.GetPathName();		// Full path and filename
		PngWrapper p;
		_models.push_back(PolygonalModel());

		_model_space_transformations.push_back(Matrices::UnitMatrixHomogeneous);
		_view_space_transformations.push_back(Matrices::UnitMatrixHomogeneous);
		CGSkelProcessIritDataFiles(m_strItdFileName, 1, _models.back(), _polygonFineness);
		
		ScaleAndCenterAll(_models.back());
		_clean_models.push_back(_models.back());
		
		_polygonNormals.push_back(std::vector<Normals::PolygonNormalData>());
		_vertexNormals.push_back(Normals::NormalList());
		_polygonAdjacencies.push_back(PolygonAdjacencyGraph());
		Normals::ComputeNormals(_models.back(), _polygonNormals.back(), _vertexNormals.back(), _polygonAdjacencies.back(), _useFileNormals);

		//FlipYAxis(_models.size() - 1);

		_bboxes.push_back(BoundingBox::OfObjects(_models.back()));

		const BoundingBox bcube = _bboxes.back().BoundingCube();
		_model_attr.push_back(ModelAttr());

		double gran_size = fmax(bcube.maxX - bcube.minX, fmax(bcube.maxY - bcube.minY, bcube.maxZ - bcube.minZ)) * 10;
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

inline void InverseNormal(LineSegment& n)
{
	const Point3D p0(n.p0), p1(n.p1);
	n.p1 = HomogeneousPoint(p0 - (p1 - p0));
}

void CCGWorkView::InverseNormals()
{
	if (active_object < 0 || active_object >= _models.size())
	{
		return;
	}

	for (size_t j = 0; j < _clean_polygonNormals[active_object].size(); ++j)
	{
		InverseNormal(_clean_polygonNormals[active_object][j].PolygonNormal);
		for (auto k = _clean_polygonNormals[active_object][j].VertexNormals.begin(); k != _clean_polygonNormals[active_object][j].VertexNormals.end(); ++k)
		{
			InverseNormal(*k);
		}
	}
	for (auto h = _clean_vertexNormals[active_object].begin(); h != _clean_vertexNormals[active_object].end(); ++h)
	{
		InverseNormal(*h);
	}
	applyMat(Matrices::UnitMatrixHomogeneous);
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

void CCGWorkView::OnZBtn()
{
	if (m_z_buf)
		m_z_buf = false;
	else
	{
		m_z_buf = true;
		RECT rect;
		GetClientRect(&rect);

		int h = rect.bottom - rect.top;
		int w = rect.right - rect.left;
		_zBufferImg.SetSize(w, h);
	}
	Invalidate();
}

void CCGWorkView::OnUpdateChangeView(CCmdUI* pCmdUI)
{
	if (_in_object_view)
		pCmdUI->SetCheck(1);
	else
		pCmdUI->SetCheck(0);
}

void CCGWorkView::OnUpdateZBtn(CCmdUI* pCmdUI)
{
	if (m_z_buf)
		pCmdUI->SetCheck(1);
	else
		pCmdUI->SetCheck(0);
}

void CCGWorkView::OnGlobalScaleBtn()
{
	if (global_scale_on)
		global_scale_on = false;
	else
		global_scale_on = true;
}

void CCGWorkView::OnUpdateGlobalScaleBtn(CCmdUI* pCmdUI)
{
	if (global_scale_on)
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

	param.bg_image_mode = m_bgimage_mode;

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

		m_bgimage_mode = param.bg_image_mode;

		if (_useBackgroundImage = (!param.background_image.IsNull()))
		{
			CopyImage(param.background_image, _backgrounImage);
		}

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

void CCGWorkView::OnInverseNormals()
{
	InverseNormals();
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
	s.load_normals_from_file = _useFileNormals;
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
		_useFileNormals = s.load_normals_from_file;
		Invalidate();
	}
}

void CCGWorkView::OnPerModel()
{
	CPerModelParam s;
	if (active_object >= _model_attr.size())
		return;
	ModelAttr& model = _model_attr[active_object];
	s.removeBackFace = model.removeBackFace;
	s.AmbientCoefficient = model.AmbientCoefficient;
	s.DiffuseCoefficient = model.DiffuseCoefficient;
	s.SpecularCoefficient = model.SpecularCoefficient;
	s.SpecularPower = model.SpecularPower;
	s.Shading = model.Shading;
	s.silluete = model.silluete;
	s.boundry = model.boundry;
	s.AmbientIntensity = model.AmbientIntensity;
	s.is_wireframe = model.is_wireframe;

	CPerModel dlg(s);
	if (dlg.DoModal() == IDOK)
	{
		model.removeBackFace = s.removeBackFace ? BACKFACE_REMOVE_BACK : BACKFACE_SHOW;
		model.AmbientCoefficient = s.AmbientCoefficient;
		model.DiffuseCoefficient = s.DiffuseCoefficient;
		model.SpecularCoefficient = s.SpecularCoefficient;
		model.SpecularPower = s.SpecularPower;
		model.Shading = s.Shading;
		model.silluete = s.silluete;
		model.boundry = s.boundry;
		model.AmbientIntensity = s.AmbientIntensity;
		model.is_wireframe = s.is_wireframe;
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

void CCGWorkView::translate_light_menu() {
	g_lights.clear();
	g_ShadowVolumes.clear();
	g_lights.reserve(MAX_LIGHT);
	g_ShadowVolumes.reserve(MAX_LIGHT);

	RECT rect;
	GetClientRect(&rect);

	int h = rect.bottom - rect.top;
	int w = rect.right - rect.left;

	for (int id = LIGHT_ID_1; id < MAX_LIGHT; id++) {
		if (!m_lights[id].enabled) {
			continue;
		}
		LightSource new_light;
		switch (m_lights[id].type) {
		case LIGHT_TYPE_POINT:
			new_light._type = LightSource::LightSourceType::POINT;
			new_light._origin = HomogeneousPoint(m_lights[id].posX, m_lights[id].posY, m_lights[id].posZ);
			break;
		case LIGHT_TYPE_DIRECTIONAL:
		case LIGHT_TYPE_SPOT:
		default:
			new_light._type = LightSource::LightSourceType::PLANE;
			if (m_lights[id].dirX != 0 || m_lights[id].dirY != 0 || m_lights[id].dirZ != 0) {
				new_light._origin = HomogeneousPoint(0, 0, 0);
				new_light._offset = HomogeneousPoint(m_lights[id].dirX, m_lights[id].dirY, m_lights[id].dirZ);
			}
			break;
		}

		new_light._intensity[0] = (double)m_lights[id].colorR / (double)255;
		new_light._intensity[1] = (double)m_lights[id].colorG / (double)255;
		new_light._intensity[2] = (double)m_lights[id].colorB / (double)255;

		g_lights.push_back(new_light);
		g_ShadowVolumes.push_back(ShadowVolume(w, h, new_light));
	}
}

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

	// Translating to our structures
	translate_light_menu();
	// Translating to our structures: end
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

	for (auto svIt = g_ShadowVolumes.begin(); svIt != g_ShadowVolumes.end(); ++svIt)
	{
		if (svIt->GetHeight() != img.GetHeight() || svIt->GetHeight() != img.GetWidth())
		{
			svIt->SetSize(img.GetWidth(), img.GetHeight());
			svIt->SetLightSource(g_lights[svIt - g_ShadowVolumes.begin()]);
		}
		svIt->Clear();
	}

	for (size_t i = 0; i < _models.size(); i++) {
		const BoundingBox bCube = _bboxes[i].BoundingCube();
		const PerspectiveData perspData = PerspectiveWarpMatrix(bCube, _nearClippingPlane, _farClippingPlane, min(width, height));

		const COLORREF normalsColor = _model_attr[i].normal_color;

		/*MatrixHomogeneous mMoveToView = 
			(m_bIsPerspective ?
			(Matrices::Flip(AXIS_X)*perspData.ScaleAndMoveToView) :
			(Matrices::Flip(AXIS_Y)*ScaleAndCenter(bCube)));*/

		MatrixHomogeneous mProj = 
			(m_bIsPerspective ?
			perspData.PerspectiveWarp : Matrices::UnitMatrixHomogeneous);

		const BoundingBox displayBox = (mProj * _bboxes[i]).BoundingCube();

		const double scalingFactor = m_bIsPerspective ? 1 :
			(0.25 * min(width, height) / ((displayBox.maxX - displayBox.minX)));

		const MatrixHomogeneous mScale = Matrices::Translate(width*0.5, height*0.5, 0) * Matrices::Scale(scalingFactor);

		const MatrixHomogeneous mTotal = mScale * mProj * (m_bIsPerspective ?
			(Matrices::Flip(AXIS_X)) :
			(Matrices::Flip(AXIS_Y)));

		PolygonalModel& model = _models[i];
		const ModelAttr attr = _model_attr[i];
		ModelAttr shadow_attr = attr;
		DrawingObject tmpDrawingObj;
		tmpDrawingObj.img = &_pxl2obj;
		tmpDrawingObj.active = DrawingObject::DRAWING_OBJECT_CIMG;

		//
		// shadow volume test
		// this displays the shadow geometry for the first light source
		if (attr.shadowVolumeWireframe >= 0 && attr.shadowVolumeWireframe < g_lights.size())
		{
			ShadowVolume sv(1, 1, g_lights[attr.shadowVolumeWireframe]);
			const std::pair<PolygonalObject, std::vector<Normals::PolygonNormalData>> svs = sv.GenerateShadowVolume(model, mTotal, _polygonNormals[i], _polygonAdjacencies[i]);
			ModelAttr svAttr;
			svAttr.color = RGB(50, 50, 50);
			svAttr.forceColor = true;
			svAttr.Shading = SHADING_NONE;
			svAttr.castShadow = false;
			svAttr.removeBackFace = BACKFACE_SHOW;
			DrawObject(img, svs.first, mTotal, svAttr, svs.second, 0, false, m_bIsPerspective, perspData.NearPlane);
			/*for (auto j = svs.second.begin(); j != svs.second.end(); ++j)
			{
				DrawLineSegment(img, TransformNormal(mTotal, j->PolygonNormal, 5), RGB(255, 255, 255), 2);
				DrawLineSegment(img, TransformNormal(mTotal, j->PolygonNormal, 10), RGB(255, 255, 0), 1);
			}*/
		}
		//

		if (attr.castShadow && img.active == DrawingObject::DRAWING_OBJECT_ZBUF)
		{
			for (auto svIt = g_ShadowVolumes.begin(); svIt != g_ShadowVolumes.end(); ++svIt)
			{
				svIt->ProcessModel(model, mTotal, _polygonNormals[i], m_bIsPerspective, perspData.NearPlane, _polygonAdjacencies[i]);
			}
		}

		size_t normalsIdx = 0;
		for (std::vector<PolygonalObject>::iterator it = model.begin(); it != model.end(); ++it)
		{
			DrawObject(img, *it, mTotal, attr, _polygonNormals[i], normalsIdx, !attr.is_wireframe, m_bIsPerspective, perspData.NearPlane);
			if (attr.line_width > 1)
			{
				ModelAttr widenAttr = attr;
				widenAttr.Shading = SHADING_NONE;
				DrawObject(img, *it, mTotal, widenAttr, _polygonNormals[i], normalsIdx, false, m_bIsPerspective, perspData.NearPlane);
			}
			shadow_attr.color = i + 1;
			shadow_attr.forceColor = true;
			shadow_attr.Shading = SHADING_NONE;
			DrawObject(tmpDrawingObj, *it, mTotal, shadow_attr, _polygonNormals[i], normalsIdx, false, m_bIsPerspective, perspData.NearPlane);
			normalsIdx += it->polygons.size();
		}

		if (attr.boundry || attr.silluete)
		{
			for (auto j = _polygonAdjacencies[i].begin(); j != _polygonAdjacencies[i].end(); ++j)
			{
				const Polygon3D& currPoly = _models[i][j->objIdx].polygons[j->polygonInObjIdx];
				const LineSegment edge(currPoly.points[j->vertexIdx], currPoly.points[(j->vertexIdx+1) % currPoly.points.size()]);
				COLORREF boundaryColor = ShiftColor(GetActualColor(_models[i][j->objIdx].color, _models[i][j->objIdx].colorValid, currPoly, edge.p0, attr), -100);
				if (attr.boundry && (j->polygonIdxs.size() == 1))
				{
					// boundary
					if (m_bIsPerspective)
					{
						DrawLineSegment(img, mTotal * ApplyClipping(edge.p0, edge.p1, perspData.NearPlane).lineSegment, boundaryColor, 1);
					}
					else
					{
						DrawLineSegment(img, mTotal*(edge.p0), mTotal*(edge.p1), boundaryColor, 1);
					}
				}
				else if (attr.silluete)
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
							DrawLineSegment(img, mTotal * ApplyClipping(edge.p0, edge.p1, perspData.NearPlane).lineSegment, boundaryColor, 1);
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
			ModelAttr bboxAttr;
			bboxAttr.color = _model_attr[i].model_bbox_color;
			bboxAttr.forceColor = true;
			DrawObject(img, _modelBoundingBoxes[i], mTotal, bboxAttr, _polygonNormals[i], 0, false, m_bIsPerspective, perspData.NearPlane);
		}
		if (attr.displaySubObjectBBox)
		{
			ModelAttr bboxAttr;
			bboxAttr.color = _model_attr[i].subObj_bbox_color;
			bboxAttr.forceColor = true;
			for (auto j = _subObjectBoundingBoxes[i].begin(); j != _subObjectBoundingBoxes[i].end(); ++j)
			{
				DrawObject(img, *j, mTotal, bboxAttr, _polygonNormals[i], false, m_bIsPerspective, 0, perspData.NearPlane);
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
		const int h = 2000, w = 2000;
		img.Create(w, h, 32);

		RECT r;
		r.top = r.left = 0;
		r.bottom = h; r.right = w;

		HDC imgDC = img.GetDC();
		FillRect(imgDC, &r, _backgroundBrush);
		img.ReleaseDC();

		DrawingObject tmpDrawingObj;
		ZBufferImage zbimg(w, h);
		tmpDrawingObj.zBufImg = &zbimg;
		tmpDrawingObj.active = DrawingObject::DRAWING_OBJECT_ZBUF;
		if (_useBackgroundImage)
		{
			//zbimg.SetBackgroundImage(_backgrounImage, ZBufferImage::REPEAT);
			zbimg.SetBackgroundImage(_backgrounImage, m_bgimage_mode);
		}
		else
		{
			zbimg.SetBackgroundColor(_backgroundColor);
		}

		DrawScene(tmpDrawingObj);

		zbimg.DrawOnImage(img);

		img.Save(fileName, Gdiplus::ImageFormatPNG);
		img.Destroy();
	}
}
