#pragma once
#include <cmn/fwd.hpp>
#include <memory>

namespace alg
{

struct pattern_sub_t
{
      cmn::image_pt     image;             //g16 accumulative image. that is if object presents in the point the point will have +1, with max == number
      int               number;            //number of images accumulated in image. -1 normalized and finalized.
};


typedef std::shared_ptr<pattern_sub_t> pattern_sub_pt;

}