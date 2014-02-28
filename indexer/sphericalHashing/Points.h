#pragma once

#include "Common.h"

class Points
{
public :
	int nP;
	int dim;
	float **d;
	
	void Initialize(int _nP, int _dim)
	{
		nP = _nP;		dim = _dim;
		d = new float * [ nP ];
		for(int i=0;i<nP;i++)
		{
			d[i] = new float [ dim ];
		}
	}

	// this function is for read point set from file
	// format:
	// (num. points) (dimensionality)
	// v0 (floats, number of elements is equal to dimensionality)
	// v1
	// ...
	void Initialize_From_File(char *fileName)
	{
		FILE *input = fopen( fileName , "rb" );
		fread( &nP , sizeof(int) , 1 , input );
		fread( &dim , sizeof(int) , 1 , input );
		d = new float * [ nP ];
		for(int i=0;i<nP;i++)
		{
			d[i] = new float [ dim ];
		}
		float *tmp = new float [ dim ];
		for(int i=0;i<nP;i++)
		{
			fread( tmp , sizeof(float) , dim , input );
			for(int k=0;k<dim;k++)
			{
				d[i][k] = (float)( tmp[k] );
			}
		}
		fclose(input);
		delete [] tmp;
	}

	// computing center of points for zero centering
	void Compute_Center(float *center)
	{
		double *tCenter = new double [dim];
		SetVector_Val<double>( tCenter , dim , 0.0 );
		for(int i=0;i<nP;i++)
		{
			for(int k=0;k<dim;k++)
			{
				tCenter[k] += d[i][k];
			}
		}
		for(int k=0;k<dim;k++)
		{
			tCenter[k] /= (double)(nP);
			center[k] = (float)( tCenter[k] );
		}
		delete [] tCenter;
	}

	void ReleaseMem()
	{
		for(int i=0;i<nP;i++)
		{
			delete [] d[i];
		}
		delete [] d;
	}

	
};