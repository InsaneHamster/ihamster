#pragma once
#include <cmn/fwd.hpp>
#include <string>

namespace adapter
{
        //to png file
        bool plotcirc_save_to_png( cmn::plotcirc_pt const & pc, char const * const szImgPath );
}