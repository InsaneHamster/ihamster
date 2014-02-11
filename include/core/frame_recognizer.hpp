#pragma once
#include <cmn/image.hpp>
#include <core/context.hpp>

namespace core
{

void frame_recognize( cmn::image_pt const & img, context_pt const & ctx );

}