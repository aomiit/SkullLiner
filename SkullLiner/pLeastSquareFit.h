//++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Least Square Fitting by PataoEngineer
//
//++++++++++++++++++++++++++++++++++++++++++++++++++
#ifndef LSF_P
#define LSF_P

#include <algorithm>
#include <vector>
#include "pImage2.h"

using namespace std;

#pragma once

static class pLeastSquareFit
{
public:
	pLeastSquareFit(void);
	~pLeastSquareFit(void);
	static float* fitParabola( vector<CvPoint2D32f> points );
	static float* fitPolynomial( vector<CvPoint2D32f> points, unsigned int degree );

protected:
	// Quadratic matrix equations solver using matrix inversion
	static CvMat* solve( CvMat* A, CvMat* Y );

};

#endif
