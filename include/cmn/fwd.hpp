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
        
        struct plotcirc_t;
        typedef std::shared_ptr<plotcirc_t> plotcirc_pt;

        struct plotcirc_db_t;
        typedef std::shared_ptr<plotcirc_db_t> plotcirc_db_pt;
        struct plotcirc_cmp_t;
}