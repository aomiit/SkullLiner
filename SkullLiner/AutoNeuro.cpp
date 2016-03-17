#include "stdafx.h"
#include "AutoNeuro.h"


AutoNeuro::AutoNeuro(void)
{
	// Initialize the class members
	this->imgCandidate = NULL;
	this->matReduced = NULL;
}

AutoNeuro::~AutoNeuro(void)
{
	// Clear all allocated memory blocks
	if (this->imgCandidate!=NULL)
	{
		cvReleaseImage( &this->imgCandidate->image );
		delete this->imgCandidate;
		this->imgCandidate = NULL;
	}
	if (this->matReduced!=NULL)
	{
		cvReleaseMat( &this->matReduced );
		this->matReduced = NULL;
	}
}

bool AutoNeuro::readSingleInput( const char* filename, pImage* &buf )
{
	// load input file
	vtkImageReader *reader = vtkImageReader::New();
	reader->SetFileName( filename );
	reader->SetFileDimensionality(2);
	reader->SetDataExtent( 0, this->crit.dimVoxel[0]-1, 0, this->crit.dimVoxel[1]-1, 0, 0 );
	reader->SetDataByteOrderToLittleEndian();
	reader->SetDataScalarTypeToUnsignedShort();
	reader->SetNumberOfScalarComponents(1);
	reader->Update();


	// Convert vtkImageData to IplImage of OpenCV
	CvMat* matImg = this->convertVTKImageDataToMatrix( reader->GetOutput() );
	if (matImg != NULL)
	{
		buf = new pImage( matImg );
		this->imgCandidate = new pImage( buf );
		return true;
	}
	else 
		return false;

	return false;
}


bool AutoNeuro::readSingleInput( const char* filename, CvMat* &mat )
{
	// load input file
	vtkImageReader *reader = vtkImageReader::New();
	reader->SetFileName( filename );
	reader->SetFileDimensionality(2);
	reader->SetDataExtent( 0, this->crit.dimVoxel[0]-1, 0, this->crit.dimVoxel[1]-1, 0, 0 );
	reader->SetDataByteOrderToLittleEndian();
	reader->SetDataScalarTypeToUnsignedShort();
	reader->SetNumberOfScalarComponents(1);
	reader->Update();


	// Convert vtkImageData to IplImage of OpenCV
	CvMat* matImg = this->convertVTKImageDataToMatrix( reader->GetOutput() );
	if (matImg != NULL)
	{
		mat = matImg;
		this->imgCandidate = new pImage( matImg );
		reader->Delete(); 
		return true;
	}
	else 
	{
		reader->Delete();
		return false;
	}

	return false;
}


bool AutoNeuro::readSingleInput( int stackNo, CvMat* &mat )
{
	if (stackNo<0) return false;
	
	char filename[255];
	char filename_out[255];
	sprintf( filename, this->fileNameFormat, stackNo );
	sprintf( filename_out, "X:\\VolumeRAW_8bit\\slice%03d.jpg", stackNo );

	return readSingleInput( filename, mat );
}

bool AutoNeuro::readSingleInput( int stackNo, pImage* &buf )
{
	if (stackNo<0) return false;
	
	char filename[255];
	char filename_out[255];
	sprintf( filename, this->fileNameFormat, stackNo );
	sprintf( filename_out, "X:\\VolumeRAW_8bit\\slice%03d.jpg", stackNo );

	// load input file
	vtkImageReader *reader = vtkImageReader::New();
	reader->SetFileName( filename );
	reader->SetFileDimensionality(2);
	reader->SetDataExtent( 0, this->crit.dimVoxel[0]-1, 0, this->crit.dimVoxel[1]-1, 0, 0 );
	reader->SetDataByteOrderToLittleEndian();
	reader->SetDataScalarTypeToUnsignedShort();
	reader->SetNumberOfScalarComponents(1);
	reader->Update();


	// Convert vtkImageData to IplImage of OpenCV
	pImage* pImg = this->convertVTKImageDataToPImage( reader->GetOutput() );
	if (pImg != NULL)
	{
		buf = pImg;
		this->imgCandidate = new pImage( pImg );
		return true;
	}
	else 
		return false;

	return false;
}

