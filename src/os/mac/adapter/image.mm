#include <adapter/image.hpp>
#include <cmn/log.hpp>
#include <cmn/image.hpp>
#include <stdio.h>

#import <AppKit/NSImage.h>
#import <AppKit/NSImageRep.h>
#import <AppKit/NSGraphicsContext.h>
#import <Foundation/NSGeometry.h>

namespace adapter
{

cmn::image_pt image_create_from_png( char const * szImgPath )
{
@autoreleasepool
{
	cmn::image_root_pt img;
	int width,height,pitch;

	NSImageRep * srcImg = [NSImageRep imageRepWithContentsOfFile: [NSString stringWithUTF8String: szImgPath] ];
	
	width = (int)[srcImg pixelsWide];
	height = (int)[srcImg pixelsHigh];
	pitch = width << 2;

	img->memory = std::shared_ptr<uint8_t>( new uint8_t[pitch*height] );
	uint8_t * pixelBuf = img->memory.get();


	NSBitmapImageRep * imgRep = [[NSBitmapImageRep alloc]
				     initWithBitmapDataPlanes: &pixelBuf pixelsWide: width pixelsHigh: height
				     bitsPerSample: 32 samplesPerPixel:8 hasAlpha:true isPlanar:false colorSpaceName:NSDeviceRGBColorSpace bitmapFormat: NSAlphaNonpremultipliedBitmapFormat bytesPerRow:pitch bitsPerPixel:32];
	
	
	NSGraphicsContext * cx = [NSGraphicsContext graphicsContextWithBitmapImageRep: imgRep];
	
	struct RestorePrevCx
	{
		NSGraphicsContext * prevCx = [NSGraphicsContext currentContext];
		~RestorePrevCx(){ [NSGraphicsContext setCurrentContext: prevCx]; }
	};
	
	{
		RestorePrevCx prevCx;
		[NSGraphicsContext setCurrentContext: cx];
		
		//NSImage * nsImg = [[NSImage alloc] initWithCGImage: [imgRep CGImage] size: NSMakeSize(width, height) ];
		
		//[nsImg drawAtPoint: NSZeroPoint fromRect: NSZeroRect operation: NSCompositeCopy fraction: 1.0];
		[srcImg draw];
		[cx flushGraphics];
	}
	
	img->header.width = width;
	img->header.height = height;
	img->header.pitch = pitch;
	img->header.format = cmn::format_rgba;
	img->header.flags = 0;
	
	img->data.bytes = pixelBuf;
	//I believe we are done. Unsure regarding alpha channel though
	return img;

}

}
    
} //adapter