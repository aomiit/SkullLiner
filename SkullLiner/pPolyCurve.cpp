#include "stdafx.h"
#include "pPolyCurve.h"

pPolyCurve::pPolyCurve(int initialDegree)
{
	for (register int i=0; i<initialDegree; i++)
		this->_c.push_back(0);
}

pPolyCurve::~pPolyCurve(void)
{
}

pPolyCurve pPolyCurve::operator = (pPolyCurve& a)
{
	if (this!=&a)
	{
		copy(a,*this);
	}
	return *this;
}

void copy(pPolyCurve &src, pPolyCurve &dest)
{
	dest._c = src._c;
}

pPolyCurve pPolyCurve::operator * (float v)
{
	vector<float>::iterator elem = this->_c.begin();
	while (elem!=this->_c.end())
	{
		(*elem) *= v;
		elem++;
	}
	return *this;
}

pPolyCurve pPolyCurve::operator / (float v)
{
	vector<float>::iterator elem = this->_c.begin();
	while (elem!=this->_c.end())
	{
		(*elem) /= v;
		elem++;
	}
	return *this;
}

pPolyCurve pPolyCurve::operator + (float v)
{
	vector<float>::iterator elem = this->_c.begin();
	while (elem!=this->_c.end())
	{
		(*elem) += v;
		elem++;
	}
	return *this;
}

pPolyCurve pPolyCurve::operator - (float v)
{
	vector<float>::iterator elem = this->_c.begin();
	while (elem!=this->_c.end())
	{
		(*elem) -= v;
		elem++;
	}
	return *this;
}

float& pPolyCurve::operator[] (const unsigned int i)
{
	return this->_c[i];
}