pImage* AutoNeuro::convertVTKImageDataToPImage( vtkImageData* dat )
{
	// For each pixel
	int width, height;
	int* dim = new int[6];
	dim = dat->GetExtent();
	width = dim[1];
	height = dim[3];
	pImage* img = new pImage( width, height );
	unsigned short* pixels = (unsigned short*)dat->GetScalarPointer();
	for (int x=0; x<width; x++)
		for (int y=0; y<height; y++)
		{
			int index = dat->FindPoint( x, y, 0 );
			unsigned short px = pixels[index];
			// Scale down 16-bit to 8-bit value for memory consumption reduction
			unsigned char value = (unsigned char)pow((float)log((float)px)/log(2.f),2.f);
			img->setRGB( x, y, value, value, value );
		}

	return img;
}

CvMat* AutoNeuro::convertVTKImageDataToMatrix( vtkImageData* dat )
{
	// For each pixel
	int width, height;
	int* dim = new int[6];
	dim = dat->GetExtent();
	width = dim[1];
	height = dim[3];
	CvMat* mat = cvCreateMat( height, width, CV_8UC1 );
	unsigned short* pixels = (unsigned short*)dat->GetScalarPointer();

	// Get window level
	unsigned short min = 1E5, max=0;
	for (int x=0; x<width; x++)
		for (int y=0; y<height; y++)
		{
			int index = dat->FindPoint( x, y, 0 );
			unsigned short px = pixels[index];
			min = (px < min? px : min );
			max = (px > max? px : max );
		}

		windowLevel_min = min;
		windowLevel_max = max;

	for (int x=0; x<width; x++)
		for (int y=0; y<height; y++)
		{
			int index = dat->FindPoint( x, y, 0 );
			unsigned short px = pixels[index];

			// Stretch the max value
			if (!ENABLE_WINDOWLEVEL)
			{
				if (px>=0.9 *max)
					px = max;
			}
			else
			{ // Window-level adjustment

				// Max value cut-off
				px = (px>4000 && windowLevel_max<16000 ?4000:px);
				px = (px<900 ? 0:px);
				// Stretch the range from 900-4000 to 0-5000
				if (px>0)
					px = (unsigned short)(((float)px-900)*5000.f/(4000.f-900.f));
			}

			// Scale down 16-bit to 8-bit value for memory consumption reduction
			cvSetReal2D( mat, y, x, (float)convert16bitTo8bit(px) );
		}

	return mat;
}

unsigned char __fastcall AutoNeuro::convert16bitTo8bit( double v )
{
	return (unsigned char)pow((float)log(v)/log(2.f),2.f);
}


bool AutoNeuro::addInputSeries( const char* _filenameFormat, unsigned int _count )
{
	strcpy( this->fileNameFormat, _filenameFormat );
	return true;
}

bool AutoNeuro::setCriteria( unsigned int maxPoints, unsigned int maxIter, unsigned long maxMem, unsigned int* voxelSize )
{
	if (voxelSize != 0)
		for (register int i=0; i<3; i++)	this->crit.dimVoxel[i] = voxelSize[i];
	this->crit.maxIter = maxIter;
	this->crit.maxMemoryAllowed = maxMem;
	this->crit.maxNumOfPoints = maxPoints;
	return true;
}

