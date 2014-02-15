#pragma once
#include <alg/pattern_sub.hpp>

#include <vector>

namespace alg
{

struct pattern_t
{
        //we need to prioritize them somehow
        std::vector<pattern_sub_pt>     subs;           //these really hold details
};

}
