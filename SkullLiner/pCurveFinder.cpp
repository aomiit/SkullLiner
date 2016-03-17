#include "stdafx.h"
#include "pCurveFinder.h"

pCurveOnPlane::pCurveOnPlane(CvMat* _mat, bool enableTrace)
{
	if (_mat!=NULL)
	{
		// Define an anchor point
		this->mat = cvCloneMat(_mat);
		CvSize size = cvGetSize(this->mat);
		this->anchor = cvPoint2D32f( size.width/2, size.height/2 );

		if (_mat!=NULL)
		{
			this->canvasRay = cvCloneMat(_mat);
			cvZero( this->canvasRay );
		}
		else
			this->canvasRay = NULL;
		
	}
	else
		this->mat = NULL;
	this->isDebugTraceEnabled = enableTrace;
}

pCurveOnPlane::~pCurveOnPlane(void)
{
	try
	{
		if (this->mat!=NULL)
		{
			cvReleaseMat(&this->mat);
			this->mat = NULL;
		}
		if (this->canvasRay!=NULL)
		{
			cvReleaseMat(&this->canvasRay);
			this->canvasRay = NULL;
		}
	}
	catch (...)
	{
	}
}

bool pCurveOnPlane::RayCastCCW(float angleStep)
{
	if (angleStep<=0 || angleStep>=PI/2) 
		return false;

	// Iteratively cast ray from the anchor point,
	// counterclockwise
	//============================
	this->listRayCasting.clear();

	// Each angle to trace a ray
	int n=0;
	for (float angle=0; angle<PI*2; angle+=angleStep) 
	{
		_pRay ray;
		ray.angle = angle;
		ray.boundInner = 0;
		ray.boundOuter = 0;

		// Calculate a directional vector from anchor at a specified angle
		float vec_x = cos(angle);
		float vec_y = sin(angle);

		// Normalize the vector (making the least component to be 1)
		float least = min(abs(vec_x),abs(vec_y));
		if (least>0.0f) // Beware of division by zero
		{
			vec_x /= least;
			vec_y /= least;
		}
		else if (angle == 0)
		{
			vec_x = 1; vec_y = 0;
		}
		else if (angle == PI*0.5)
		{
			vec_x = 0; vec_y = 1;
		}
		else if (angle == PI)
		{
			vec_x = -1; vec_y = 0;
		}
		else if (angle == PI*1.5)
		{
			vec_x = 0; vec_y = -1;
		}

	
		// Each step from the anchor point
		CvPoint2D32f p = this->anchor;
		CvSize size = cvGetSize(this->mat);

		int prevValue = 0;
		while (p.x>0 && p.x<size.width && p.y>0 && p.y<size.height)
		{
			int x = (int)floor(p.x);
			int y = (int)floor(p.y);
			int value = (unsigned int)floor(cvGetReal2D(
					this->mat, 
					y, x 
					));
			ray.projectedLine.push_back(value);

			// When object detected, decide whether it is a bound
			if (value>0 && ray.boundInner==0)
			{
				// Inner bound edge found
				float radius = sqrt(pow(y-anchor.y,2.f) + pow(x-anchor.x,2.f));
				ray.boundInner = radius; 
			}
			else if (value==0 && ray.boundInner>0 && prevValue>0 /*&& ray.boundOuter==0*/)
			{
				// Outer bound edge found
				float radius = sqrt(pow(y-anchor.y,2.f) + pow(x-anchor.x,2.f));
				ray.boundOuter = radius;
			}

			// Crease on the canvas
			cvSetReal2D( this->canvasRay, y, x, value );

			// Next pixel
			p.x += vec_x;
			p.y += vec_y;
			prevValue = value;
		}

		// Add the casted line to the list
		if (ray.projectedLine.size()>1)
		{
			this->listRayCasting.insert(
				std::pair<int,_pRay>(n, ray)
				);
			n++;
		}

	} // For each angle step
	return true;
}