vector<CvPoint2D32f> AutoNeuro::generateOutputCurve2( int numControlPoints )
{
	vector<CvPoint2D32f> listCurvePoint;

	// Step#1:  For each candidate, try to smoothen the lines
	// REMARK: The process stops in this step if it fails to load the candidate image from memory	
	pImage* pCandidate = new pImage(this->matReduced);
	float scaleRatio = ((float)this->crit.dimVoxel[0]) / ((float)TEMPLATE_WIDTH);

	if (pCandidate !=NULL)
	{
		pCandidate->Smooth( 5 );
		cvThreshold( pCandidate->image, pCandidate->image, POSTSMOOTHEN_THRESH, 255, CV_THRESH_BINARY );	
		
		// Remove small particles from the image
		float sizeParticles = 36;
		pCandidate->FilterSmallParticles( sizeParticles );


		// Raycasting
		//====================
		CvMat* matDebug = cvCreateMat(
			pCandidate->image->height, 
			pCandidate->image->width, 
			CV_32FC1 
			);
		cvZero(matDebug);

		// Copy pCandidate to matDebug
		for (register int u=0; u<pCandidate->image->width; u++)
			for (register int v=0; v<pCandidate->image->height; v++)
			{
				int r, g, b;
				pCandidate->getRGB(u, v, r, g, b);
				cvSetReal2D( matDebug, v, u, (double)r );
			}

		if (DISP_CONTOUR)
		{
			cvNamedWindow("contour");
			cvShowImage("contour", matDebug);
		}

		pCurveOnPlane* curve = new pCurveOnPlane(
			matDebug, 
			DISP_CONTOUR
			);
		std::vector<CvPoint2D32f> listPointsDebug;

		curve->FindApproxCurve(
			listPointsDebug,
			PI/96
			);
		
		// Extract the parameters from curve (and scale up to match the actual size)
		vector<CvPoint> outputSkeleton = curve->listOutput;
		CvSize sizeReduced = cvGetSize( this->matReduced );
		float scaleX = (float)this->imgCandidate->image->width/(float)sizeReduced.width;
		float scaleY = (float)this->imgCandidate->image->height/(float)sizeReduced.height;
		for (register int i=0; i<outputSkeleton.size(); i++)
		{
			outputSkeleton[i].x = (int)ceil(outputSkeleton[i].x*scaleX);
			outputSkeleton[i].y = (int)ceil(outputSkeleton[i].y*scaleY);
		}

		delete curve;

		//------ END Raycasting---------------------

		// Step#2: Extract the skeletal line of the extracted region
		if (DISP_ALIGNMENT_RESULT)
		{
			pImage *pEL = new pImage( this->imgCandidate );
			
			// Draw each point and connected lines
			CvPoint p_precede;
			vector<CvPoint>::iterator p = outputSkeleton.begin();
			while (p!=outputSkeleton.end())
			{
				pEL->DrawCircle( 
					*p, 
					4, 
					CV_RGB(30,215,90), 
					true 
					);
				if (p!=outputSkeleton.begin())
				{
					cvLine( pEL->image, p_precede, *p, CV_RGB(0,0,255), 2 );
				}
				p_precede = *p;
				p++;
			}

			char strLabel[100];
			sprintf( strLabel, "%d points", outputSkeleton.size() );
			pEL->DrawCaption(
				cvPoint(40,50),
				0.45,
				CV_RGB(255,255,210),
				strLabel
				);

			char strWindowLevel[100];
			sprintf( strWindowLevel, "WndLevel: %d : %d", windowLevel_min, windowLevel_max );
			pEL->DrawCaption(
				cvPoint(40,70),
				0.45,
				CV_RGB(255,255,210),
				strWindowLevel
				);

			cvNamedWindow( "output" );
			pEL->ShowImage( "output" );

			if (SAVE_RESULT_IMAGE)
			{
				char pathResult[255];
				sprintf( pathResult, PATH_TRACE_RESULT, _counter_ );
				pEL->SaveImage( pathResult );
				_counter_++;
			}

			cvReleaseImage( &pEL->image );
			delete pEL;
		}

		// Step#3: 
		// Produce the output chain of points
		vector<CvPoint>::iterator p = outputSkeleton.begin();
		while (p!=outputSkeleton.end())
		{
			CvPoint _p = *p;
			listCurvePoint.push_back(
				cvPoint2D32f( _p.x, _p.y )
				);
			p++;
		}

		if (TRACE_ELEMENTS==true)
			cvWaitKey(0);

		// Clean things
		cvReleaseMat(&matDebug);

		if (pCandidate->image) cvReleaseImage( &pCandidate->image );
		delete pCandidate;

	} // If Candidate not null

	pCandidate = NULL;

	cvReleaseMat( &matReduced );
	matReduced = NULL;

	return listCurvePoint;
}

