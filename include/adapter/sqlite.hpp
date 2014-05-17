#pragma once
#include <sqlite3.h>
#include <memory>
#include <typeinfo>
#include <stdexcept>

namespace adapter
{

struct sqlite_exception : public std::runtime_error
{
        sqlite_exception( std::string const & err ) : std::runtime_error(err){}        
};
        
        
struct sqlite_db_t
{        
        sqlite3 * db_handle;
                
        ~sqlite_db_t();
};
        
typedef std::shared_ptr<sqlite_db_t> sqlite_db_pt;
        
enum open_mode_et
{
        open_mode_read,
        open_mode_read_write
};

sqlite_db_pt    sqlite_db_create( std::string const & dbpath, open_mode_et mode );
void            sqlite_db_query(sqlite_db_pt const & db, char const * szQuery );                //without result...


struct sqlite_query_t
{
        sqlite_db_pt    db;
        sqlite3_stmt *  stm_handle;
        
        ~sqlite_query_t();
};
typedef std::shared_ptr<sqlite_query_t> sqlite_query_pt;

struct blob_t
{
        void * ptr;
        size_t size;
};


sqlite_query_pt sqlite_query_create( sqlite_db_pt const & db, char const * szQuery );

//or bind
//index here is 1-based! [sqlite requirement]
//buf points to blob or int... or see below in sqlite_query_column
void sqlite_query_embed(sqlite_query_pt const & q, void const * buf, std::type_info const & ti, int one_based_index );

template<typename T> void
sqlite_query_embed(sqlite_query_pt const & q, T const &t, int one_based_index )
{
     sqlite_query_embed( q, &t, typeid(T), one_based_index );   
}


//@return - true - go on, false - the end
bool sqlite_query_next( sqlite_query_pt const & q );

//types supported fot std::type_info const & ti:
//blob_t - blob. in this case ptr should be void** and pointer returned will be valid until next call of sqlite_query_next
//std::string, int64_t, double
//@column - zero based
void sqlite_query_column( void * ptr, sqlite_query_pt const & q, std::type_info const & ti, int column  );

//@return - true - go on, false - the end
template<typename T> void 
sqlite_query_column( T * t, sqlite_query_pt const & q, int column )
{        
        sqlite_query_column((void*)t, q, typeid(T), column );
}

//to reuse
bool sqlite_query_reset( sqlite_query_pt const & q );

}