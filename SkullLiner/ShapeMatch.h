//////////////////////////////////////////////////////////////////
//
//	Shape Matching Toolkit : 2011 - Last Update 2012
//	by Pathompong Ruangyam
//
//////////////////////////////////////////////////////////////////

#pragma once

#include <String.h>
#include <stdio.h>
#include <algorithm>
#include "pImage2.h"

//------------------------------------------------------------------------------------------

class ShapeMatch
{
protected:
	vector<CvPoint2D32f>	listPoints;
	vector<float> listScores;
	float score;
	float virtual calculateSinglePointScore( int i, int j, CvSize size );
	float constraintTop, constraintBottom;
private:
public:
	ShapeMatch(void);
	~ShapeMatch(void);
	void init();
	void finalize();
	bool fromMatrix( CvMat* mat );
	float getScore();
};

//------------------------------------------------------------------------------------------

class UShape : public ShapeMatch
{
private:
	float virtual calculateSinglePointScore( int i, int j, CvSize size );
public:
	UShape(void);
	~UShape(void);
	vector<float> getScores();
	int numPoints();
	void getVerticalConstraint( float &yTop, float &yBottom );
};

