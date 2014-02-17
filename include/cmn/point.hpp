#pragma once

namespace cmn
{

template< typename T, int D>
struct point_tt
{        
        typedef T                       value_type;
        static int const dimension =    D;

};
 
template< typename T >
struct point_tt<T, 2>
{
        typedef T                       value_type;
        static int const dimension =    2;
        T       x,y;
        
        point_tt(){}
        point_tt( value_type _x, value_type _y ):x(_x),y(_y){}
        
        bool operator == ( point_tt const & other ) const { return x == other.x && y == other.y; }
        bool operator != ( point_tt const & other ) const { return x != other.y || y != other.y; }
        
        point_tt operator+ (point_tt const & other) const { return point_tt( x+other.x, y+other.y ); }
        point_tt operator- (point_tt const & other) const { return point_tt( x-other.x, y-other.y ); }                
};

typedef point_tt<int,   2> point2i_t;
typedef point_tt<float, 2> point2f_t;


        
} //namespace cmn