#pragma once
#include <alg/spot.hpp>

//algorith which find spots (or objects of same color but different illumination)
namespace alg
{

	//img_lab - f32 CIE L*a*b* format
	void spot_detector( std::vector< spot_t > * objects, cmn::image_pt * colored, cmn::image_pt const & img_lab );
	
}