#include <core/frame_recognizer.hpp>
#include <alg/watershed.hpp>
#include <cmn/plotcirc.hpp>

#include <adapter/image.hpp>
#include <adapter/plotcirc.hpp>

#include <stdio.h>
#include <string>

int main()
{
	char const * path = "/Users/hein/Desktop/a.jpg";
	cmn::image_pt img = adapter::image_create_from_png( path );
	
	if (!img) return 2;
	
	bool res = adapter::image_save_to_png( img, "/Users/hein/Desktop/b.png" );
		
	return res?0:1;
}