vector<CvPoint2D32f> AutoNeuro::generateOutputCurve( int numControlPoints  )
{
	vector<CvPoint2D32f> listCurvePoint;

	// Step#1:  For each candidate, try to smoothen the lines
	// REMARK: The process stops in this step if it fails to load the candidate image from memory
	pImage* pCandidate = new pImage(this->matReduced);

	if (pCandidate !=NULL)
	{
		float scaleRatio = ((float)this->crit.dimVoxel[0]) / ((float)TEMPLATE_WIDTH);
		pCandidate->Smooth( 5 );
		cvThreshold( pCandidate->image, pCandidate->image, POSTSMOOTHEN_THRESH, 255, CV_THRESH_BINARY );	

		// Step#2: Extract all contours
		vector<CvMat*> listContour = pCandidate->ExtractContours();
		if (DISP_CONTOUR==TRUE)
		{
			pImage* pCanvas = new pImage(this->imgCandidate);
			if (pCanvas != NULL)
			{
				vector<CvMat*>::iterator contour = listContour.begin();
				while (contour!=listContour.end())
				{
					for (register int i=0; i<pCandidate->image->width; i++)
						for (register int j=0; j<pCandidate->image->height; j++)
						{
							if (cvGetReal2D(*contour,j,i)>0)
								pCanvas->DrawCircle( 
									cvPoint( (int)ceil(i*scaleRatio), (int)ceil(j*scaleRatio) ),
									2, CV_RGB(255,0,0),
									true, CV_FILLED
									);
						}

					contour++;

				}
				cvNamedWindow( "cand" );
				cvNamedWindow( "contour" );
				pCandidate->ShowImage( "cand" );
				pCanvas->ShowImage( "contour" );
			}
			delete pCanvas;
		}

		// Step#3: Pick the right contour to process (in case of multitudes of the extracted contours)
		CvMat* matContour = NULL;
		CvMat* matThinnedContour = NULL;
		bool isOpenTop = false;
		char wndName[20];
		float parab_a, parab_b, parab_c;

		if (listContour.size()>=1)
		{
			vector<CvMat*>::iterator contour = listContour.begin();
			// Foreach contour --> Analyze the shape score (similarity to the U shape)
			while (contour!=listContour.end())
			{
				CvMat* mat = *contour;
				// Get the contour thinned & try to connect the fragmented lines together
	   			
				pImage* img = new pImage(mat);
				CvMat* matThin = img->GetParallelThinnedMatrix( true );				

				if (DISP_THINNING_RESULT)
				{
					cvNamedWindow( "thin" );
					cvShowImage( "thin", matThin );
				}

				// Analyze the skeleton shape
				//=========================================================
				// Convert matThin --> listPoints (left to right scanning)

				UShape ushape;
				if (ushape.fromMatrix(matThin))
				{
					vector<float> listScores = ushape.getScores();
	
					pImage* pThin = new pImage( TEMPLATE_WIDTH+60, TEMPLATE_HEIGHT+80 );
					cvSet( pThin->image, CV_RGB(255,255,255 ));
					pThin->PasteMat( matThin, CV_RGB(0,90,255) );

					// Analyze the score (truncate to integer)
					int YshiftL = (int)listScores[0];
					int YshiftR = (int)listScores[4];
					int YshiftM = (int)listScores[2];
					int diffLR	= abs(YshiftL)-abs(YshiftM);
					int maxLR	= max(abs(YshiftL),abs(YshiftR));
					bool LPos = YshiftL>0;
					bool RPos = YshiftR>0;
					if (abs(YshiftL)>=2 && abs(YshiftR)>=2 && (LPos!=RPos) 
						&& maxLR>=5 && ushape.numPoints()>=16)
					{
						// Contour qualifies!
						isOpenTop			= (LPos && !RPos);

						// Reject the contour if it is open-top but located on the top part
						if (isOpenTop)
						{
							float yTop, yBottom;
							ushape.getVerticalConstraint( yTop, yBottom );
							if (yBottom > TEMPLATE_HEIGHT*0.5)
							{
								// Contour qualifies!!
								matContour			= cvCloneMat( mat );
								matThinnedContour	= cvCloneMat( matThin );	
							}
						}
						else
						{
							matContour			= cvCloneMat( mat );
							matThinnedContour	= cvCloneMat( matThin );	
						}

						delete pThin;
					}					
				}

				cvReleaseImage( &img->image );
				delete img;
				contour++;
			}
		}
		else
			return listCurvePoint; // Return nothing

		FREE_CONTOURS( listContour );	


		if (matContour!=NULL)
		{
			// Step#4: Align the skeleton lines through the thinned contour 

			// Calculate the bounding box of the thinned contour (top and bottom constraints)
			CvRect bound = cvRect(0,-100,TEMPLATE_WIDTH,-100);
	   		for (register int v=0; v<TEMPLATE_HEIGHT; v++)
			{
				bool topScanFound = false;
				for (register int u=0; u<TEMPLATE_WIDTH; u++)
				if (cvGetReal2D(matContour,v,u)>0)
				{
					topScanFound = true;
					bound.y = v;
				}
				if (topScanFound)
					break;
			}
			for (register int v=TEMPLATE_HEIGHT-1; v>0; v--)
			{
				bool bottomScanFound = false;
				for (register int u=0; u<TEMPLATE_WIDTH; u++)
				if (cvGetReal2D(matContour,v,u)>0)
				{
					bottomScanFound = true;
					bound.height = v-bound.y;
				}
				if (bottomScanFound)
					break;
			}

			// Extract the line into a string of points
			vector<CvPoint2D32f> vectorPoints = vector<CvPoint2D32f>();
			for (register int u=0; u<TEMPLATE_WIDTH; u++)
				for (register int v=bound.y; v<bound.y+bound.height; v++)
				{
					if (cvGetReal2D(matThinnedContour,v,u)>0)
					{
						vectorPoints.push_back(cvPoint2D32f(u,v));
						u++;
						continue;
					}
				}

			// Sort the elements for proper order
			if (isOpenTop)
				std::sort( vectorPoints.begin(), vectorPoints.end(), arrangePointOpenTop );
			else
				std::sort( vectorPoints.begin(), vectorPoints.end(), arrangePointOpenBottom );
			
			// Scale the control points to match the real coordinate
			vector<CvPoint2D32f>::iterator p = vectorPoints.begin();
			while (p!=vectorPoints.end())
			{
				(*p).x *= scaleRatio; (*p).x = ceil((*p).x);
				(*p).y *= scaleRatio; (*p).y = ceil((*p).y);
				p++;
			}


			if (DISP_EXTRACTED_LINE)
			{
				pImage *pEL = new pImage( this->imgCandidate );
				
				// Draw each point and connected lines
				vector<CvPoint2D32f>::iterator p = vectorPoints.begin();
				while (p!=vectorPoints.end())
				{
	   	   			CvPoint2D32f point = *p;
					pEL->DrawCircle( cvPointFrom32f(point), 3, CV_RGB(200,255,0), true, 1 );

					if (p!=vectorPoints.begin())
					{
						CvPoint2D32f point_a = *(p-1);
						cvLine( pEL->image, cvPointFrom32f(point_a), cvPointFrom32f(point),
							CV_RGB(220,215,0), 1, CV_AA );
					}

					p++;
				}

				char strPoint[30];
				sprintf( strPoint, "%d points", vectorPoints.size() );
				pEL->DrawCaption( cvPoint(8, pEL->image->height-20), 0.4, CV_RGB(128,220,0), strPoint );

				cvNamedWindow( "EL" );
				pEL->ShowImage( "EL" );
				cvWaitKey(5);
				delete pEL;
			} // if DISP_EXTRACTED_LINE	


			// Step#5: Utilize least square to solve for the polynomial
			// which best fits the extracted point data.
			
			float* params = NULL;
			if (POLY_DEGREE==2)
				params = pLeastSquareFit::fitParabola( vectorPoints );
			else
				params = pLeastSquareFit::fitPolynomial( vectorPoints, POLY_DEGREE );
			vector<float> polyCoeff;
			
			for (register int k=0; k<=POLY_DEGREE; k++)
				polyCoeff.push_back( params[k] );
			delete []params;

			// Step#6: Extend the line using the calculated parabolic parameters
			// REMARK: This will remove all scattered points which yield high error
			// between the point itself and the fitted parabola
			refineParabolicLine( vectorPoints, polyCoeff, isOpenTop );

			if (DISP_ALIGNMENT_RESULT)
			{
				pImage* pParabola = new pImage( this->imgCandidate );

				// Draw the fitted parabola
				if (DISP_DRAW_PARABOLA)
				{
					for (float _x=0; _x<pParabola->image->width; _x+=0.01)
					{
						float x = _x*scaleRatio;
						float y = 0;
						for (register int k=0; k<=POLY_DEGREE; k++)
						{
							y += pow(x,(float)k)*polyCoeff[k];
						}
						pParabola->DrawCircle( cvPoint(x,y), 1, CV_RGB(255,200,0), true, 1 );
					}
				}


				// Draw the vector point as overlay
				vector<CvPoint2D32f>::iterator p = vectorPoints.begin();
				while (p!=vectorPoints.end())
				{
					pParabola->DrawCircle( cvPointFrom32f(*p), 2, CV_RGB(0,100,255), false, 2 );
					p++;
				}

				cvNamedWindow( "output_baseline" );
				pParabola->ShowImage( "output_baseline" );
				cvWaitKey(PAUSE_ON_SUCCESS?0:10);

				delete pParabola;
			}				

		
			listCurvePoint = vectorPoints;


		} // If matContour is not null

		else if (DISP_EXTRACTED_LINE)
		{
			// If there is no extracted line but we need to see it,
			// just display the blank one

			string strNothing = "-FAILED-";
			pImage *pEL = new pImage( this->imgCandidate );		
			pEL->DrawCaption( cvPoint(20,60), 1.0, CV_RGB(200,0,0), strNothing.c_str() );

			cvNamedWindow( "EL" );
			pEL->ShowImage( "EL" );
			cvWaitKey(PAUSE_ON_FAILURE?0:10);
			delete pEL;
		}


		if (pCandidate->image) cvReleaseImage( &pCandidate->image );
		delete pCandidate;
		cvReleaseMat( &matThinnedContour );
		cvReleaseMat( &matContour );

	} // if pCandidate is not null



	return listCurvePoint;
}