vector<float> __fastcall pCurveOnPlane::ApproximateParabolaInRange(float startAngle, float stopAngle, std::string wndCaption, int &numPoints)
{
	// Take a regression on the ray-casted curve to extract
	// the parabolic parameters
	// REMARK: Starting angle, Stop angle must lies counterclockwise
	//
	// Return: a1,b1,c1,a2,b2,c2
	//			Where y=a+bx+cx^2 and [1] denotes inner parabola, [2] denotes outer one

	vector<CvPoint2D32f> listPointsInnerBound;
	vector<CvPoint2D32f> listPointsOuterBound;

	int numInner = 0, numOuter = 0; // Number of points lying within the range

	// Start scanning
	for (std::map<int,_pRay>::iterator rayItem=this->listRayCasting.begin(); 
		rayItem != this->listRayCasting.end();
		rayItem++)
	{
		// Angle range checking
		_pRay ray = (*rayItem).second;

		// Case 1: The constraints range between Q1 and Q4
		if (stopAngle<startAngle)
		{
			if (ray.angle>stopAngle && ray.angle<startAngle)
				continue;
		}
		else // Case 2: Otherwise
		{
			if (ray.angle<startAngle || ray.angle>stopAngle)
				continue;
		}
		
		// Skip the undefined points
		if (ray.boundInner==0 || ray.boundOuter==0)
			continue;

		// Now all unwanted elements are filtered out

		// Start gathering all points
		numInner ++;
		numOuter ++;

		listPointsInnerBound.push_back( cvPoint2D32f(
			this->anchor.x + ray.boundInner*cos(ray.angle),
			this->anchor.y + ray.boundInner*sin(ray.angle)
			)
			);
		listPointsOuterBound.push_back( cvPoint2D32f(
			this->anchor.x + ray.boundOuter*cos(ray.angle),
			this->anchor.y + ray.boundOuter*sin(ray.angle)
			)
			);
	} // For each element

	// Ignore this process if the number of bound points are too small
	if (listPointsInnerBound.size()<20 || listPointsOuterBound.size()<20)
		return vector<float>();

	// Return the number of the points as the smallest number
	numPoints = min(numInner, numOuter);

	// Start to calculate the parabolic fitting on both
	// inner bound and outer bound sparse points
	float* innerCoef = pLeastSquareFit::fitParabola(
		listPointsInnerBound
		);
	float* outerCoef = pLeastSquareFit::fitParabola(
		listPointsOuterBound
		);

	// Don't proceed if the parabola is impossible
	//-------------------------------------------
	// Case #1 > Upside-down parabola on Q3,Q4
	// Case #2 > Parabola on Q1,Q2
	// Case #3 > Convexities of Inner and outer curves are different
	if (innerCoef[2]>0 && startAngle<PI && stopAngle<PI)
		return vector<float>();
	if (innerCoef[2]<0 && startAngle>PI && stopAngle>PI)
		return vector<float>();
	if (innerCoef[2]/abs(innerCoef[2]) != outerCoef[2]/abs(outerCoef[2]))
		return vector<float>();

	// Display the trace if enabled
	if (this->isDebugTraceEnabled)
	{
		CvSize sizeMat = cvGetSize(this->mat);
		pImage* pMat = new pImage(this->mat);
		pImage* pTrace = new pImage(400,400);
		
		float ratioX = 400/(float)sizeMat.width;
		float ratioY = 400/(float)sizeMat.height;
		cvResize( pMat->image, pTrace->image );

		cvReleaseImage( &pMat->image );
		delete pMat;

		//  Draw the approximated parabolas
		//-----------------------------------
		for (float _x=-this->anchor.x; _x<pTrace->image->width; _x+=0.01)
		{
			float x = _x; 
			float y_inner = 0;
			float y_outer = 0;
			for (register int k=0; k<=2; k++)
			{
				y_inner += pow(x,(float)k)*innerCoef[k];
				y_outer += pow(x,(float)k)*outerCoef[k];
			}

			// Shift the center point (as the image has been resized)
			x *= ratioX;
			y_inner *= ratioY;
			y_outer *= ratioY;


			if (y_inner>=0 && y_inner<pTrace->image->height)
				pTrace->DrawCircle( cvPoint(x,y_inner), 1, CV_RGB(255,200,0), true, 1 );
			if (y_outer>=0 && y_outer<pTrace->image->height)
				pTrace->DrawCircle( cvPoint(x,y_outer), 1, CV_RGB(200,150,0), true, 1 );
		}


		// Crease the angle range
		//-------------------------------------
		// Find the terminal points to crease
		for (int i=0; i<listPointsInnerBound.size(); i++)
		{
			pTrace->DrawCircle( 
				cvPoint(listPointsInnerBound[i].x*ratioX, listPointsInnerBound[i].y*ratioY),
				3,
				CV_RGB(0,240,30),
				true
				);
			pTrace->DrawCircle( 
				cvPoint(listPointsOuterBound[i].x*ratioX, listPointsOuterBound[i].y*ratioY),
				3,
				CV_RGB(0,40,230),
				true
				);
			cvLine(
				pTrace->image,
				cvPoint(listPointsInnerBound[i].x*ratioX, listPointsInnerBound[i].y*ratioY),
				cvPoint(listPointsOuterBound[i].x*ratioX, listPointsOuterBound[i].y*ratioY),
				CV_RGB(100,180,220),
				1,
				8
				);			
				
		}

		pTrace->DrawCircle(
			cvPoint(anchor.x*ratioX, anchor.y*ratioY),
			4,
			CV_RGB(25,10,250),
			true,
			2
			);

		// Draw parabolic parameters
		char lblParabInner[80];
		char lblParabOuter[80];
		char lblOverall[40];

		sprintf( lblParabInner, "INNER [%0.1f, %0.1f, %0.1f] %d points", innerCoef[0], innerCoef[1], innerCoef[2], numInner);
		sprintf( lblParabOuter, "OUTER [%0.1f, %0.1f, %0.1f] %d points", outerCoef[0], outerCoef[1], outerCoef[2], numOuter);
		sprintf( lblOverall, "ALL POINTS %d", this->listRayCasting.size() );

		pTrace->DrawCaption( 
			cvPoint(8,18),
			0.5,
			CV_RGB(255,200,0),
			lblParabInner
			);
		pTrace->DrawCaption( 
			cvPoint(8,31),
			0.5,
			CV_RGB(255,200,0),
			lblParabOuter
			);
		pTrace->DrawCaption(
			cvPoint(8,48),
			0.5,
			CV_RGB(255,200,0),
			lblOverall
			);

		cvNamedWindow(wndCaption.c_str());
		pTrace->ShowImage((char*)wndCaption.c_str());

		cvReleaseImage(&pTrace->image);
		delete pTrace;
	
	} // if DEBUG

	vector<float> listCoefReturn;
	for (register int i=0; i<=2; i++)
		listCoefReturn.push_back(innerCoef[i]);
	for (register int i=0; i<=2; i++)
		listCoefReturn.push_back(outerCoef[i]);


	// Clean up the memory blocks
	delete[] innerCoef;
	delete[] outerCoef;

	return listCoefReturn;
}


