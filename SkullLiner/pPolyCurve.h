//
// Curve Representator by PataoEngineer
//
//++++++++++++++++++++++++++++++++++++++++++++++++++
#pragma once

#include "pImage2.h"

//++++++++++++++++++++++++++++++++++++++++++++++++++

class pPolyCurve
{
private:
	std::vector<float>	_c; // Coefficiences C0 + C1X + C2X2 + ... cNxN
public:
	pPolyCurve(int initialDegree);
	~pPolyCurve(void);

	pPolyCurve operator = (pPolyCurve&);
	pPolyCurve operator * (float);
	pPolyCurve operator / (float);
	pPolyCurve operator + (float);
	pPolyCurve operator - (float);
	friend void copy(pPolyCurve &src, pPolyCurve &dest);

	float& operator[] (const unsigned int i); // Accessing coefficience
};

