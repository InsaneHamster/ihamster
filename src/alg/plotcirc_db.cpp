#include <alg/plotcirc_db.hpp>
#include <map>
#include <float.h>
#include <math.h>


namespace alg
{

struct plotcirc_weight_t
{
        float           weight;
        int             found_dots;
};
        

        
void plotcirc_db_t_add( cmn::plotcirc_db_pt const & pcd, cmn::plotcirc_pt const & pc )
{
        for( int i = 0; i < cmn::plotcirc_discr; ++i )
        {
             std::vector< cmn::point2f_t > const & vv = pc->rows[i];
             cmn::point2f_t const * v = &vv[0];
             cmn::point2f_t const * v_end = &vv[0] + vv.size();
             
             while( v != v_end )
             {
                pcd->begins[i].insert( std::make_pair( v->x, pc ) );
                pcd->ends[i].insert( std::make_pair( v->y, pc ) );
                ++v;
             }
        }                
}

static void record_weight( std::map< cmn::plotcirc_pt, plotcirc_weight_t > & w, int n, float t, cmn::plotcirc_db_map_t::const_iterator f, cmn::plotcirc_db_map_t const & map )
{        
        cmn::plotcirc_db_map_t::const_iterator f_down = f;
        cmn::plotcirc_db_map_t::const_iterator f_prev = f;
        float dist_min;
                        
        while( !(f_down == map.begin() && f == map.end() && w.size() < n ) )
        {
                if( f != map.end() )
                {
                        dist_min = f->first - t;                
                        ++f;
                }
                else        
                        dist_min = FLT_MAX; //t - (--map.end())->first;
                
                if( f_down != map.begin() )
                {
                        float dist_cur = t - f_down->first;
                        if( dist_cur < dist_min )
                        {
                                dist_min = dist_cur;
                                auto wf = w.find( f_down->second );
                                if( wf != w.end() )
                                {
                                        wf->second.weight += dist_cur;          //optionally add * t or even *t*t
                                        ++(wf->second.found_dots);          //optionally add * t or even *t*t
                                }
                                else
                                        w.emplace( f_down->second, plotcirc_weight_t{dist_cur,1} );
                        }
                        else
                        {
                                auto wf = w.find( f_prev->second );
                                if( wf != w.end() )
                                {
                                        wf->second.weight += dist_min;
                                        ++(wf->second.found_dots);
                                }
                                else
                                        w.emplace( f_prev->second, plotcirc_weight_t{dist_min,1} );
                                f_prev = f;
                        }
                        
                        --f_down;
                }                                                                
        }                
};

void plotcirc_db_find( cmn::plotcirc_find_vt * res, int find_nth_best, cmn::plotcirc_db_pt const & pcd, cmn::plotcirc_pt const & test )
{                
        //to gather in a fair manner nth best of them we have to collect all, and select at last stage
        //but it's too heavy operation so we will select actually 10% of them ... or 1 max
        std::map< cmn::plotcirc_pt, plotcirc_weight_t > weights;  
        int to_collect = (int)ceil(pcd->begins[0].size() * 0.1f);        
                                
        for( size_t i = 0; i < cmn::plotcirc_discr; ++i )
        {
                cmn::plotcirc_db_map_t const & begins = pcd->begins[i];
                cmn::plotcirc_db_map_t const & ends = pcd->ends[i];
                std::vector< cmn::point2f_t > const & row_src = test->rows[i];
                
                for( size_t j = 0; j != row_src.size(); ++j )
                {
                        cmn::point2f_t const & point_src = row_src[j];
                        
                        cmn::plotcirc_db_map_t::const_iterator fb = pcd->begins[i].lower_bound( point_src.x );
                        record_weight( weights, to_collect, point_src.x, fb, begins );
                        
                        cmn::plotcirc_db_map_t::const_iterator fe = pcd->ends[i].lower_bound( point_src.x );
                        record_weight( weights, to_collect, point_src.x, fe, begins );

                }                
        }
        
        //now select the best one
        std::map< float, cmn::plotcirc_pt >     best;
        for( auto i = weights.begin(); i != weights.end(); ++i )
        {
                cmn::plotcirc_pt const & pc = i->first;
                plotcirc_weight_t const & pw = i->second;
                int diff = pc->num_segments - pw.found_dots / 2;
                //how large penalty will we assign for each missing dot ?
                float penalty = diff * 0.1f;    //1.f - is hypotenuse of the image
                float err = (penalty + pw.weight)/cmn::plotcirc_discr;
                best[err] = pc;
        }
        
        float hypotenuse_test = sqrt( test->img_size.sq() );
        
        auto j = best.begin();
        for( int i = 0; i < find_nth_best && j != best.end(); ++i, ++j )
        {                
                float hypotenuse_pat = sqrt( j->second->img_size.sq() );

                cmn::plotcirc_find_t f;
                f.plotcirc = j->second;
                f.cmp_res.diff = j->first;                                          
                f.cmp_res.scale = hypotenuse_test / hypotenuse_pat;
                f.cmp_res.angle = 0;
                res->push_back( f );
        }
}

        
} //alg