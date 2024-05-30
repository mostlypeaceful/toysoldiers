#include "BasePch.hpp"
#include "tApplication.hpp"
#include "Input/tTouch.hpp"
#import "GameAppView_ios.hpp"

namespace Sig
{
	namespace Input
	{
		// see tTouch_ios.cpp
		extern void fHandleIosTouch( UIView* view, NSSet* touches, b32 down );
	}
}

using namespace Sig;

@implementation GameAppView

@synthesize animating;
@dynamic animationFrameInterval;

+ (Class) layerClass 
{
	return [CAEAGLLayer class];
}

- (id) initWithFrame:(CGRect)frame
{
	self = [super initWithFrame:frame];
	if ( self != nil )
	{
		// Get the layer
        CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.layer;
		
        eaglLayer.opaque = TRUE;
        eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
                                        [NSNumber numberWithBool:FALSE], kEAGLDrawablePropertyRetainedBacking, kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, nil];
										
        animating = FALSE;
        mLayerResized = FALSE;
        mDisplayLinkSupported = FALSE;
        mAnimationFrameInterval = 1;
        mDisplayLink = nil;
        mAnimationTimer = nil;
		
        // A system version of 3.1 or greater is required to use CADisplayLink. The NSTimer
        // class is used as fallback when it isn't available.
        NSString *reqSysVer = @"3.1";
        NSString *currSysVer = [[UIDevice currentDevice] systemVersion];
        if ([currSysVer compare:reqSysVer options:NSNumericSearch] != NSOrderedAscending)
            mDisplayLinkSupported = TRUE;
	}
	return self;
}

- (void)drawView:(id)sender
{
	tApplication::fInstance( ).fExternalDelayedInit();
	if( mLayerResized )
	{
		Gfx::tDevice::fGetDefaultDevice()->fOnLayerResized(self.layer);
		mLayerResized = false;
	}
	const float dt = (1.0 / 60.0) * mAnimationFrameInterval;
	tApplication::fInstance( ).fExternalOnTick( dt );
}

- (void)layoutSubviews
{
	mLayerResized = true;
}

- (NSInteger)animationFrameInterval
{
    return mAnimationFrameInterval;
}

- (void)setAnimationFrameInterval:(NSInteger)frameInterval
{
    // Frame interval defines how many display frames must pass between each time the
    // display link fires. The display link will only fire 30 times a second when the
    // frame internal is two on a display that refreshes 60 times a second. The default
    // frame interval setting of one will fire 60 times a second when the display refreshes
    // at 60 times a second. A frame interval setting of less than one results in undefined
    // behavior.
    if (frameInterval >= 1)
    {
        mAnimationFrameInterval = frameInterval;
		
        if (animating)
        {
            [self stopAnimation];
            [self startAnimation];
        }
    }
}

- (void)startAnimation
{
    if (!animating)
    {
        if (mDisplayLinkSupported)
        {
            // CADisplayLink is API new to iPhone SDK 3.1. Compiling against earlier versions will result in a warning, but can be dismissed
            // if the system version runtime check for CADisplayLink exists in -initWithCoder:. The runtime check ensures this code will
            // not be called in system versions earlier than 3.1.
			
            mDisplayLink = [NSClassFromString(@"CADisplayLink") displayLinkWithTarget:self selector:@selector(drawView:)];
            [mDisplayLink setFrameInterval:mAnimationFrameInterval];
            [mDisplayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
        }
        else
            mAnimationTimer = [NSTimer scheduledTimerWithTimeInterval:(NSTimeInterval)((1.0 / 60.0) * mAnimationFrameInterval) target:self selector:@selector(drawView:) userInfo:nil repeats:TRUE];
		
        animating = TRUE;
    }
}

- (void)stopAnimation
{
    if (animating)
    {
        if (mDisplayLinkSupported)
        {
            [mDisplayLink invalidate];
            mDisplayLink = nil;
        }
        else
        {
            [mAnimationTimer invalidate];
            mAnimationTimer = nil;
        }
		
        animating = FALSE;
    }
}

- (void) dealloc
{
	//gGameApp->fExternalShutdown( );
	//delete gGameApp;
	// dealt with via atexit handler.  Is this the right thing to do ?
    [super dealloc];
}

- (void) touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
	Input::fHandleIosTouch( self, [event allTouches], true );
}

- (void) touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
	Input::fHandleIosTouch( self, [event allTouches], true );
}

- (void) touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
	Input::fHandleIosTouch( self, [event allTouches], false );
}

- (void) touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event
{
	Input::fHandleIosTouch( self, [event allTouches], false );
}

@end