bool pCurveOnPlane::FindApproxCurve(std::vector<CvPoint2D32f> &listPoints, float angleStep)
{
	// Raycasting counterclockwise to get a vector of
	// projected lines of the specified image plane
	if (!this->RayCastCCW(angleStep))
		return false;

	// --- TRACE: ---------
	if (this->isDebugTraceEnabled)
	{
		pImage* pSrc = new pImage(this->mat);

		// DEBUG: Display the ray casted result
		//=====================================
		int size = cvGetSize(this->mat).height*2;
		unsigned numRays = this->listRayCasting.size();

		int i=0;
		for (std::map<int,_pRay>::iterator ray=this->listRayCasting.begin();
			ray!=this->listRayCasting.end();
			ray++)
		{
			_pRay _ray = (*ray).second;
			vector<unsigned int>::iterator castElem = _ray.projectedLine.begin();
			int j=0;

			// Crease the bound points
			CvPoint2D32f pointInnerBound = cvPoint2D32f(
				anchor.x + _ray.boundInner * cos(_ray.angle),
				anchor.y + _ray.boundInner * sin(_ray.angle)
				);
			CvPoint2D32f pointOuterBound = cvPoint2D32f(
				anchor.x + _ray.boundOuter * cos(_ray.angle),
				anchor.y + _ray.boundOuter * sin(_ray.angle)
				);
			i++;
		}

		cvReleaseImage( &pSrc->image );
		delete pSrc;
	}
	//-------- END DEBUG -------------



	// Fit parabolas (Angle range = 0.8PI)
	//---------------------------------------
	int numPointsQ1Q2, numPointsQ3Q4;

	// Parabola#1 > Q1 and Q2 (Spatial coordinate)
	std::vector<float> parabQ1Q2 = this->ApproximateParabolaInRange(
		1.1*PI,
		1.9*PI,
		"UpperCurve",
		numPointsQ1Q2
		);

	// Parabola#2 > Q3 and Q4 (Spatial coordinate)
	std::vector<float> parabQ3Q4 = this->ApproximateParabolaInRange(
		0.1,
		0.9*PI,
		"LowerCurve",
		numPointsQ3Q4
		);

	std::vector<float> parabola;

	// Analyze the calculated parabola
	if (parabQ1Q2.size()>0 && parabQ3Q4.size()>0)
	{
		// Both parabolas are detected, pick the most optimal one
		if (numPointsQ1Q2 > numPointsQ3Q4)
			parabola = parabQ1Q2;
		else
			parabola = parabQ3Q4;

	}
	else if (parabQ1Q2.size()>0)
	{
		parabola = parabQ1Q2;
	}
	else if (parabQ3Q4.size()>0)
	{
		parabola = parabQ3Q4;
	}

	// Given the inner and outer parabolas as constraints,
	// Find the region of the objects lying within that constraints
	CvRect constraint;
	listPoints = CollectObjectsInConstraint(
			parabola,
			constraint
		);
	return true;
}

