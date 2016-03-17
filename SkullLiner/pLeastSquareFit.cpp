#include "StdAfx.h"
#include "pLeastSquareFit.h"

pLeastSquareFit::pLeastSquareFit(void)
{
}

pLeastSquareFit::~pLeastSquareFit(void)
{
}

float* pLeastSquareFit::fitParabola( vector<CvPoint2D32f> points )
{
	// Result in form of y = a + bx + cx^2

	// As Y[1x3] = C[1x3] x A[3x3]
	// We solve for C = Y(invertA)
	// Refer to http://www.efunda.com/math/leastsquares/lstsqr2dcurve.cfm for further information
	// 
	// Let C be the solution vector [a,b,c]
	//	A be the square matrix of the equation system described in the prementioned link
	//	Y be the target function vector
	CvMat* matY, *matA, *matC;
	
	matY = cvCreateMat( 1, 3, CV_32FC1 );
	matA = cvCreateMat( 3, 3, CV_32FC1 );

	cvZero( matY );
	cvZero( matA );		
	for (register int i=0; i<points.size(); i++)
	{
		float x = points[i].x;
		float y = points[i].y;
		float xy = x*y;
		float x2y = xy*x;
		float x2 = x*x;

		cvSetReal1D( matY, 0, cvGetReal2D(matY, 0, 0)+y );
		cvSetReal1D( matY, 1, cvGetReal2D(matY, 0, 1)+xy );
		cvSetReal1D( matY, 2, cvGetReal2D(matY, 0, 2)+x2y );

		cvSetReal2D( matA, 0, 0, cvGetReal2D(matA, 0, 0)+1 );
		cvSetReal2D( matA, 0, 1, cvGetReal2D(matA, 0, 1)+x );
		cvSetReal2D( matA, 0, 2, cvGetReal2D(matA, 0, 2)+x2 );
		cvSetReal2D( matA, 1, 0, cvGetReal2D(matA, 1, 0)+x );
		cvSetReal2D( matA, 1, 1, cvGetReal2D(matA, 1, 1)+x2 );
		cvSetReal2D( matA, 1, 2, cvGetReal2D(matA, 1, 2)+x2*x );
		cvSetReal2D( matA, 2, 0, cvGetReal2D(matA, 2, 0)+x2 );
		cvSetReal2D( matA, 2, 1, cvGetReal2D(matA, 2, 1)+x2*x );
		cvSetReal2D( matA, 2, 2, cvGetReal2D(matA, 2, 2)+x2*x2 );
	}

	// Find the solution C
	matC = pLeastSquareFit::solve( matA, matY );

	float* solution = new float[3];
	solution[0] = cvGetReal1D( matC, 0 );
	solution[1] = cvGetReal1D( matC, 1 );
	solution[2] = cvGetReal1D( matC, 2 );
	cvReleaseMat( &matC );
	cvReleaseMat( &matA );
	cvReleaseMat( &matY );

	return solution;
}

float* pLeastSquareFit::fitPolynomial( vector<CvPoint2D32f> points, unsigned int degree )
{
	// Solve for specific degree of polynomial
	// Results in form of y = a[0] + a[1]x + a[2]x^2 + ... + a[degree]x^degree

	// Refer to http://www.efunda.com/math/leastsquares/lstsqrmdcurve.cfm for equations.

	CvMat* matY, *matA, *matC;
	
	matY = cvCreateMat( 1, degree+1, CV_32FC1 );
	matA = cvCreateMat( degree+1, degree+1, CV_32FC1 );

	cvZero( matY );
	cvZero( matA );		
	for (register int i=0; i<points.size(); i++)
	{
		float x = points[i].x;
		float y = points[i].y;

		for (register int n=0; n<=degree; n++)
		{
			cvSetReal1D( matY, n, cvGetReal1D(matY, n)+pow(x,n)*y );
			for (register int m=0; m<=degree; m++)
			{
				cvSetReal2D( matA, n, m, cvGetReal2D(matA, n, m)+pow(x,n+m) );
			}
		}
	}

	// Find the solution C
	matC = pLeastSquareFit::solve( matA, matY );

	float* solution = new float[degree+1];
	for (register int i=0; i<=degree; i++)
		solution[i] = cvGetReal1D( matC, i );

	cvReleaseMat( &matC );
	cvReleaseMat( &matA );
	cvReleaseMat( &matY );

	return solution;
}


CvMat* pLeastSquareFit::solve( CvMat* A, CvMat* Y )
{
	// Problem form:	C = Y x inv(A)
	// We solve for C = Y(invertA)
	// 
	int row = cvGetSize(Y).height, col = cvGetSize(A).width;
	CvMat* matC = cvCreateMat( row, col, CV_32FC1 );

	// Solve the matrices for the unique solution C
	cvInvert( A, A ); 
	cvMatMul( Y, A, matC );
		
	// Now we get the parabolic parameters C = [a,b,c]
	float parab_a = cvGetReal1D( matC, 0 );
	float parab_b = cvGetReal1D( matC, 1 );
	float parab_c = cvGetReal1D( matC, 2 );
	return matC;
}