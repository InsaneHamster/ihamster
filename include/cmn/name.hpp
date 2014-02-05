#pragma once
#include <string>
#include <string.h>
//#include <functional>

namespace cmn
{

struct name_t
{
	enum
	{
		c_chars = 16
	};
	
	union
	{
		uint64_t namei[2];
		char	 name[c_chars];
	};

	bool operator == ( name_t const & other ) const { return namei[0]==other.namei[0] && namei[1]==other.namei[1];}
	bool operator != ( name_t const & other ) const { return namei[0]!=other.namei[0] || namei[1]!=other.namei[1];}
	bool operator <  ( name_t const & other ) const //suitable for std::set,map
	{ 
		return namei[0] < other.namei[0] ? true : namei[0] > other.namei[0] ? false : namei[1] < other.namei[1];
	}
};


inline name_t name_create( char const * szStr )
{
	name_t n;
	strncpy( n.name, szStr, name_t::c_chars );
	return n;
}

inline std::string name_print( name_t const & n )
{ 
	return n.name[name_t::c_chars-1] == 0 ? std::string(n.name) : std::string(n.name, name_t::c_chars); 
}

}	//end of namespace cmn


//providing hash for the name_t
namespace std {
template< typename T > struct hash;

template<>
struct hash<cmn::name_t> 
{
	size_t operator()(cmn::name_t const & s) const
	{
		return s.namei[0] + s.namei[1];
	}
};
} //of std
