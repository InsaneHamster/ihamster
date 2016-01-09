#include <adapter/image.hpp>
#include <cmn/log.hpp>
#include <cmn/image.hpp>
#include <stdio.h>

#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>

namespace adapter
{

cmn::image_pt image_create_from_png( char const * szImgPath )
{
@autoreleasepool
{
	cmn::image_root_pt img = std::make_shared<cmn::image_root_t>();
	int width,height,pitch,size_dst;
	CGImageRef cgimg;
	CGDataProviderRef source;
	
	source = CGDataProviderCreateWithFilename( szImgPath );
	if( !source ) goto error_00;
	cgimg = CGImageCreateWithPNGDataProvider( source, 0, false, kCGRenderingIntentDefault );
	if( !cgimg )
	{
		cmn::log("adapter::imagec_create_from_png (mac os) - Can't create png image from file: %s (bad png ?)", szImgPath);
		goto error_01;
	}
	
	width = (int)CGImageGetWidth( cgimg );
	height = (int)CGImageGetHeight( cgimg );
	pitch = (int)CGImageGetBytesPerRow( cgimg );  //width << 2;		//dst pitch that is!
	size_dst = pitch * height;
	
	img->memory = std::shared_ptr<uint8_t>( new uint8_t[size_dst], [](uint8_t* mem){delete[] mem;} );
	
	{
		uint8_t * pixelBuf = img->memory.get();

		//there is a large chance that the image is already in a necessary format
		uint32_t info = CGImageGetBitmapInfo(cgimg);
		int32_t bits_per_channel = (int32_t)CGImageGetBitsPerComponent(cgimg);
		int32_t bits_per_pixel = (int32_t)CGImageGetBitsPerPixel(cgimg);
		
		bool just_copy = (info & kCGImageAlphaPremultipliedLast);
		just_copy = just_copy && (info & ~kCGBitmapFloatComponents);
		//just_copy = just_copy && (info & kCGBitmapByteOrder32Little);
		just_copy = just_copy && bits_per_channel == 8 && bits_per_pixel == 32;
		
		//TODO: [5] add handling of usual RGB format without alpha at all
		if(just_copy)
		{
			CFDataRef data = CGDataProviderCopyData(CGImageGetDataProvider(cgimg));
			int size_src = (int)CFDataGetLength(data);
			
			if( size_dst != size_src )
			{
				CFRelease(data);
				cmn::log("adapter::image_create_from_png (mac os) expected size: %d, real size: %d", size_dst, size_src );
				goto error_02;
			}

			CFRange r = CFRangeMake( 0, size_src );
			CFDataGetBytes( data, r, img->memory.get() );
			CFRelease(data);
		}
		else
		{
			//TODO: [1] write me
			cmn::log_and_throw("Implement me file: %s, line: %d", __FILE__, __LINE__);
			
			//YS and VM notices - you can't use not premultiplied alpha with context
			//VM:
			// here is the reason why it does not work https://developer.apple.com/library/mac/documentation/graphicsimaging/conceptual/drawingwithquartz2d/dq_context/dq_context.html#//apple_ref/doc/uid/TP30001066-CH203-BCIBHHBB
			// the tabel says that Quartz grahpic context does not suppoer alpha at all. Sad but true.
			// one more prof-link https://developer.apple.com/library/mac/qa/qa1037/_index.html
			// and disscution http://www.cocoabuilder.com/archive/cocoa/184516-obtaining-unpremultiplied-bitmap-from-png-file.html
			// and ultimate answer on stackoverflow http://stackoverflow.com/questions/6396139/how-to-get-the-real-rgba-or-argb-color-values-without-premultiplied-alpha
			//YS: but we can use floats! and one more conversion.
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
				//[srcImg draw];
				[cx flushGraphics];
			}
		}
	}
	
	img->header.width = width;
	img->header.height = height;
	img->header.pitch = pitch;
	img->header.format = cmn::format_rgba;
	img->header.flags = 0;
	
	img->bytes = img->memory.get();
	//I believe we are done. Unsure regarding alpha channel though
	
	CFRelease(cgimg);
	CFRelease(source);
	return img;
	
error_02:
	CFRelease(cgimg);
error_01:
	CFRelease(source);
error_00:
	return cmn::image_root_pt();
}
}

	
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