vector<CvPoint2D32f> __fastcall pCurveOnPlane::CollectObjectsInConstraint(vector<float> parabolaConstraints, CvRect &region )
{
	if (parabolaConstraints.size()<6)
		return vector<CvPoint2D32f>();

	// Gather parabolas coeffs
	float c_i[3] = { parabolaConstraints[0], parabolaConstraints[1], parabolaConstraints[2] };
	float c_o[3] = { parabolaConstraints[3], parabolaConstraints[4], parabolaConstraints[5] };

	// Initialize the contraint rectangle
	float constraint_x0	= 99;
	float constraint_y0	= 99;
	float constraint_x1	= 0;
	float constraint_y1 = 0;

	CvSize size = cvGetSize(this->mat);
	CvMat* matObj = cvCreateMat( size.height, size.width, CV_32FC1 );
	cvZero( matObj );

	// List of feasible pixels meeting the constraints
	
	vector<CvPoint2D32f> listPixelsInConstraints;
	for (int u=0; u<size.width; u++)
		for (int v=0; v<size.height; v++)
		{
			float fu = (float)u;
			float fv = (float)v;

			// Skip the points outside the angle range
			float dy = abs(fv-anchor.y);
			float dx = abs(fu-anchor.x);
			float angle = atan2(dy, dx);
			
			// Check if point (u,v) lies between two parabolas
			bool isConvexInner;
			bool isConvexOuter;

			if (c_i[2]<0) // Upside down (in cartesian view)
			{
				// y <= c0 + c1x + c2x^2, c2<0
				isConvexInner = (fv <= c_i[0] + c_i[1]*fu + c_i[2]*fu*fu);
			}
			else // normal placing
			{
				// y >= c0 + c1x + c2x^2, c2>0
				isConvexInner = (fv >= c_i[0] + c_i[1]*fu + c_i[2]*fu*fu);
			}

			// Do so with the outer criterion
			if (c_o[2]<0) // Upside down (in cartesian view)
			{
				// y <= c0 + c1x + c2x^2, c2<0
				isConvexOuter = (fv <= c_o[0] + c_o[1]*fu + c_o[2]*fu*fu);
			}
			else // normal placing
			{
				// y >= c0 + c1x + c2x^2, c2>0
				isConvexOuter = (fv >= c_o[0] + c_o[1]*fu + c_o[2]*fu*fu);
			}

			// The point (u,v) must meet both criteria
			if (!isConvexInner && cvGetReal2D(this->mat,v,u)>0)
			{
				listPixelsInConstraints.push_back( cvPoint2D32f(fu,fv) );

				// Draw in the matrix as thinning input
				cvSetReal2D( matObj, v, u, 255 );

				// Update the constraint (if meet)
				constraint_x0 = fu < constraint_x0 ? fu : constraint_x0;
				constraint_y0 = fv < constraint_y0 ? fv : constraint_y0;
				constraint_x1 = fu > constraint_x1 ? fu : constraint_x1;
				constraint_y1 = fv > constraint_y1 ? fv : constraint_y1;
			}
		}

	// Conclude the region
	region = cvRect(constraint_x0, constraint_y0, constraint_x1, constraint_y1 );

	// Thin the extracted line
	// by calculating average line between the upper and lower limit
	//---------------------------------------
	vector<CvPoint> upperBound, lowerBound;
	for (register int u=constraint_x0; u<constraint_x1; u++)
	{
		bool upperFound = false, lowerFound = false;
		CvPoint p_upper, p_lower;
		// Find the upper bound and lower bound at the time
		for (register int v=1; v<size.height; v++)
		{
			float valUpper = cvGetReal2D(matObj,v,u);
			float valLower = cvGetReal2D(matObj,size.height-v,u);

			if (!upperFound && valUpper>0)
			{
				p_upper = cvPoint(u,v);
				upperFound = true;
			}
			if (!lowerFound && valLower>0)
			{
				p_lower = cvPoint(u,size.height-v);
				lowerFound = true;
			}

			if (upperFound && lowerFound)
				break;
		}
		
		// Collect the upper/lower bound points
		upperBound.push_back( p_upper );
		lowerBound.push_back( p_lower );
	}

	// Average the upper bound and lower bound to work out the back bone line
	listOutput.clear();
	vector<CvPoint2D32f> listSkeleton;
	for (int i=0; i<upperBound.size(); i++)
	{
		// Weighted boundaries
		listSkeleton.push_back(
			cvPoint2D32f( 
				0.3*upperBound[i].x + 0.7*lowerBound[i].x, 
				0.3*upperBound[i].y + 0.7*lowerBound[i].y
				)
			);
	}

	// Now fit the thinned skeleton with a simple polynomial
	//-------------------------------------------------------
	const int maxDegree = 4;
	float* polyLine = new float[maxDegree+1]; // ORIGINAL:pLeastSquareFit::fitPolynomial( listSkeleton, finalDegree );
	float minError = 1E10; // Error of the chosen degree
	int finalDegree = -1; // Chosen degree
	float errorDegree[maxDegree];

	// Find the best degree to fit our data
	for (int d=2; d<=maxDegree; d++)
	{
		float* poly = pLeastSquareFit::fitPolynomial( listSkeleton, d );
		float polyError = 0;
		// Calculate the average error
		for (unsigned int m=0;m<listSkeleton.size();m++)
		{
			CvPoint2D32f pp = listSkeleton[m];
			float py = 0;
			for (int n=0;n<=d;n++)
				py += pow(pp.x,(float)n)*poly[n];
			polyError += pow(py-pp.y,2.f);
		}
		
		if (polyError<minError) // Update the chosen degree
		{
			minError = polyError;
			finalDegree = d;
			for (int j=0;j<=d;j++) polyLine[j] = poly[j];
		}

		errorDegree[d] = polyError;

		delete[] poly;
	}


		// Get the sample points from the polynomial
	float ratioX = 400.f/size.width;
	float ratioY = 400.f/size.height;
	for (float _x=0; _x<size.width; _x+=4)
	{
			float x = _x;
			float y = 0;
			for (register int k=0; k<=finalDegree; k++)
			{
				y += pow(x,(float)k)*polyLine[k];
			}

			// Skip if the point exceeding the constraints
			if (x<constraint_x0 || x>constraint_x1 || y<constraint_y0 || y>constraint_y1)
				continue;

				listOutput.push_back(
					cvPoint((int)floor(x), (int)floor(y))
				);
	}
	

	// Trace (If enabled)
	//-----------------------
	if (this->isDebugTraceEnabled)
	{
		pImage* pTrace = new pImage(400,400);
		pImage* pMat = new pImage(this->mat);

		cvResize( pMat->image, pTrace->image, CV_INTER_LINEAR );

		// Draw the eligible region
		for (vector<CvPoint2D32f>::iterator p = listPixelsInConstraints.begin();
			p != listPixelsInConstraints.end();
			p++
			)
		{
			CvPoint2D32f point = *p;
			pTrace->DrawCircle(
				cvPoint( point.x*ratioX, point.y*ratioY ),
				2,
				CV_RGB(255,210,0),
				true,
				-1
				);
		}

		for (int i=0; i<listSkeleton.size(); i++)
			pTrace->DrawCircle(
				cvPoint( listSkeleton[i].x*ratioX, listSkeleton[i].y*ratioY ),
				3,
				CV_RGB(190,20,0),
				true
				);

		for (int i=0; i<listOutput.size(); i++)
			pTrace->DrawCircle(
				cvPoint( listOutput[i].x*ratioX, listOutput[i].y*ratioY ),
				4,
				CV_RGB(50,170,0),
				true,
				2
				);


		// Draw the fitted polynomial curve
		for (float _x=-this->anchor.x; _x<pTrace->image->width; _x+=0.01)
		{
			float x = _x; // REMARK: Multiply the scale ratio here
			float y = 0;
			for (register int k=0; k<=finalDegree; k++)
			{
				y += pow(x,(float)k)*polyLine[k];
			}

			// Skip if the point exceeding the constraints
			if (x<constraint_x0 || x>constraint_x1 || y<constraint_y0 || y>constraint_y1)
				continue;

			// Shift the center point (as the image has been resized)
			x *= ratioX;
			y *= ratioY;

			if (y>=0 && y<pTrace->image->height)
				pTrace->DrawCircle( cvPoint(x,y), 1, CV_RGB(255,200,0), true, 1 );
		}


		if (DISP_EXTRACTED_LINE)
		{
			char strCoeff[160];
			sprintf( strCoeff, "degree %d",finalDegree
			);

			for (int n=2; n<=4; n++)
			{
				char strErr[30];
				sprintf( strErr, "Err[%d] = %3.3f", n, errorDegree[n] );
				pTrace->DrawCaption(
					cvPoint(20, 80+(n-2)*12),
					0.42,
					CV_RGB(0,200,140),
					strErr
					);
			}

			pTrace->DrawCaption(
				cvPoint(20,30),
				0.42,
				CV_RGB(255,255,0),
				strCoeff
				);

			cvNamedWindow("trace");
			pTrace->ShowImage("trace");
		}

		cvReleaseImage( &pMat->image );
		cvReleaseImage( &pTrace->image );
		delete pTrace;
		delete pMat;
	}

	delete[] polyLine;
	cvReleaseMat( &matObj );

	return listPixelsInConstraints;
}

