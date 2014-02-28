#pragma once

#include <cstdio>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <algorithm>

#define __forceinline __attribute__((always_inline))

#include "Parameters.h"
#include "Stopwatch.hpp"

using namespace std;

#define		PI				3.1415926536

int Rand_NDigits(int nDigits);
int Rand_Uniform_Int(int minVal, int maxVal);

template<typename RealType>
RealType Rand_Uniform(RealType minVal, RealType maxVal)
{
	return ( minVal + ( maxVal - minVal ) * ( (RealType)rand() / (RealType)(RAND_MAX) ) );
}

template<typename RealType>
RealType Rand_Gaussian()
{
	RealType x1, x2, ret;
	do
	{
		x1 = Rand_Uniform<RealType>( 0.0 , 1.0 );
	} while( x1 == 0 );
	x2 = Rand_Uniform<RealType>( 0.0 , 1.0 );
	ret = sqrt( -2.0 * log( x1 ) ) * cos( 2.0 * PI * x2 );
	return ret;
}

template<typename RealType>
RealType Compute_Distance_L2Sq(RealType *v0, RealType *v1, int dim)
{
	RealType ret = 0.0;
	for(int i=0;i<dim;i++)
	{
		ret += ( v0[i] - v1[i] ) * ( v0[i] - v1[i] );
	}
	return ret;
}

template<typename RealType>
void SetVector_Val(RealType *vec, int dim, RealType val)
{
	for(int i=0;i<dim;i++)
	{
		vec[i] = val;
	}
}

template<typename RealType>
void SetVector_Vec(RealType *dest, RealType *src, int dim)
{
	for(int i=0;i<dim;i++)
	{
		dest[i] = src[i];
	}
}

template<typename RealType>
void Scalar_Vector(RealType *A, RealType s, int dim)
{
	for(int i=0;i<dim;i++)
	{
		A[i] = s * A[i];
	}
}

template<typename RealType>
void Sub_Vector(RealType *A, RealType *B, RealType *ret, int dim)
{
	for(int i=0;i<dim;i++)
	{
		ret[i] = A[i] - B[i];
	}
}


template<typename RealType>
void Add_Vector(RealType *A, RealType *B, RealType *ret, int dim)
{
	for(int i=0;i<dim;i++)
	{
		ret[i] = A[i] + B[i];
	}
}

template<typename RealType>
void SetMatrix_Val(RealType **mat, int nY, int nX, RealType val)
{
	for(int i=0;i<nY;i++)
	{
		for(int j=0;j<nX;j++)
		{
			mat[i][j] = val;
		}
	}
}
