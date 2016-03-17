//=======================================
//	Polar Coordinate Handler
//	by PataoEngineer: 2012
//=======================================

#pragma once

#include "pImage2.h"

//=======================================

class pPolarCoord
{
public:
	pPolarCoord(void);
	pPolarCoord(CvPoint2D32f p);
	pPolarCoord(float _r, float _angle);
	~pPolarCoord(void);

	//------- Value Retrieval --------
	CvPoint2D32f ToCartesian();
	float Radius();
	float Angle();

	//------- Inputing data ----------
	void SetCenter( CvPoint2D32f centerCartesian );
	void FromCartesian( float x, float y );
	void FromPolar( float _r, float _angle );

	//------- Manipulating fns ---------
	void MoveTowardsRadius( float _r );
	void MoveTowardsAngle( float _r );


private:
	float radius, angle; // radian
	CvPoint2D32f center; // anchor point (in cartesian)
};
