// LibAutoPanora.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "AutoNeuro.h"
#include "jAutoPano.h"


#ifdef _MANAGED
#pragma managed(push, off)
#endif

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
    return TRUE;
}

__declspec(dllexport) LPVOID libapGenerateCurveFromRaw( LPCSTR lpstrPathRaw, int numControlPoints=32, bool debug=false )
{
	AutoNeuro* autoN = new AutoNeuro();
	autoN->registerPotentialInputNo( lpstrPathRaw );
	vector<CvPoint2D32f> points = autoN->generateOutputCurve( numControlPoints );
	delete autoN;

	// REMARK: This function is no longer available publicly.

	return NULL;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

