#pragma once
#include <memory>

namespace cmn
{
        struct image_t;
        typedef std::shared_ptr<image_t> image_pt;

        struct image_root_t;
        typedef std::shared_ptr<image_root_t> image_root_pt;

        struct image_sub_t;
        typedef std::shared_ptr<image_sub_t> image_sub_pt;
        
        struct name_t;
        
        struct plotcirc_pat_t;
        typedef std::shared_ptr<plotcirc_pat_t> plotcirc_pat_pt;
        struct plotcirc_test_t;
        typedef std::shared_ptr<plotcirc_test_t> plotcirc_test_pt;
        struct plotcirc_cmp_t;
}