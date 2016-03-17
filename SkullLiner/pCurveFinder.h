//
// Binary U-Curve Finder by PataoEngineer
//
//++++++++++++++++++++++++++++++++++++++++++++++++++
#pragma once

#include "pImage2.h"
#include "pLeastSquareFit.h"
#include "pPolyCurve.h"
#include "pPolarCoord.h"
#include "AutoNeuro.h"
#include <math.h>

//+++++++++++++++++++++++++++++++++++++++++++++++++

#ifndef PI
#define PI		acos(-1.0f)
#endif
#define CURVE_DEGREE	2	// Degree of polynomial to utilize

//------------------------------------

// Representing a casting ray on a plane
typedef struct _pRay 
{
	float	angle;
	vector<unsigned int>	projectedLine; // Starting from center (containing intensity value)
	float boundInner, boundOuter; // Inner-, Outer-bound of the projected object (radius)
}
*LPRAY;

//------------------------------------

// Designed for circular (or u-curve) detection
class pCurveOnPlane
{
private:
	bool			isDebugTraceEnabled; // Set TRUE = tracing
	CvMat*			mat; // The target canvas to inspect U-curve 
	CvMat*			canvasRay; // Canvas ray (For debugging purpose)
	
	// Exploring States
	//-------------------------
	CvPoint2D32f	anchor; // Anchor point
	float _C[CURVE_DEGREE+1]; // Coefficiences: CnXn + Cn-1Xn-1 ... C0
	std::map<int,_pRay>	listRayCasting;//[Key]=angle,[value]=ray casting value


	// Elementary curve finders
	//--------------------------
	bool __fastcall RayCastCCW(float angleStep); // Continuously cast rays on [mat]
	vector<float> __fastcall ApproximateParabolaInRange(float startAngle, float stopAngle, std::string wndCaption, int &numPoints);
	vector<CvPoint2D32f> __fastcall CollectObjectsInConstraint(vector<float> parabolaConstraints, CvRect &region );

public:
	pCurveOnPlane(CvMat* _mat, bool enableTrace=false);
	~pCurveOnPlane(void);

	// Curve finding methods
	//-----------------------
	bool FindApproxCurve(std::vector<CvPoint2D32f> &listPoints, float angleStep=PI/18);
	vector<CvPoint> listOutput;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++
