#pragma once
#include "point.hpp"

namespace cmn
{

template< typename T, int D >
struct rect_tt
{
        typedef T                       value_type;
        static int const dimension =    D;
        typedef point_tt<T,D>           point_t;

        point_t origin;
        point_t size;                

};
        
template<typename T>
struct rect_tt<T, 2>
{
        typedef T                       value_type;
        static int const dimension =    2;
        typedef point_tt<T,dimension>   point_t;
        
        point_t origin;
        point_t size;                

        rect_tt(){}
        rect_tt( point_t const & _origin, point_t const & _size ) : origin(_origin), size(_size){}
        rect_tt( T const & x, T const & y, T const & width, T const & height ) : origin(x,y), size(width, height){}
        
        bool operator == (rect_tt const & other) const { return origin == other.origin && size == other.size; }
        bool operator != (rect_tt const & other) const { return origin != other.origin || size != other.size; }
                
        //will fail on unsigned
        bool is_inside( point_t const & pt ) const { point_t s = pt - origin; if( s.x < 0 || s.y < 0 || s.x >= size.x || s.y >= size.y ) return false; else return true; };
};


typedef rect_tt< int, 2  > rect2i_t;
typedef rect_tt< float, 2  > rect2f_t;

};