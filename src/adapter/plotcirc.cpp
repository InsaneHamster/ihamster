#include <adapter/plotcirc.hpp>
#include <adapter/image.hpp>
#include <adapter/sqlite.hpp>
#include <cmn/plotcirc.hpp>
#include <cmn/image.hpp>
#include <cmn/point.hpp>
#include <cmn/log.hpp>
#include <memory.h>
#include <math.h>


namespace adapter 
{
        
        
bool plotcirc_save_to_png( cmn::plotcirc_pt const & pc, char const * const szImgPath )
{
        //what size ot chose ? let it be 256x128! (just for great justice)
        //or precisely plotcirc_discr * 4
        int const scale = 4;
        int const width = cmn::plotcirc_discr * scale * 2, height = cmn::plotcirc_discr * scale;
        cmn::image_pt img_p = cmn::image_create( width, height, cmn::pitch_default, cmn::format_rgba );
        cmn::image_t * img = img_p.get();
        //memset( img->bytes, 0, img->header.pitch * height );            //make it very black and transparent
        //upd: transparent looks bad in most viewers...
        
        cmn::color4b_t const white(255,255,255,255);
        cmn::color4b_t const black(0,0,0,255);
        
        {
                cmn::color4b_t * row_dst_begin = (cmn::color4b_t *)img->bytes;
                cmn::color4b_t * const row_dst_end = img->row<cmn::color4b_t>( height-1 ) + img->header.width;
                while( row_dst_begin < row_dst_end ) *row_dst_begin++ = black;
        }
        
        for( int y = 0; y < height; ++y )
        {
                cmn::color4b_t * const row_dst = img->row<cmn::color4b_t>(y);
                int const row_index_src = y / scale;
                std::vector< cmn::point2f_t > const & row_src = pc->rows[row_index_src];                
                size_t const row_src_size = row_src.size();
                
                //make-half transparent drawing...                
                for( int i = 0; i < row_src_size; ++i )
                {
                        cmn::point2f_t seg = row_src[i];
                        seg = seg * width;
                        
                        //now fill it with white
                        //first and last points draw with help of anti-aliasing                        
                        float fint;
                        float frac = modff( seg.x, &fint );
                        float intensity = 1.f - frac;
                        
                        int pos_write = (int)fint;
                        row_dst[pos_write] = white * intensity;
                        ++pos_write;
                        
                        intensity = modff( seg.y, &fint );
                        int pos_stop = ceilf(seg.y);
                        
                        while( pos_write < pos_stop )                        
                                row_dst[pos_write++] = white;
                        if( pos_stop < width && intensity ) //tail, add to value which is there to correctly handle case when length is less than 1 pixel!
                                row_dst[pos_stop] += white * intensity, row_dst[pos_stop].a = 255;  
                }                
        }
        
        bool ok = adapter::image_save_to_png( img_p, szImgPath );        
        
        return ok;
}

static std::string const g_sqlite_db_name = "ihamster.db";

sqlite_db_pt open_and_create_db( open_mode_et mode )
{
        sqlite_db_pt db = sqlite_db_create( g_sqlite_db_name, mode );
        if( mode == open_mode_read_write )
        {
                sqlite_db_query( db, "CREATE TABLE IF NOT EXISTS plotcircs ( name TEXT, width INTEGER, height INTEGER, value BLOB, num_segments INTEGER )" );
        }
        
        return db;
}

namespace
{
struct row_t
{
        int32_t             num_segments;
        cmn::point2f_t      segs[];
};
}


void plotcirc_db_import_from_sqlite( cmn::plotcirc_db_pt const & pcd, std::string const & db_path )
{
        try
        {
                static char const select_protcircs[] = "SELECT * FROM plotcircs";
                
                sqlite_db_pt db = open_and_create_db( open_mode_read );
                sqlite_query_pt q = sqlite_query_create( db, select_protcircs );
                                                
                blob_t b;
                std::string name;
                                
                while( sqlite_query_next(q) )
                {
                        cmn::plotcirc_pt pc = std::make_shared<cmn::plotcirc_t>();
                        
                        sqlite_query_column( &name, q, 0 );
                        sqlite_query_column( &pc->img_size.x, q, 1 );
                        sqlite_query_column( &pc->img_size.y, q, 2 );
                        sqlite_query_column( &b, q, 3 );
                        sqlite_query_column( &pc->num_segments, q, 4 );
                        
                        //parse tail...
                        pc->name = cmn::name_create( name.c_str() );
                        
                        for( int y = 0; y < cmn::plotcirc_discr; ++y )
                        {
                                row_t const * row = (row_t const *)b.ptr;
                                pc->rows[y].resize(row->num_segments);
                                if( row->num_segments )
                                        memcpy( &pc->rows[y][0], row->segs, row->num_segments * sizeof(cmn::point2f_t) );
                                row = (row_t const *)((size_t)b.ptr + sizeof( row->num_segments ) + sizeof(cmn::point2f_t) * row->num_segments);
                        }
                }
                
        }
        catch( sqlite_exception const & )
        {
                //just don't import anything then
        }
}

void plotcirc_db_export_to_sqlite( cmn::plotcirc_db_pt const & pcd, std::string const & db_path )
{
        static char const insert_plotcirc[] = "INSERT INTO plotcircs(name,width,height,value,num_segments) VALUES(?,?,?,?,?)";
        
        sqlite_db_pt db = open_and_create_db( open_mode_read_write );
        sqlite_query_pt q = sqlite_query_create( db, insert_plotcirc );
        
        std::vector<uint8_t> buf;
        //buf.reserve( sizeof(int32_t)*cmn::plotcirc_discr + sizeof(cmn::point2f_t)* );
        std::string name;
        
        for( auto pci = pcd->plotcircs.cbegin(); pci != pcd->plotcircs.cend(); ++pci )
        {
                cmn::plotcirc_pt pc = *pci;
                buf.resize( sizeof(int32_t)*cmn::plotcirc_discr + sizeof(cmn::point2f_t)*pc->num_segments );
                int write_pos = 0;
                                                
                for( int y = 0; y < cmn::plotcirc_discr; ++y )
                {
                        *(int32_t*)&buf[write_pos] = pc->rows[y].size();
                        write_pos += sizeof(int32_t);
                        int32_t size = pc->rows[y].size() * sizeof(cmn::point2f_t);
                        if( !pc->rows[y].empty() )
                                memcpy( &buf[write_pos], &pc->rows[y][0], size );
                        write_pos += size;
                }
                
                blob_t b;
                b.ptr = &buf[0];
                b.size = buf.size();
                
                name = cmn::name_print( pc->name );
                sqlite_query_embed( q, name, 1 );
                sqlite_query_embed( q, pc->img_size.x, 2 );
                sqlite_query_embed( q, pc->img_size.y, 3 );
                sqlite_query_embed( q, b, 4 );
                sqlite_query_embed( q, pc->num_segments, 5);
                
                bool call_one_more_time = sqlite_query_next(q);            //or step..
                if( call_one_more_time )
                        cmn::log_and_throw("adapter::plotcirc_db_export_to_sqlite - something gone wrong: sqlite_query_next asked to go on for INSERT command");
                sqlite_query_reset( q );                
        }                
}
        
} //adapter