void __fastcall AutoNeuro::clean()
{
	// Clean up the memory and states in order to get ready for the new incoming inputs.
	this->listControlPoints.clear();
	this->candidateNo = 0;
	strcpy( this->fileNameFormat, "" );
	this->fileSeriesCount = 0;
}


bool AutoNeuro::registerPotentialInputNo( CvMat* mat )
{
	if (mat==NULL) return false;
	this->imgCandidate = new pImage( mat );
	processSingleInput( mat );
	return true;
}


bool AutoNeuro::registerPotentialInputNo( const char* path )
{
	CvMat* matCand = NULL;
	strcpy( this->strCandidatePath, path );
	if (!this->readSingleInput( path, matCand ))
		return false;
	else
	{
		processSingleInput( matCand );
	}
}


void AutoNeuro::processSingleInput( CvMat* matCand )
{
	// Sample down the candidate image
	CvMat* matReduced = this->sampleDownMatrix( matCand, cvRect( 0,0,TEMPLATE_WIDTH, TEMPLATE_HEIGHT ));
	
	// Find the best treshold value and utilize it
	float thresh = findBestThresholdValue( matReduced );
	cvThreshold( matReduced, matReduced, thresh, 255, CV_THRESH_BINARY );

	// Dilate then erode the contours to get them connected together
	cvDilate( matReduced, matReduced );
	cvErode( matReduced, matReduced );

	this->matReduced = cvCloneMat( matReduced );
	cvReleaseMat( &matReduced );
	cvReleaseMat( &matCand );
}

