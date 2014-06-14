#pragma once
#include <type_traits>
#include <stdint.h>

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
        
        point_tt operator/ (T const& d) const { return point_tt( x/d, y/d ); }
        point_tt operator* (T const& m) const { return point_tt( x*m, y*m ); }
        
        T distance_sq( point_tt const & p ) const { T dx = x-p.x; T dy = y - p.y; return dx*dx + dy*dy; }
        T sq() const { return x*x + y*y; };
};


template< typename T >
struct point_tt<T, 3>
{
        typedef T                                               value_type;        
        static int const dimension =                            3;

        template<typename Z, typename E=void> 
        struct deduce_stype
        {
                typedef Z                                       type;
        };

        template<typename Z> 
        struct deduce_stype<Z, typename std::enable_if< std::is_integral<Z>::value >::type >
        {
                typedef typename std::make_signed<Z>::type      type;
        };
        
        typedef typename deduce_stype<T>::type                  svalue_type;
        typedef point_tt<svalue_type, dimension>                spoint_t;
        
        union
        {
                struct{ value_type       x,y,z; };
                struct{ value_type       r,g,b; };                
        };
        
        
        point_tt(){}
        point_tt( value_type _x, value_type _y, value_type _z ):x(_x),y(_y),z(_y){}
        point_tt( spoint_t const & p ) : x(p.x), y(p.y), z(p.z) {};
        
        T distance_sq( point_tt const & p ) const { T dx = x-p.x; T dy = y - p.y; T dz = z - p.z; return dx*dx + dy*dy + dz*dz; }
        T sq() const { return x*x + y*y + z*z; };
        
        bool operator == ( point_tt const & other ) const { return x == other.x && y == other.y && z == other.z; }
        bool operator != ( point_tt const & other ) const { return x != other.y || y != other.y || z != other.z; }
        
        point_tt operator+ ( point_tt const & other ) const { return point_tt( x+other.x, y+other.y, z+other.z ); }
        spoint_t operator- ( point_tt const & other ) const { return spoint_t( x-other.x, y-other.y, z-other.z ); }
        
        point_tt operator/ ( value_type const v ) const { return point_tt( x/v, y/v, z/v ); }
        
        point_tt const & operator+= ( point_tt const & other ) { x += other.x; y += other.y; z += other.z; return *this; }
};


template< typename T >
struct point_tt<T, 4>
{
        typedef T                                               value_type;        
        static int const dimension =                            4;

        typedef typename std::make_signed<T>::type              svalue_type;
        typedef point_tt<svalue_type, dimension>                spoint_t;

        union
        {
                struct{ value_type       x,y,z,w; };
                struct{ value_type       r,g,b,a; };
        };
        
        point_tt(){}
        explicit point_tt(point_tt<T, 3> const & pt3) : x(pt3.x), y(pt3.y), z(pt3.z), w(value_type(1)){}
        point_tt( value_type _x, value_type _y, value_type _z, value_type _w ) : x(_x), y(_y), z(_z), w(_w){}
        
        template< typename T2 > 
        point_tt operator * ( T2 const & v ) const { return point_tt(T(x*v), T(y*v), T(z*v), T(w*v)); }
        
        template< typename T2 > 
        point_tt operator / ( T2 const & v ) const { return point_tt(T(x/v), T(y/v), T(z/v), T(w/v)); }
        
        point_tt & operator+= ( point_tt const & p ) { x+=p.x; y+=p.y; z+=p.z; w+=p.w; return *this; }
        
};

typedef point_tt<int,   2>      point2i_t;
typedef point_tt<uint16_t, 2>   point2w_t;
typedef point_tt<int16_t, 2>    point2s_t;
typedef point_tt<float, 2>      point2f_t;
typedef point_tt<int,   3>      point3i_t;
typedef point_tt<uint8_t, 3>    point3b_t;
typedef point_tt<int8_t,  3>    point3c_t;
typedef point_tt<float, 3>      point3f_t;
typedef point_tt<float, 3>      color3f_t;
typedef point_tt<uint8_t, 4>    point4b_t;
typedef point_tt<uint8_t, 4>    color4b_t;
typedef point_tt<int8_t,  4>    point4c_t;
typedef point_tt<float, 4>      point4f_t;

        
} //namespace cmn