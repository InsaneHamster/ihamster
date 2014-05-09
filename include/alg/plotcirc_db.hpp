#pragma once
#include <cmn/plotcirc.hpp>

namespace alg
{
  
struct plotcirc_find_t
{
        cmn::plotcirc_pt     plotcirc;
        cmn::plotcirc_cmp_t  cmp_res;        
};

typedef std::vector<plotcirc_find_t> plotcirc_find_vt;
        
struct plotcirc_db_t
{
        plotcirc_db_t();
        ~plotcirc_db_t();
        
        void add( cmn::plotcirc_pt const & pc );        
        void find( plotcirc_find_vt * res, int find_nth_best, cmn::plotcirc_pt const & test  );
        
        struct plotcirc_db_it;
        
protected:        
        typedef std::shared_ptr<plotcirc_db_it> plotcirc_db_ipt;        
        plotcirc_db_ipt m_impl;        
};
        
        
};