bool AutoNeuro::registerPotentialInputNo( int no )
{
	if (no<0&&no>this->fileSeriesCount)
		return false;

	candidateNo = no;
	sprintf( this->strCandidatePath, this->fileNameFormat, no );
	CvMat* matCand = NULL;
	if (!this->readSingleInput( no, matCand ))
		return false;
	else
	{
		processSingleInput( matCand );
	}
	return true;
}


float AutoNeuro::findBestThresholdValue( CvMat* input )
{
	if (input==NULL) return 0;

	// IDEA: If the deviation of the histogram is packed on the dark area, utilize the constant threshold value
	// else if the deviation is bell-shaped, utilize the hill method

	// Utilizing 255-bin histogram to find best threshold value
	float *binHist = new float[255];
	CvSize size = cvGetSize( input );
	float properThresh = 0;

	for (register int u=0; u<size.width; u++)
		for (register int v=0; v<size.height; v++)
		{
			float val = cvGetReal2D( input, v, u );
			binHist[(int)val]++;
		}

	// Now analyze, find the rightmost bound of the histogram hill
	// and let it be our threshold
	//======================
	// Inspect the location of the highest peak (most frequent color bin)
	int binPeak = -1;
	int peakFreq = 0;
	int numBinFarFromPeak = 0;

	for (register int bin=155; bin>66; bin--)
	{
		if (binHist[bin]<70)
			continue;
		if (binHist[bin]>binHist[bin+1] && binHist[bin]>=binHist[bin-1] &&
			binHist[bin]>binHist[bin+2] && binHist[bin]>=binHist[bin-2])
		{
			binPeak = bin;
			peakFreq = binHist[bin];
			break;
		}
	}

	if (binPeak<=10) // Histogram is packed at the dark area
		properThresh = 255/5; // constantly valued threshold
	else
	{
		// Estimate the width of the bell-shaped threshold
		for (register int i=1; i+binPeak<255; i++)
		{
			if (binHist[i+binPeak]<=0.63*binHist[binPeak] || 
				binHist[i+binPeak]<=6) // Pick the threshold at the certain position
			{
				properThresh = binPeak + i;
				break;
			}
		}
		properThresh = binPeak + 8;
	}

	// DEBUG:: Display the histogram
	if (DISP_HISTOGRAM)
	{
		pImage* pHist = new pImage( 255, 100 );
		cvZero( pHist->image );
		for (register int bin=0; bin<255; bin++)
		{
			cvLine( pHist->image, cvPoint(bin,99), cvPoint(bin,100-binHist[bin]), CV_RGB(255,255,255), 1 );
		}

		// Crease the peak bin and the picked threshold
		cvLine( pHist->image, cvPoint(binPeak,0), cvPoint(binPeak,99), CV_RGB(0,230,60),2, CV_AA );
		cvLine( pHist->image, cvPoint(properThresh,0), cvPoint(properThresh,99), CV_RGB(255,0,0), 2, CV_AA );
		cvNamedWindow( "hist" );
		char strThresh[255];
		char strFreq[255];
		sprintf( strThresh, "%0.0f", properThresh );
		sprintf( strFreq, "Freq = %d", peakFreq );
		pHist->DrawCaption( cvPoint(5,10), 0.35, CV_RGB(255,0,0), strThresh );
		pHist->DrawCaption( cvPoint(5,36), 0.35, CV_RGB(255,0,0), strFreq );
		cvShowImage( "hist", pHist->image );
		cvReleaseImage( &pHist->image );
		delete pHist;
	}

	return properThresh ;
}


