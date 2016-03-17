#include "pPolarCoord.h"

pPolarCoord::pPolarCoord(void)
{
	this->angle = 0;
	this->center = cvPoint2D32f(0,0);
	this->radius = 0;
}

pPolarCoord::pPolarCoord(CvPoint2D32f p)
{
	this->center = cvPoint2D32f(0,0);
	this->FromCartesian( p.x, p.y );
}

void pPolarCoord::SetCenter( CvPoint2D32f centerCartesian )
{
	this->center = centerCartesian;
}

pPolarCoord::pPolarCoord(float _r, float _angle)
{
	this->center = cvPoint2D32f(0,0);
	this->radius = _r;
	this->angle = _angle;
}

pPolarCoord::~pPolarCoord(void)
{
}

CvPoint2D32f pPolarCoord::ToCartesian()
{
	return cvPoint2D32f(
		this->radius * cos(this->angle),
		this->radius * sin(this->angle)
		);
}

float pPolarCoord::Radius()
{
	return this->radius;
}

float pPolarCoord::Angle()
{
	return this->angle;
}

void pPolarCoord::FromCartesian( float x, float y )
{
	// ASSUMPTION: The cartesian center point has been already set
	float dx = x-this->center.x;
	float dy = y-this->center.y;
	this->radius = sqrt(
		pow(dx,2.f) + pow(dy,2.f)
		);
	this->angle = atan2(dy,dx);
}

void pPolarCoord::FromPolar( float _r, float _angle )
{
	this->angle = _angle;
	this->radius = _r;
}

void pPolarCoord::MoveTowardsRadius( float _r )
{
	this->radius += _r;
}

void pPolarCoord::MoveTowardsAngle( float _a )
{
	this->angle += _a;
}


