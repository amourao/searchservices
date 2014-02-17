#pragma once

#include "Common.h"
#include "Points.h"

class LSH
{
public :


	// hamming distance function
	__forceinline int Compute_HD(bitset<MAX_BCODE_LEN> &a, bitset<MAX_BCODE_LEN> &b)
	{
		return ( ( a ^ b ).count() );
	}

	// spherical hamming distance function
	__forceinline double Compute_SHD(bitset<MAX_BCODE_LEN> &a, bitset<MAX_BCODE_LEN> &b)
	{
		return ( ( (double)( ( a ^ b ).count() ) ) / ( (double)( ( a & b ).count() - (MAX_BCODE_LEN - bCodeLen)) + 0.1 ));
	}

	int dim;
	int bCodeLen;
	REAL_TYPE **pM;

	void Initialize(int _dim, int _bCodeLen)
	{
		dim = _dim;
		bCodeLen = _bCodeLen;
		pM = new REAL_TYPE * [ dim ];
		for(int k=0;k<dim;k++)
		{
			pM[k] = new REAL_TYPE [ bCodeLen ];
			for(int i=0;i<bCodeLen;i++)
			{
				pM[k][i] = Rand_Gaussian<REAL_TYPE>();
			}
		}
	}

	__forceinline void Compute_BCode(REAL_TYPE *x, bitset<MAX_BCODE_LEN> &y)
	{
		REAL_TYPE tmp;
		for(int i=0;i<bCodeLen;i++)
		{
			tmp = 0.0;
			for(int k=0;k<dim;k++)
			{
				tmp += x[k] * pM[k][i];
			}
			if( tmp > 0.0 )
			{
				y[i] = 1;
			}
			else
			{
				y[i] = 0;
			}
		}
	}
};

class Index_Distance
{
public :
	bool flag;
	int index;
	REAL_TYPE dist, distSq;
	bool operator < (const Index_Distance &T) const
	{
		if( this->distSq < T.distSq )	{			return true;		}
		return false;
	}
};

class Sphere
{
public :
	REAL_TYPE *c, r, rSq;

	void Initialize(int _dim)
	{
		c = new REAL_TYPE [ _dim ];
		r = 0.0;		rSq = 0.0;
	}

	// function to set radius to include desired portion of training set
	void Set_Radius(Points *ps, Index_Distance *ids, float incRatio);
};

class SphericalHashing
{
public :
	Points *ps;

	// training set
	Points tps;

	Sphere *s;

    int dim;
	int bCodeLen;
	int trainSamplesCount;
	float incRatio;
	float overRatio;
	float epsMean;
	float epsStdDev;
	int maxItr;

	Index_Distance **ids;
	bitset<MAX_NUM_TRAIN_SAMPLES> *table;

	void Initialize(Points *_ps, int _dim, int _bCodeLen, int _trainSamplesCount, int _maxItr,
    float _incRatio, float _overRatio, float _epsMean, float _epsStdDev);
	void Compute_Table();
	void Compute_Num_Overlaps(int **overlaps);
	void Set_Spheres();

	void ReleaseMem();

	__forceinline void Compute_BCode(REAL_TYPE *x, bitset<MAX_BCODE_LEN> &y)
	{
		for(int i=0;i<bCodeLen;i++)
		{
			if( Compute_Distance_L2Sq<REAL_TYPE>( s[i].c , x , ps->dim ) > s[i].rSq )
			{
				y[i] = 0;
			}
			else
			{
				y[i] = 1;
			}
		}
	}
};
