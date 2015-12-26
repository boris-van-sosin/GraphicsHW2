// CGWorkView.h : interface of the CCGWorkView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_CGWORKVIEW_H__5857316D_EA60_11D5_9FD5_00D0B718E2CD__INCLUDED_)
#define AFX_CGWORKVIEW_H__5857316D_EA60_11D5_9FD5_00D0B718E2CD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "gl\gl.h"    // Include the standard CGWork  headers
#include "gl\glu.h"   // Add the utility library

#include <atlimage.h>

#include "Light.h"

#include "Geometry.h"
#include "GeometricTransformations.h"

typedef std::vector<PolygonalObject> model_t;

class model_attr_t { // don't make this struct big
public:
	COLORREF color = RGB(0, 0, 255);
	COLORREF normal_color = RGB(0, 255, 0);
	COLORREF model_bbox_color = RGB(255, 0, 0);
	COLORREF subObj_bbox_color = RGB(255, 0, 0);
	bool forceColor = false;
	unsigned int line_width = 1;
	bool displayBBox = false;
	bool displaySubObjectBBox = false;
	double sensitivity = 0.1;
};

class CCGWorkView : public CView
{
protected: // create from serialization only
	CCGWorkView();
	DECLARE_DYNCREATE(CCGWorkView)

	// Attributes
public:
	CCGWorkDoc* GetDocument();

	// Operations
public:

private:
	int m_nAxis;				// Axis of Action, X Y or Z
	int m_nAction;				// Rotate, Translate, Scale
	int m_nView;				// Orthographic, perspective
	bool m_bIsPerspective;			// is the view perspective
	bool _displayPolygonNormals, _displayVertexNormals;
	bool _dummyDisplayModelBBox, _dummyDisplaySubObjBBox;
	COLORREF _normalsColor;

	CString m_strItdFileName;		// file name of IRIT data

	int m_nLightShading;			// shading: Flat, Gouraud.

	double m_lMaterialAmbient;		// The Ambient in the scene
	double m_lMaterialDiffuse;		// The Diffuse in the scene
	double m_lMaterialSpecular;		// The Specular in the scene
	int m_nMaterialCosineFactor;		// The cosine factor for the specular

	LightParams m_lights[MAX_LIGHT];	//configurable lights array
	LightParams m_ambientLight;		//ambient light (only RGB is used)


	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCGWorkView)
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	//}}AFX_VIRTUAL

	// Implementation
public:
	virtual ~CCGWorkView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	static const COLORREF DefaultModelColor;
protected:
	BOOL InitializeCGWork();
	BOOL SetupViewingFrustum(void);
	BOOL SetupViewingOrthoConstAspect(void);

	virtual void RenderScene();


	HGLRC    m_hRC;			// holds the Rendering Context
	CDC*     m_pDC;			// holds the Device Context
	int m_WindowWidth;		// hold the windows width
	int m_WindowHeight;		// hold the windows height
	double m_AspectRatio;		// hold the fixed Aspect Ration

	// Generated message map functions
protected:
	//{{AFX_MSG(CCGWorkView)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnFileLoad();
	afx_msg void OnViewOrthographic();
	afx_msg void OnUpdateViewOrthographic(CCmdUI* pCmdUI);
	afx_msg void OnViewPerspective();
	afx_msg void OnUpdateViewPerspective(CCmdUI* pCmdUI);
	afx_msg void OnActionRotate();
	afx_msg void OnUpdateActionRotate(CCmdUI* pCmdUI);
	afx_msg void OnActionScale();
	afx_msg void OnUpdateActionScale(CCmdUI* pCmdUI);
	afx_msg void OnActionTranslate();
	afx_msg void OnUpdateActionTranslate(CCmdUI* pCmdUI);
	afx_msg void OnAxisX();
	afx_msg void OnUpdateAxisX(CCmdUI* pCmdUI);
	afx_msg void OnAxisY();
	afx_msg void OnUpdateAxisY(CCmdUI* pCmdUI);
	afx_msg void OnAxisZ();
	afx_msg void OnUpdateAxisZ(CCmdUI* pCmdUI);
	afx_msg void OnLightShadingFlat();
	afx_msg void OnUpdateLightShadingFlat(CCmdUI* pCmdUI);
	afx_msg void OnLightShadingGouraud();
	afx_msg void OnUpdateLightShadingGouraud(CCmdUI* pCmdUI);
	afx_msg void OnLightConstants();
	afx_msg void OnChangeView();
	afx_msg void OnUpdateChangeView(CCmdUI* pCmdUI);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT, short, CPoint);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDown(UINT nHitTest, CPoint point);
	afx_msg void OnTogglePolygonNormals();
	afx_msg void OnToggleVertexNormals();
	afx_msg void OnChooseColors();
	afx_msg void OnToggleModelBBox();
	afx_msg void OnToggleSubObjBBox();
	afx_msg void OnToggleAllModelBBox();
	afx_msg void OnToggleAllSubObjBBox();
	afx_msg void OnSettings();
	afx_msg void OnFileSave();

	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void FlipYAxis(int obj_idx);
	void DrawScene(CImage& img);

private:
	std::vector<model_t> _models;
	std::vector<model_t> _clean_models;
	std::vector<MatrixHomogeneous> _model_space_transformations;
	std::vector<MatrixHomogeneous> _view_space_transformations;

	std::vector<Normals::NormalList> _polygonNormals;
	std::vector<Normals::NormalList> _clean_polygonNormals;
	std::vector<Normals::NormalList> _vertexNormals;
	std::vector<Normals::NormalList> _clean_vertexNormals;
	std::vector<PolygonAdjacencyGraph> _polygonAdjacencies;

	std::vector<PolygonalObject> _modelBoundingBoxes;
	std::vector<PolygonalObject> _clean_modelBoundingBoxes;
	std::vector<model_t> _subObjectBoundingBoxes;
	std::vector<model_t> _clean_subObjectBoundingBoxes;

	std::vector<model_attr_t> _model_attr;
	std::vector<BoundingBox> _bboxes;

	COLORREF _backgroundColor;
	HBRUSH _backgroundBrush;

	int glowing_object = -1; // -1 is none
	int active_object = 0;
	bool _in_object_view = false;	// in what wview to apply the matrix? object or view?

	double _nearClippingPlane = 0.9, _farClippingPlane = 2.1;

	int _polygonFineness = 20;

	CImage _pxl2obj; // to know the position of the objects on the screen
					// there is 1 offset of the object index, because the bg is 0

	//BoundingBox* _bbox;
	bool applyMat(const MatrixHomogeneous& mat);

	void rotate(double rotate_angle);
	void translate(const Axis& axis, double dist);
	void scale(const Axis& axis, double factor);

};

#ifndef _DEBUG  // debug version in CGWorkView.cpp
inline CCGWorkDoc* CCGWorkView::GetDocument()
{
	return (CCGWorkDoc*)m_pDocument;
}
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CGWORKVIEW_H__5857316D_EA60_11D5_9FD5_00D0B718E2CD__INCLUDED_)