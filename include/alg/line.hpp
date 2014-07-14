#pragma once
#include "cmn/point.hpp"
#include "cmn/utils.hpp"
#include <math.h>


namespace alg
{
        
inline float near_zero( float a )
{
        //return fabs(a) < 0.001f;
        return fabs(a) < 0.03f; //01f;
}

//resolve relative to r or x or Luminance (in L*a*b)
//@return a,b,c of equation x = a + by + cz [plane]
inline bool
linear_least_squares_plane_x( cmn::point3f_t * plane, cmn::point3f_t const * pts, int const size )
{                        
        using cmn::point3f_t;
        using cmn::accumulate;
        
        float const sy =  accumulate<float>( pts, size, [](point3f_t const & a, float b){ return a.y + b; } );
        float const sz =  accumulate<float>( pts, size, [](point3f_t const & a, float b){ return a.z + b; } );
        float const syy = accumulate<float>( pts, size, [](point3f_t const & a, float b){ return a.y*a.y + b; } );
        float const syz = accumulate<float>( pts, size, [](point3f_t const & a, float b){ return a.y*a.z + b; } );
        float const szz = accumulate<float>( pts, size, [](point3f_t const & a, float b){ return a.z*a.z + b; } );
        float const sx =  accumulate<float>( pts, size, [](point3f_t const & a, float b){ return a.x + b; } );
        float const syx = accumulate<float>( pts, size, [](point3f_t const & a, float b){ return a.y*a.x + b; } );
        float const szx = accumulate<float>( pts, size, [](point3f_t const & a, float b){ return a.z*a.x + b; } );
        
        //plane: x = a + by + cz
        //solve equation
        // |a| = |n  sy  sz |-1  |sx | 
        // |b| = |sy syy syz|  * |syx|
        // |c| = |sz syz szz|    |szx|
        
        //finding determinant        
        float const det = size * syy * szz - sz*syy*sz - syz*syz*size - sy*sy*szz + 2.f*sy*syz*sz;
                
        if( !near_zero(det) )
        {
                float const a11 = syy*szz - syz*syz;
                float const a22 = size*szz - sz*sz;
                float const a33 = size*syy - sy*sy;
                float const a12 = sy*szz - sz*syz;            //a21 as well
                float const a13 = sy*syz - sz*syy;            //a31 as well
                float const a23 = size*syz - sz*sy;           //a32 as well
                
                float const im[3][3] =           //inverse matrix
                {
                        { a11/det, a12/det, a13/det },
                        { a12/det, a22/det, a23/det },
                        { a13/det, a23/det, a33/det }
                };
                
                float const a = im[0][0]*sx + im[0][1]*syx + im[0][2]*szx;
                float const b = im[1][0]*sx + im[1][1]*syx + im[1][2]*szx;
                float const c = im[2][0]*sx + im[2][1]*syx + im[2][2]*szx;
                *plane = cmn::point3f_t(a,b,c);
                return true;
        }
        else        
        {
                *plane = cmn::point3f_t(0.f,0.f,0.f);
                return false;
        }                                        
}        

//resolve relative to a or y or Luminance (in L*a*b)
//@return a,b,c of equation y = a + bx + cz [plane]
inline bool
linear_least_squares_plane_y( cmn::point3f_t * plane, cmn::point3f_t const * pts, int const size )
{                        
        using cmn::point3f_t;
        using cmn::accumulate;
        
        float const sx =  accumulate<float>( pts, size, [](point3f_t const & a, float b){ return a.x + b; } );
        float const sz =  accumulate<float>( pts, size, [](point3f_t const & a, float b){ return a.z + b; } );
        float const sxx = accumulate<float>( pts, size, [](point3f_t const & a, float b){ return a.x*a.x + b; } );
        float const szx = accumulate<float>( pts, size, [](point3f_t const & a, float b){ return a.z*a.x + b; } );
        float const szz = accumulate<float>( pts, size, [](point3f_t const & a, float b){ return a.z*a.z + b; } );
        float const sy =  accumulate<float>( pts, size, [](point3f_t const & a, float b){ return a.y + b; } );
        float const sxy = accumulate<float>( pts, size, [](point3f_t const & a, float b){ return a.x*a.y + b; } );
        float const szy = accumulate<float>( pts, size, [](point3f_t const & a, float b){ return a.z*a.y + b; } );
        
        //plane: x = a + by + cz
        //solve equation
        // |a| = |n  sx  sz |-1  |sy | 
        // |b| = |sx sxx sxz|  * |sxy|
        // |c| = |sz sxz szz|    |szy|
        
        //finding determinant        
        float const det = size * sxx * szz - sz*sxx*sz - szx*szx*size - sx*sx*szz + 2.f*sx*szx*sz;
                
        if( !near_zero(det) )
        {
                float const a11 = sxx*szz - szx*szx;
                float const a22 = size*szz - sz*sz;
                float const a33 = size*sxx - sx*sx;
                float const a12 = sx*szz - sz*szx;            //a21 as well
                float const a13 = sx*szx - sz*sxx;            //a31 as well
                float const a23 = size*szx - sz*sx;           //a32 as well
                
                float const im[3][3] =           //inverse matrix
                {
                        { a11/det, a12/det, a13/det },
                        { a12/det, a22/det, a23/det },
                        { a13/det, a23/det, a33/det }
                };
                
                float const a = im[0][0]*sy + im[0][1]*sxy + im[0][2]*szy;
                float const b = im[1][0]*sy + im[1][1]*sxy + im[1][2]*szy;
                float const c = im[2][0]*sy + im[2][1]*sxy + im[2][2]*szy;
                *plane = cmn::point3f_t(a,b,c);
                return true;
        }
        else        
        {
                *plane = cmn::point3f_t(0.f,0.f,0.f);
                return false;
        }                                        
}

//resolve relative to b or z or Luminance (in L*a*b)
//@return a,b,c of equation z = a + bx + cy [plane]
inline bool
linear_least_squares_plane_z( cmn::point3f_t * plane, cmn::point3f_t const * pts, int const size )
{                        
        using cmn::point3f_t;
        using cmn::accumulate;
        
        float const sx =  accumulate<float>( pts, size, [](point3f_t const & a, float b){ return a.x + b; } );
        float const sy =  accumulate<float>( pts, size, [](point3f_t const & a, float b){ return a.y + b; } );
        float const sxx = accumulate<float>( pts, size, [](point3f_t const & a, float b){ return a.x*a.x + b; } );
        float const sxy = accumulate<float>( pts, size, [](point3f_t const & a, float b){ return a.x*a.y + b; } );
        float const syy = accumulate<float>( pts, size, [](point3f_t const & a, float b){ return a.y*a.y + b; } );
        float const sz =  accumulate<float>( pts, size, [](point3f_t const & a, float b){ return a.z + b; } );
        float const sxz = accumulate<float>( pts, size, [](point3f_t const & a, float b){ return a.x*a.z + b; } );
        float const syz = accumulate<float>( pts, size, [](point3f_t const & a, float b){ return a.y*a.z + b; } );
        
        //plane: z = a + bx + cy
        //solve equation
        // |a| = |n  sx  sy |-1  |sz | 
        // |b| = |sx sxx sxy|  * |sxz|
        // |c| = |sy sxy syy|    |syz|
        
        //finding determinant        
        float const det = size * sxx * syy - sy*sxx*sy - sxy*sxy*size - sx*sx*syy + 2.f*sx*sxy*sy;
                
        if( !near_zero(det) )
        {
                float const a11 = sxx*syy - sxy*sxy;
                float const a22 = size*syy - sy*sy;
                float const a33 = size*sxx - sx*sx;
                float const a12 = sx*syy - sy*sxy;            //a21 as well
                float const a13 = sx*sxy - sy*sxx;            //a31 as well
                float const a23 = size*sxy - sy*sx;           //a32 as well
                
                float const im[3][3] =           //inverse matrix
                {
                        { a11/det, a12/det, a13/det },
                        { a12/det, a22/det, a23/det },
                        { a13/det, a23/det, a33/det }
                };
                
                float const a = im[0][0]*sz + im[0][1]*sxz + im[0][2]*syz;
                float const b = im[1][0]*sz + im[1][1]*sxz + im[1][2]*syz;
                float const c = im[2][0]*sz + im[2][1]*sxz + im[2][2]*syz;
                *plane = cmn::point3f_t(a,b,c);
                return true;
        }
        else        
        {
                *plane = cmn::point3f_t(0.f,0.f,0.f);
                return false;
        }                                        
} 

//on input: two planes gotten from linear_least_squares_plane_x and linear_least_squares_plane_z
//on output: line equation in form a = p + dir*t; where p - is any point, dir - direction: normalized (length == 1) vector
inline bool
line_parametric_from_planes_yz( cmn::point3f_t * op, cmn::point3f_t * odir, 
                                cmn::point3f_t const plane_y, cmn::point3f_t const plane_z )
{
        float const a2 = plane_y.x; float const b2 = plane_y.y; float const c2 = plane_y.z;
        float const a3 = plane_z.x; float const b3 = plane_z.y; float const c3 = plane_z.z;

        float const zinv = 1-c3*c2;
        float const xinv = (b3 + c3*b2);
        float const xinvzinv = xinv * zinv;
        
        if( !near_zero(xinvzinv) )
        {        
                op->x = 0;
                op->y = a2 + c2*(a3 + c3*a2)/zinv;
                op->z = (a3 + c3*a2)/zinv;
                
                odir->x = 1.f / xinv;
                odir->y = (b2 + c2*b3) / xinvzinv;
                odir->z = 1.f / zinv;        
                
                float length = sqrtf(odir->sq());
                if( !near_zero( length ) )
                {
                        *odir /= length;
                        return true;
                }
                else                
                        return false;                                                
        }
        else
                return false;                
}


//on input: two planes gotten from linear_least_squares_plane_x and linear_least_squares_plane_z
//on output: line equation in form a = p + dir*t; where p - is any point, dir - direction: normalized (length == 1) vector
inline bool
line_parametric_from_planes_xz( cmn::point3f_t * op, cmn::point3f_t * odir, 
                                cmn::point3f_t const plane_x, cmn::point3f_t const plane_z )
{
        cmn::point3f_t dir = plane_x.vec(plane_z);                
        float const denom = plane_z.y*plane_x.z - 1;
        
        if( near_zero(denom) )
                return false;
        
        //find a dot supposing fact that our plane is a good one 
        float const z = (-plane_z.x - plane_z.y*plane_x.x)/denom;
        float const x = plane_x.x + plane_x.z*z;
        
        *op = cmn::point3f_t(x, 0, z);
        
        float const dlen = sqrtf(dir.sq());
        
        if( !near_zero(dlen) )
                return *odir = dir/dlen, true;
        else
                return false;        
}


//@a - point distance for which we a looking for
//@p - dot on a line
//@d - normalized line direction (length of d == 1)
//@return - dot on p + dt line which makes perpendicular with 'a' point
inline cmn::point3f_t 
distance_point2line_pt( cmn::point3f_t const & a, cmn::point3f_t const & p, cmn::point3f_t const & d )
{
        cmn::point3f_t const v1 = p - a;
        cmn::point3f_t const dv = v1 - d*v1.dot(d);        
        return a + dv;        
}

//@a - point distance for which we a looking for
//@p - dot on a line
//@d - normalized line direction (length of d == 1)
inline float 
distance_point2line( cmn::point3f_t const & a, cmn::point3f_t const & p, cmn::point3f_t const & d )
{
        cmn::point3f_t const v1 = p - a;
        cmn::point3f_t const dv = v1 - d*v1.dot(d);        
        float const length = sqrtf(dv.sq());
        return length;
}


enum part_et
{
        part_s0,
        part_s1,
        part_line
};

inline float
distance_point2segment( part_et * part, cmn::point3f_t const p, cmn::point3f_t s0, cmn::point3f_t s1)
{
        cmn::point3f_t const v = s1-s0;
        cmn::point3f_t const w = p - s0;
        
        float const c1 = w.dot(v);
        if ( c1 <= 0 )
                return *part=part_s0, sqrtf( p.distance_sq(s0) );

        float const c2 = v.dot(v);
        if ( c2 <= c1 )
                return *part=part_s1, sqrtf(p.distance_sq(s1));

        float const b = c1 / c2;
        cmn::point3f_t pb = s0 + v*b;
        return *part=part_line, sqrtf(p.distance_sq(pb));
}

        
}