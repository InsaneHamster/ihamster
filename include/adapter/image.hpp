#pragma once
#include <cmn/fwd.hpp>

namespace adapter
{

//one possible implementation is in os/mac/adapter/image.mm
cmn::image_pt image_create_from_png( char const * szImgPath );

}