CvMat* AutoNeuro::sampleDownMatrix( CvMat* input, CvRect desiredSize )
{
	CvMat* matResult = cvCreateMat( desiredSize.height, desiredSize.width, CV_8UC1 );
	cvResize( input, matResult );
	return matResult;
}

bool AutoNeuro::exportToJpegs( const char* _filenameFormat )
{
	for (int i=0; i<this->crit.dimVoxel[2]; i++)
	{
		// Load the input into buffer memory storage
		pImage* pImg;
		if (this->readSingleInput(i, pImg))
		{
			char pathToSave[255];
			sprintf( pathToSave, _filenameFormat, i );
			pImg->SaveImage( pathToSave );
			cvReleaseImage( &pImg->image );
		}
		delete pImg;
	}
	return true;
}


bool AutoNeuro::refineParabolicLine( vector<CvPoint2D32f> &vec, vector<float> polyCoeff, bool openTop )
{
	if (vec.size()<3 || polyCoeff.size()<=1) return false;

	// Remove the scattered points
	//=============================================
	vector<CvPoint2D32f> newVec;
	vector<CvPoint2D32f>::iterator p = vec.begin();
	CvPoint2D32f p0 = vec.front(); // Recent adjacent point
	do
	{
		// The point to be removed:
		// a) its y coordinate is more than 20 pixels away from the parabola
		// b) it is located too close to the sibling points
		float y = 0;
		for (register int n=0; n<polyCoeff.size(); n++)
			y +=  pow((*p).x,(float)n)*polyCoeff[n];
		CvPoint2D32f parab = cvPoint2D32f((*p).x, y ); 
		if ((abs((*p).y - parab.y)<20) &&
			(EUCLIDEAN_DIST(p0,(*p))>20))
		{
			newVec.push_back(*p);
			p0 = *p;
		}
		p++;
	}
	while (p!=vec.end());


	// Remove the points which cause a zig-zag line
	int numOfZigzagPoints = 0;
	do
	{
		numOfZigzagPoints = 0;
		vec = vector<CvPoint2D32f>();
		vec.push_back( newVec.front() );
		for (int i=1; i<vec.size()-1; i++)
		{
			// Three adjacent points must not produce acute angle.
			float edge1 = vec[i-1].y - vec[i].y;
			float edge2 = vec[i].y - vec[i+1].y;
			bool zigzag = ((edge1<0)==!(edge2>0));
			if (edge1==0 || edge2==0 || !zigzag)
			{
				vec.push_back( newVec[i] );
			}
			else
				numOfZigzagPoints++;
		}
		vec.push_back( newVec.back() );


		// Now we get the cleaned list of points
		vec = newVec;
	}
	while (numOfZigzagPoints>0);
	// Repeat the iteration until we completely remove all unwanted points.


	// Now extend the line using parabolic curve (both wing simulataneously)
	//=============================================
	
	float y0; // Get the starting point (y component from the list)
	if (openTop) y0 = (vec.front().y<vec.back().y)?vec.front().y:vec.back().y;
	else y0 = (vec.front().y>vec.back().y)?vec.front().y:vec.back().y;

	float limitY;
	if (openTop) limitY = 150;
	else limitY = this->imgCandidate->image->height-100;

	// Left extending
	float xL = vec.front().x;
	float xR = vec.back().x;
	do
	{
		xL-=12.f; xR+=12.f;
		float yL = 0, yR = 0;
		for (register int n=0; n<polyCoeff.size(); n++)
		{
			yL += pow(xL,(float)n)*polyCoeff[n];
			yR += pow(xR,(float)n)*polyCoeff[n];
		}
		
		if (openTop) y0 = min(yL,yR);
		else y0 = max(yL,yR);

		vec.insert( vec.begin(), cvPoint2D32f(xL,yL) );
		vec.push_back( cvPoint2D32f(xR,yR) );

		if (min(xL,xR)<50 || max(xL,xR)>this->imgCandidate->image->width-50) break;
	}
	while ((openTop && y0>limitY) || (!openTop && y0<limitY));

	return true;
}



