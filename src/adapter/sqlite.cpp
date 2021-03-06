#include <adapter/sqlite.hpp>
#include <cmn/log.hpp>

namespace adapter 
{

sqlite_db_t::~sqlite_db_t()
{
        if(this->db_handle)
                sqlite3_close_v2(this->db_handle);
}
        
sqlite_db_pt    sqlite_db_create( std::string const & dbpath, open_mode_et mode )
{
        sqlite_db_pt db(new sqlite_db_t);
        db->db_handle = 0;
        
        int imode = mode == open_mode_read ? SQLITE_OPEN_READONLY : (SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);        
        int ret = sqlite3_open_v2( dbpath.c_str(), &db->db_handle, imode, 0 );
        if( ret != SQLITE_OK )
        {
                cmn::log_and_throw<sqlite_exception>("adapter::sqlite_db_create - error: can't open database ret: %d", ret);
        }
        
        return db;
}

void            sqlite_db_query(sqlite_db_pt const & db, char const * szQuery )
{
        char * err = 0;
        int ret = sqlite3_exec( db->db_handle, szQuery, 0, 0, &err );
        if( ret != SQLITE_OK )
        {
                std::string err_msg;
                if( err )                
                        err_msg = err, sqlite3_free(err);
                cmn::log_and_throw<sqlite_exception>("adapter::sqlite_db_query - error: ret: %d; err_msg: %s", ret, err_msg.c_str());                                                        
        }
}


sqlite_query_t::~sqlite_query_t()
{
        if( this->stm_handle )
                sqlite3_finalize( this->stm_handle );                
}

sqlite_query_pt sqlite_query_create( sqlite_db_pt const & db, char const * szQuery )
{
        sqlite_query_pt q( new sqlite_query_t );
        q->db = db;
        q->stm_handle = 0;
        int ret = sqlite3_prepare_v2( db->db_handle, szQuery, -1, &q->stm_handle, 0 );
        if( ret != SQLITE_OK )
        {
                char const * err_msg = sqlite3_errmsg( q->db->db_handle );
                cmn::log_and_throw<sqlite_exception>("adapter::sqlite_query_create - error: ret: %d, err_msg: %s", ret, err_msg);
        }
        return q;        
}

//or bind
//index here is 1-based! [sqlite requirement]
void sqlite_query_embed(sqlite_query_pt const & q, void const * buf, std::type_info const & ti, int one_based_index)
{
        int ret = SQLITE_ERROR;
        
        if( ti == typeid(blob_t) )        
                ret = sqlite3_bind_blob( q->stm_handle, one_based_index, ((blob_t const*)buf)->ptr, (int)((blob_t const*)buf)->size, SQLITE_TRANSIENT );
        else if( ti == typeid(int) )
                ret = sqlite3_bind_int( q->stm_handle, one_based_index, *(int const*)buf );
        else if( ti == typeid(int64_t) )
                ret = sqlite3_bind_int64( q->stm_handle, one_based_index, *(int64_t const*)buf );
        else if( ti == typeid(std::string) )
                ret = sqlite3_bind_text( q->stm_handle, one_based_index, ((std::string const*)buf)->c_str(), (int)((std::string const*)buf)->size(), SQLITE_TRANSIENT );
        else if( ti == typeid(double) )
                ret = sqlite3_bind_double( q->stm_handle, one_based_index, *(double const*)buf );
        else if( ti == typeid(float) )
                ret = sqlite3_bind_double( q->stm_handle, one_based_index, *(float const*)buf );
        else
        {
                cmn::log_and_throw<sqlite_exception>("adapter::sqlite_query_embed - type: %s is not supported for binding/embedding", ti.name() );
        }
        
        
        if( ret != SQLITE_OK )
        {
                char const * err_msg = sqlite3_errmsg( q->db->db_handle );
                cmn::log_and_throw<sqlite_exception>("adapter::sqlite_query_embed - error: ret: %d, err_msg: %s", ret, err_msg);
        }        
}

bool sqlite_query_next( sqlite_query_pt const & q )
{
        int ret = sqlite3_step( q->stm_handle );
        switch( ret )
        {
                case SQLITE_ROW: return true;
                case SQLITE_DONE: return false;
                default:
                {
                        char const * err_msg = sqlite3_errmsg( q->db->db_handle );
                        cmn::log_and_throw<sqlite_exception>("adapter::sqlite_query_next - error: ret: %d, err_msg: %s", ret, err_msg);
                }       
        };
        return false;
}

void sqlite_query_column( void * ptr, sqlite_query_pt const & q, std::type_info const & ti, int column  )
{
        sqlite3_stmt * stm_handle = q->stm_handle;
        if( ti == typeid(blob_t) )        
        {
                blob_t * b = (blob_t*)ptr;
                b->ptr = (void*)sqlite3_column_blob( stm_handle, column );
                b->size = sqlite3_column_bytes( stm_handle, column );
        }
        else if( ti == typeid(std::string) )      
        {
                int len = sqlite3_column_bytes( stm_handle, column );
                if( len )                
                        (*(std::string*)ptr) = std::string( (char const*)sqlite3_column_text( stm_handle, column ), len );
                else
                        (*(std::string*)ptr).clear();                                
        }
        else if( ti == typeid(int64_t) )        
                (*(int64_t*)ptr) = sqlite3_column_int64( stm_handle, column );        
        else if( ti == typeid(int) )        
                (*(int*)ptr) = sqlite3_column_int( stm_handle, column );        
        else if( ti == typeid(double) )        
                (*(double*)ptr) = sqlite3_column_double( stm_handle, column );
        else
                cmn::log_and_throw<sqlite_exception>("adapter::sqlite_query_fetch - type: %s for fetching from db is not supported", ti.name() );
}

bool sqlite_query_reset( sqlite_query_pt const & q )
{
	int res = sqlite3_reset( q->stm_handle );
	
	return res == SQLITE_OK;
}

}