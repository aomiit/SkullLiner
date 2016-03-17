//////////////////////////////////////////////////////////////////
//
//	Auto Neuron Locator : 2011
//	by Pathompong Ruangyam
//
//////////////////////////////////////////////////////////////////

#pragma once

#ifndef AUTONEURO
#define AUTONEURO

#include <String.h>
#include <stdio.h>
#include <algorithm>
#include <vector>
#include "pImage2.h"
#include "ShapeMatch.h"
#include "vtkImageReader.h"
#include "vtkImageData.h"
#include "pLeastSquareFit.h"
#include "pCurveFinder.h"

//------------------------------------------------

#define MIN(a,b)	((a>b?b:a))
#define FREE_CONTOURS(a)	vector<CvMat*>::iterator item=a.begin(); while(item!=a.end()){cvReleaseMat(&(*item));item++;}
#define EUCLIDEAN_DIST(a,b)	sqrt(pow((float)(a.x-b.x),2)+pow((float)(a.y-b.y),2))
#define BLOCK_DIST(a,b)	MIN(a.x, b.y)

bool inline arrangePointOpenTop( CvPoint2D32f p1, CvPoint2D32f p2 )
{
	if (p1.x==p2.x) return p1.y>p1.y;
	else return p1.x<p2.x;
}
bool inline arrangePointOpenBottom( CvPoint2D32f p1, CvPoint2D32f p2 )
{
	if (p1.x==p2.x) return p1.y<p1.y;
	else return p1.x<p2.x;
}


typedef struct ANCriteria
{
	unsigned int	maxNumOfPoints; // Maximum number of all generated points
	unsigned int maxIter;
	unsigned long maxMemoryAllowed; // Maximum allowed memory size
	unsigned int dimVoxel[3]; // Dimension of the voxel space
} 
_ANCriteria;


// Degree of polynomial to fit, set to degree 2 for parabola fitting.
// REMARK: For higher degree than 4, please allocate larger heap memory.
const int POLY_DEGREE	= 2;

// Template Size
const int TEMPLATE_HEIGHT	= 64;
const int TEMPLATE_WIDTH	= 64;

// Orientation list (indicating the location of the origin point)
const int ORIENT_TOPLEFT		= 0x00;
const int ORIENT_TOPRIGHT		= 0x01;
const int ORIENT_BOTTOMRIGHT	= 0x02;
const int ORIENT_BOTTOMLEFT		= 0x03;

// Threshold after smoothening image
const int POSTSMOOTHEN_THRESH = 120;

// Toggle window/level
const bool ENABLE_WINDOWLEVEL	= false;

// DEBUG: Configurations of previews
const bool DISP_THINNING_RESULT	= false;
const bool DISP_HISTOGRAM		= false;
const bool DISP_CONTOUR			= false;
const bool DISP_EXTRACTED_LINE	= false;
const bool DISP_DRAW_PARABOLA	= false;
const bool DISP_ALIGNMENT_RESULT = true;


// DEBUG: Configurations of tracing
const bool TRACE_ELEMENTS		= true;
const bool PAUSE_ON_SUCCESS		= false;
const bool PAUSE_ON_FAILURE		= false;// Must be used with DISP_EXTRACTED_LINE
const bool SAVE_RESULT_IMAGE	= false;
#define PATH_TRACE_RESULT		"X:\\CT_VOLUME\\RESULT_64x64_orig\\%03d.jpg"
static long _counter_	= 1;


/////////////////////////////////////////////////////////////////
class AutoNeuro
{
private:
	char	fileNameFormat[255]; // Formatted filename with fullpath: using %d, %s, so on to apply number running
	unsigned int		fileSeriesCount; 
	vector<CvPoint2D32f> listControlPoints;
	ANCriteria		crit; // Criteria of the operation (effective to all class methods)

	// State variables
	//-----------------------------
	unsigned long	memoryConsumed; // Aggregated memory size
	unsigned int		numIter; // Number of iteration so far
	char	strCandidatePath[255]; // Candidate file name
	int		candidateNo; // Number of the candidate (if any)
	pImage*		imgCandidate;	// Candidate image (registered)
	CvMat*	matReduced; // Reduced image for processing
	unsigned short windowLevel_min, windowLevel_max;

	// Methods & Functions
	//--------------------------------
	CvMat* sampleDownMatrix( CvMat* input, CvRect desiredSize );
	float findBestThresholdValue( CvMat* input );

protected:
	// Object detection algorithm
	//------------------------------------------
	bool readSingleInput( int stackNo, pImage* &buf  );
	bool readSingleInput( int stackNo, CvMat* &mat );
	bool readSingleInput( const char* filename, CvMat* &mat );
	bool readSingleInput( const char* filename, pImage* &buf );
	void processSingleInput( CvMat* matCand ); // Preprocessing after loading
	pImage* convertVTKImageDataToPImage( vtkImageData* dat ); // Convert a RAW image to 8-bit pImage
	CvMat* convertVTKImageDataToMatrix( vtkImageData* dat ); // Convert a RAW image to 8-bit CvMat

	// Parabolic algorithms
	//------------------------------------------
	bool refineParabolicLine( vector<CvPoint2D32f> &vec, vector<float> polyCoeff, bool openTop );
public:
	AutoNeuro(void);
	~AutoNeuro(void);
	void __fastcall clean();
	// Preparation
	//-------------------------
	bool addInputSeries( const char* _filenameFormat, unsigned int _count );
	bool setCriteria( unsigned int maxPoints, unsigned int maxIter, unsigned long maxMem, unsigned int* voxelSize );
	bool registerPotentialInputNo( int no );
	bool registerPotentialInputNo( const char* path );
	bool registerPotentialInputNo( CvMat* mat ); 
	unsigned char __fastcall convert16bitTo8bit( double v );
	// Core function
	//-------------------------
	vector<CvPoint2D32f> generateOutputCurve( int numControlPoints ); // One stop method
	vector<CvPoint2D32f> generateOutputCurve2( int numControlPoints ); // One stop method (for phaseII)
	// Additional IO
	//-------------------------
	bool exportToJpegs( const char* _filenameFormat );

};

/////////////////////////////////////////////////////////////////

#endif