#include <adapter/image.hpp>
#include <cmn/log.hpp>
#include <cmn/image.hpp>
#include <stdio.h>

#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>

namespace adapter
{
	
#if 1
	cmn::image_pt image_create_from_png( char const * szImgPath )
	{
		cmn::image_root_pt img;
		
		// TODO: invent something!
		
		return img;
	}
#else
namespace
	{
	CGContextRef MyCreateBitmapContext (int pixelsWide,
										
										int pixelsHigh)
	
	{
		
		CGContextRef    context = NULL;
		CGColorSpaceRef colorSpace;
		void *          bitmapData;
		int             bitmapByteCount;
		int             bitmapBytesPerRow;
		
		
		bitmapBytesPerRow   = (pixelsWide * 4);// 1
		bitmapByteCount     = (bitmapBytesPerRow * pixelsHigh);
		
		
		colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);// 2
		bitmapData = malloc( bitmapByteCount );// 3
		
		if (bitmapData == NULL)
		{
			fprintf (stderr, "Memory not allocated!");
			return NULL;
			
		}
		
		context = CGBitmapContextCreate (bitmapData,// 4
										 pixelsWide,
										 pixelsHigh,
										 8,      // bits per component
										 bitmapBytesPerRow,
										 colorSpace,
										 kCGImageAlphaLast);
		
		if (context== NULL)
		{
			free (bitmapData);// 5
			fprintf (stderr, "Context not created!");
			return NULL;
			
		}
		
		CGColorSpaceRelease( colorSpace );// 6
		
		return context;// 7
	}
	
	}
cmn::image_pt image_create_from_png( char const * szImgPath )
{
@autoreleasepool
{
	cmn::image_root_pt img = std::make_shared<cmn::image_root_t>();
	int width,height,pitch;

	NSBitmapImageRep * srcImg = [NSBitmapImageRep imageRepWithContentsOfFile: [NSString stringWithUTF8String: szImgPath] ];
	
	width = (int)[srcImg pixelsWide];
	height = (int)[srcImg pixelsHigh];
	pitch = width << 2;
	
	CGContextRef cgcr = MyCreateBitmapContext(width, height);
	// here is the reason why it does not work https://developer.apple.com/library/mac/documentation/graphicsimaging/conceptual/drawingwithquartz2d/dq_context/dq_context.html#//apple_ref/doc/uid/TP30001066-CH203-BCIBHHBB
	// the tabel says that Quartz grahpic context does not suppoer alpha at all. Sad but true.
	// one more prof-link https://developer.apple.com/library/mac/qa/qa1037/_index.html
	// and disscution http://www.cocoabuilder.com/archive/cocoa/184516-obtaining-unpremultiplied-bitmap-from-png-file.html
	// and ultimate answer on stackoverflow http://stackoverflow.com/questions/6396139/how-to-get-the-real-rgba-or-argb-color-values-without-premultiplied-alpha

	img->memory = std::shared_ptr<uint8_t>( new uint8_t[pitch*height] );
	uint8_t * pixelBuf = img->memory.get();


	NSBitmapImageRep * imgRep = [[NSBitmapImageRep alloc]
								 initWithBitmapDataPlanes: &pixelBuf
								 pixelsWide: width
								 pixelsHigh: height
								 bitsPerSample: 8
								 samplesPerPixel: 4
								 hasAlpha: YES
								 isPlanar: NO
								 colorSpaceName:NSDeviceRGBColorSpace
								 bitmapFormat: 0//NSAlphaNonpremultipliedBitmapFormat -- not supported for Quartz context any more http://lists.apple.com/archives/quartz-dev/2007/Nov/msg00049.html
								 bytesPerRow: pitch
								 bitsPerPixel: 32];
	
	
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
	
	img->bytes = pixelBuf;
	//I believe we are done. Unsure regarding alpha channel though
	return img;

}
}
#endif
	
bool image_save_to_png( cmn::image_pt const & img, char const * szImgPath )
{
@autoreleasepool
{
	bool ok = false;

	uint8_t * pixelBuf = img->bytes;
	NSBitmapImageRep * imgRep = [[NSBitmapImageRep alloc]
								 initWithBitmapDataPlanes: &pixelBuf
								 pixelsWide: img->header.width
								 pixelsHigh: img->header.height
								 bitsPerSample: 8
								 samplesPerPixel: 4
								 hasAlpha: YES
								 isPlanar: NO
								 colorSpaceName:NSDeviceRGBColorSpace
								 bitmapFormat: NSAlphaNonpremultipliedBitmapFormat
								 bytesPerRow: img->header.pitch
								 bitsPerPixel: 32];
	
	if (imgRep)
	{
		NSBitmapImageFileType const imgType = NSPNGFileType; // choose one that sutes you better ;)
		NSMutableDictionary * imgProps = [[NSMutableDictionary alloc] init];
		switch (imgType)
		{
			case NSPNGFileType:
			{
				[ imgProps setObject: [ NSNumber numberWithBool: NO ] forKey:NSImageInterlaced ];
				[ imgProps setObject: [ NSNumber numberWithFloat: 1.0 ] forKey:NSImageGamma ];
				break;
			}
			case NSJPEGFileType:
			{
				[ imgProps setObject: [ NSNumber numberWithFloat: 1.0 ] forKey:NSImageCompressionFactor ];
				[ imgProps setObject: [ NSColor whiteColor ] forKey:NSImageFallbackBackgroundColor ];
				[ imgProps setObject: [ NSNumber numberWithBool: NO ] forKey:NSImageProgressive ];
				break;
			}
			case NSBMPFileType:
			{
				break;
			}
			default:
			{
			}
		};
		
		NSData * imgData = [ imgRep representationUsingType: imgType properties: imgProps];
		
		if (imgData)
		{
			BOOL success = [[ NSFileManager defaultManager ] createFileAtPath: [NSString stringWithUTF8String: szImgPath]  contents: imgData attributes: nil];
			ok = success;
		}
	}
	
	return ok;
}
}
    
} //adapter