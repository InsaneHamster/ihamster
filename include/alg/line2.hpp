#pragma once
#include "cmn/point.hpp"
#include "cmn/utils.hpp"
#include <math.h>

namespace alg
{
        
template<typename T>
T matrix_determinant( T const (&matrix)[2][2] )
{
        return matrix[0][0]*matrix[1][1] - matrix[0][1]*matrix[1][0];
}

template<typename T>
T matrix_determinant( T const (&matrix)[1][1] )
{
        return matrix[0][0];
}
        
template<typename T, int N> 
T matrix_determinant( T const (&m)[N][N] )
{
        T sum = 0;
        for( int i = 0; i < N; ++i )
        {
                T s[N-1][N-1];
                T factor = m[0][i];
                if(factor)
                {                                                
                        for( int si = 1; si < N; ++si )
                        {
                                int sjw = 0;
                                for( int sj = 0; sj < N; ++sj )
                                {
                                        if( sj != i )
                                        {
                                                s[si-1][sjw] = m[si][sj];
                                                ++sjw;
                                        }
                                
                                }
                        }      
                        if( i & 1 ) factor = -factor;                        
                        sum += factor * matrix_determinant(s);
                }                                
        }
        return sum;
}

template<typename T, int N>
T matrix_minor( T const (&im)[N][N], int const i, int const j )
{
        T m[N-1][N-1];
        
        int siw = 0;
        for( int si = 0; si < N; ++si )
        {
                if( si != i )
                {
                        int sjw = 0;
                        for( int sj = 0; sj < N; ++sj )
                        {
                                if( sj != j )
                                {
                                        m[siw][sjw] = im[si][sj];
                                        ++sjw;
                                }
                        }                        
                        ++siw;
                }
        }        
        return matrix_determinant(m);
}

template<typename T, int N>
void matrix_transpose( T (*om)[N][N], T const (&im)[N][N] )
{
        for( int i = 0; i < N; ++i )        
                for( int j = 0; j < N; ++j )
                        (*om)[i][j] = im[j][i];        
}


template<typename T, int N>
bool matrix_invert( T (*om)[N][N], T const (&im)[N][N] )
{
        T m[N][N];                
        T const det = matrix_determinant( im );
        
        if( det == T(0) )                       //TODO: put near_zero here...
                return false;
        
        T const inv_det = T(1)/det;
        
        for( int i = 0; i < N; ++i )
        {
                for( int j = 0; j < N; ++j )
                {
                        T val = matrix_minor( im, i, j );
                        if( (i + j)&1 ) val = -val;
                        m[i][j] = val * inv_det;
                }
        }        
        matrix_transpose( om, m );
        return true;
}

#if 0
template<typename T, int N, int M, int K>
void matrix_multiply( T (*om)[N][K], T const (&m)[N][M], T const (&c)[M][K] )
{
}
#endif
template<typename T, int N>
void matrix_column_multiply( T (*r)[N], T const (&m)[N][N], T const (&c)[N] )
{        
        for( int i = 0; i < N; ++i )
        {
                T sum = 0;
                for( int j = 0; j < N; ++j )
                {
                        sum += m[i][j]*c[j];
                }
                (*r)[i] = sum;
        }
}

        
// t belongs to range [0 ... size )
// x,y,z of pts - actually coords
//@op - [out] point on a line
//@odir - [out] direction of the line
inline bool
linear_least_squares_parametic( cmn::point3f_t * op, cmn::point3f_t * dir, 
                                cmn::point3f_t const * pts, int const size )
{

/*we get small matrix:
|ax|    | n     st      0       0       0       0  |-1    spx
|bx|    | st    stt     0       0       0       0  |      spx*t
|ay|    | 0     0       n       st      0       0  |   *  spy
|by| =  | 0     0       st      stt     0       0  |      spy*t
|az|    | 0     0       0       0       n       st |      spz
|bz|    | 0     0       0       0       st      stt|      spz*t

solve the system and we get an ideal line :)
*/
        typedef double real_t;
        using cmn::accumulate;
        using cmn::point3f_t;
        
                
        real_t const n = (real_t)size;
        real_t t = 0;
        real_t const st = n/2*(n-1);
        real_t stt = 0;
        
        auto stt_compute = [t, &stt]() mutable -> void { stt += t*t; ++t; };
        for( int i = 0; i < n; ++i ) stt_compute();
        
        real_t const spx = accumulate<double>( pts, size, [](point3f_t const & a, real_t b){ return a.x + b; } );
        real_t const spxt = accumulate<double>( pts, size, [t](point3f_t const & a, real_t b) mutable { return a.x*(t++) + b; } );
        real_t const spy = accumulate<double>( pts, size, [](point3f_t const & a, real_t b){ return a.y + b; } );
        real_t const spyt = accumulate<double>( pts, size, [t](point3f_t const & a, real_t b) mutable { return a.y*(t++) + b; } );
        real_t const spz = accumulate<double>( pts, size, [](point3f_t const & a, real_t b){ return a.z + b; } );
        real_t const spzt = accumulate<double>( pts, size, [t](point3f_t const & a, real_t b) mutable { return a.z*(t++) + b; } );
        
        real_t const m[6][6] = 
        {
                {n,     st,     0,      0,      0,      0},
                {st,    stt,    0,      0,      0,      0},
                {0,     0,      n,      st,     0,      0},
                {0,     0,      st,     stt,    0,      0},
                {0,     0,      0,      0,      n,      st},
                {0,     0,      0,      0,      st,     stt}
        };
        real_t mi[6][6];
        
        bool ok = matrix_invert( &mi, m );
        if( !ok )
                return false;
        
        real_t const c[6] = {spx, spxt, spy, spyt, spz, spzt};
        real_t r[6];
        
        
        matrix_column_multiply(&r, mi, c);
        
        op->x = r[0]; op->y = r[2]; op->z = r[4];
        dir->x = r[1]; dir->y = r[3]; dir->z = r[5];  
        
        real_t length_sq = dir->sq();
        if( length_sq )
        {
                *dir /= sqrtf(length_sq);
                return true;
        }
        else
                return false;
}



} //end of namespace alg