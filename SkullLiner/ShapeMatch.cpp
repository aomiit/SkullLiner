#include "stdafx.h"
#include "ShapeMatch.h"

ShapeMatch::ShapeMatch(void)
{
}

ShapeMatch::~ShapeMatch(void)
{
}

void ShapeMatch::init()
{
}

void ShapeMatch::finalize()
{
}

bool ShapeMatch::fromMatrix( CvMat* mat )
{
	if (mat==NULL) return false;
	else
	{
		this->constraintTop = 0;
		this->constraintBottom = 0;
		CvSize sizeMat = cvGetSize( mat );
		score = 0;
		listScores.clear();
		for (int i=0; i<sizeMat.width; i++) // Left-to-right scanning
			for (int j=0; j<sizeMat.height; j++)
			{
				if (cvGetReal2D(mat,j,i)>0)
				{
					// Calculate and accumulate the score at the coordinate (i,j)
					float _score = calculateSinglePointScore(i,j,sizeMat);
					listScores.push_back( _score );
					listPoints.push_back( cvPoint2D32f(i,j) );
					score += _score;
				}
			}

		// Calculate the boundaries
		for (register int v=3; v<sizeMat.height-2; v++)
		{
			bool pointFound = false;
			if (this->constraintBottom > 0 && this->constraintTop > 0) 
				break;

			for (register int u=4; u<sizeMat.width-4; u++)
			{
				if (cvGetReal2D(mat,v,u)>0)
				{
					pointFound = true;
					if (this->constraintTop == 0)
						this->constraintTop = v;
				}
			}
			if (!pointFound && this->constraintTop>0 && this->constraintBottom==0)
			{
				this->constraintBottom = v;
				break;
			}
		}

		return true;
	}
}

float ShapeMatch::calculateSinglePointScore( int i, int j, CvSize size )
{
	// Just an abstract function
	return i*j;
}

float ShapeMatch::getScore()
{
	return score;
}

//------------------------------------------------------------------------------------------

UShape::UShape()
{
}


UShape::~UShape()
{
}

int UShape::numPoints()
{
	return this->listPoints.size();
}

float UShape::calculateSinglePointScore( int i, int j, CvSize size )
{
	// Score of each point (i,j):
	// The Euclidean distance from p(i,j) to the center of the image
	CvPoint2D32f pointCenter = cvPoint2D32f( (float)size.width/2, (float)size.height/2 );	
	return sqrt( pow(pointCenter.x-(float)i,2) + pow(pointCenter.y-(float)j,2) );
}

vector<float> UShape::getScores()
{
	// Validate the shape score
	vector<float>::iterator s = this->listScores.begin();
	vector<CvPoint2D32f>::iterator p = this->listPoints.begin();

	int itemNo=0;

	// Three-part shape analysis
	float partScoreYShift[] = {0,0,0}; 
	float partScoreDistance[] = {0,0,0};
	int indexPart1 = 0.4*this->listScores.size();
	int indexPart3 = 0.6*this->listScores.size();

	if (this->listScores.size()>=1) s++;
	if (this->listPoints.size()>=1) p++;
	while (s!=this->listScores.end())
	{
		// Normalize the score
		float _score		= (*s)/this->score;
		float _scorePrev	= (*(s-1))/this->score;
		CvPoint2D32f _point		= (*p);
		CvPoint2D32f _pointPrev	= (*(p-1));

		// Score#1: Y-shifting
		float scoreYShift = _point.y - _pointPrev.y;
		// Score#2: Distance dynamic
		float scoreDistance = _score - _scorePrev;

		if (itemNo<indexPart1)
		{
			partScoreYShift[0] += scoreYShift;
			partScoreDistance[0] += scoreDistance;
		}
		else if (itemNo>indexPart3)
		{
			partScoreYShift[2] += scoreYShift;
			partScoreDistance[2] += scoreDistance;
		}
		else
		{
			partScoreYShift[1] += scoreYShift;
			partScoreDistance[1] += scoreDistance;
		}

		itemNo++;
		s++; p++;
	}

	vector<float> listReturnScore;
	for (register int i=0; i<3; i++)
	{
		listReturnScore.push_back( partScoreYShift[i] );
		listReturnScore.push_back( partScoreDistance[i] );
	}
	return listReturnScore;
}

void UShape::getVerticalConstraint( float &yTop, float &yBottom )
{
	yTop = this->constraintTop;
	yBottom = this->constraintBottom;
}


