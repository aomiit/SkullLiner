//////////////////////////////////////////////////////////////////
//
//	Skull Liner : Main Testing Application
//	by Pathompong Ruangyam
//
//////////////////////////////////////////////////////////////////

#ifndef MAIN_APP
#define MAIN_APP

#include <windows.h>
#include <vector>
#include <string>
#include <iostream>

using namespace std;
#include <stdio.h>
#include "AutoNeuro.h"

int CALLBACK WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine,int nCmdShow)
{
	// TEST:
	AutoNeuro* autoNeu = new AutoNeuro();
	AutoNeuro* autoNeu2 = new AutoNeuro();

	// CONFIG HERE
	bool displayBaseline	= false;
	bool displayPhase2	= true;
	

	// Assign the criteria
	unsigned int dim[3] = {400,400,300};
	unsigned int maxPoints = 1024;
	unsigned int maxIter = 10;
	unsigned long maxMem = 250*1024*1024; // Maximum 250 MB allowed
	autoNeu->setCriteria( maxPoints, maxIter, maxMem, dim );
	autoNeu2->setCriteria( maxPoints, maxIter, maxMem, dim );

	UINT count = 0;
	UINT failedCount = 0;

	// DEBUG:
	// Repeat extraction test
	/*char path[255];
	sprintf( path, "X:\\CT_VOLUME\\SAMPLE2\\Stack%03d.raw", 220 );
	for (int n=0; n<30; n++)
	{
		autoNeu2->registerPotentialInputNo( path );
		vector<CvPoint2D32f> points = autoNeu2->generateOutputCurve2( 90 );
		autoNeu2->clean();

		// Save the point list
		char pointdata[2048];
		sprintf( pointdata, "%d points > ", points.size() );
		for (int i=0; i<points.size(); i++)
		{
			char szp[30];
			sprintf( szp, " %0.0f", points[i].x );
			strcat( pointdata, szp );
		}
		strcat( pointdata, "\n" );

		ofstream pencil;
		pencil.open( "X:\\CT_VOLUME\\RESULT\\repeattest.txt", std::ios_base::app );
		pencil << pointdata;
		pencil.close();
	}*/



	for (register int i=200; i<235; i++) // Artifact-less ORIGINAL:201-225
	{
		char path[255];
		sprintf( path, "X:\\CT_VOLUME\\SAMPLE2\\Stack%03d.raw", i );
		autoNeu->registerPotentialInputNo( path );
		autoNeu2->registerPotentialInputNo( path );
		if (displayBaseline) { if (autoNeu->generateOutputCurve( 90 ).size()<=1) failedCount++; }
		if (displayPhase2) autoNeu2->generateOutputCurve2( 90 );
		count++;
		autoNeu->clean();
		autoNeu2->clean();
		//cvWaitKey(0);
	}
	for (register int i=153; i<180; i++) // ÍØ´¿Ñ¹  ORIGINAL: 163-176
	{
		autoNeu->addInputSeries( "X:\\CT_VOLUME\\SAMPLE4\\SAMPLE_%03d.raw", 300 );
		autoNeu->registerPotentialInputNo( i );
		autoNeu2->addInputSeries( "X:\\CT_VOLUME\\SAMPLE4\\SAMPLE_%03d.raw", 300 );
		autoNeu2->registerPotentialInputNo( i );
		if (displayBaseline) { if (autoNeu->generateOutputCurve( 90 ).size()<=1) failedCount++; }
		if (displayPhase2) autoNeu2->generateOutputCurve2( 90 );
		count++;		
		autoNeu->clean();
		autoNeu2->clean();
		//cvWaitKey(0);
	}
	for (register int i=201; i<217; i++) // ÍØ´¿Ñ¹ (clean part)
	{
		autoNeu->addInputSeries( "X:\\CT_VOLUME\\SAMPLE4\\SAMPLE_%03d.raw", 300 );
		autoNeu->registerPotentialInputNo( i );
		autoNeu2->addInputSeries( "X:\\CT_VOLUME\\SAMPLE4\\SAMPLE_%03d.raw", 300 );
		autoNeu2->registerPotentialInputNo( i );
		if (displayBaseline) { if (autoNeu->generateOutputCurve( 90 ).size()<=1) failedCount++; }
		if (displayPhase2) autoNeu2->generateOutputCurve2( 90 );
		count++;		
		autoNeu->clean();
		autoNeu2->clean();
		//cvWaitKey(0);
	}
	
	// ORIGINAL: 182-207  
	for (register int i=180; i<215; i++) // Artifact-less High-contrast, multiple parts
	{
		autoNeu->addInputSeries( "X:\\CT_VOLUME\\SAMPLE1\\Skull,Skull_07292010_1737_%03d.raw", 300 );
		autoNeu->registerPotentialInputNo( i );
		autoNeu2->addInputSeries( "X:\\CT_VOLUME\\SAMPLE1\\Skull,Skull_07292010_1737_%03d.raw", 300 );
		autoNeu2->registerPotentialInputNo( i );
		if (displayBaseline) { if (autoNeu->generateOutputCurve( 90 ).size()<=1) failedCount++; }
		if (displayPhase2) autoNeu2->generateOutputCurve2( 90 );
		count++;		
		autoNeu->clean();
		autoNeu2->clean();
		//cvWaitKey(0);
	}
	for (register int i=235; i<254; i++)// Artifact-less High-contrast, multiple parts
	{
		autoNeu->addInputSeries( "X:\\CT_VOLUME\\SAMPLE1\\Skull,Skull_07292010_1737_%03d.raw", 300 );
		autoNeu->registerPotentialInputNo( i );
		autoNeu2->addInputSeries( "X:\\CT_VOLUME\\SAMPLE1\\Skull,Skull_07292010_1737_%03d.raw", 300 );
		autoNeu2->registerPotentialInputNo( i );
		if (displayBaseline) { if (autoNeu->generateOutputCurve( 90 ).size()<=1) failedCount++; }
		if (displayPhase2) autoNeu2->generateOutputCurve2( 90 );
		count++;		
		autoNeu->clean();
		autoNeu2->clean();
		//cvWaitKey(0);
	}
	// ORIGINAL: 197-217
	for (register int i=188; i<227; i++) // ´Ñ´¿Ñ¹
	{
		autoNeu->addInputSeries( "X:\\CT_VOLUME\\SAMPLE3\\SAMPLE_%03d.raw", 300 );
		autoNeu->registerPotentialInputNo( i );
		autoNeu2->addInputSeries( "X:\\CT_VOLUME\\SAMPLE3\\SAMPLE_%03d.raw", 300 );
		autoNeu2->registerPotentialInputNo( i );
		if (displayBaseline) { if (autoNeu->generateOutputCurve( 90 ).size()<=1) failedCount++; }
		if (displayPhase2) autoNeu2->generateOutputCurve2( 90 );
		count++;
		autoNeu->clean();
		autoNeu2->clean();
		//cvWaitKey(0);
	}

	// DEBUG: Display the false ratio
	float falseRate = (float)failedCount*100/(float)count;
	pImage* pResult = new pImage(200, 40);
	cvZero( pResult->image );
	char strFalse[32];
	sprintf( strFalse, "%0.2f%%/%d", falseRate, count );
	pResult->DrawCaption( cvPoint(8,20), 0.7, CV_RGB(255,200,0), strFalse );
	cvNamedWindow( "result" );
	pResult->ShowImage( "result" );
	cvWaitKey(0);

	delete pResult;
	return 0;
}

#endif 