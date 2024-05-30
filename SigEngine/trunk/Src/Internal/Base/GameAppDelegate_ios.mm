#include "BasePch.hpp"
#if defined( platform_ios )

#import "GameAppDelegate_ios.hpp"

@implementation GameAppDelegate

@synthesize mUIWindow;

#pragma mark -
#pragma mark Application lifecycle

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{    
	// Override point for customization after application launch.

	// create window
    mUIWindow = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];  
    mUIWindow.backgroundColor = [UIColor greenColor];
	
	// create view and add to window
    mGameAppView = [[GameAppView alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
	[mUIWindow addSubview:mGameAppView];
	
	// make visible
    [mUIWindow makeKeyAndVisible];  

	// start animation (i.e. register for render/tick callback)
    [mGameAppView startAnimation];
	
	return YES;
}

- (void)applicationWillResignActive:(UIApplication *)application
{
	// Sent when the application is about to move from active to inactive state.
	// This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message)
	// or when the user quits the application and it begins the transition to the background state.
	// Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates.
	// Games should use this method to pause the game.
	
	[mGameAppView stopAnimation];
}


- (void)applicationDidBecomeActive:(UIApplication *)application
{
	// Restart any tasks that were paused (or not yet started) while the application was inactive.
	
	[mGameAppView startAnimation];
}


- (void)applicationWillTerminate:(UIApplication *)application
{
	// Called when the application is about to terminate.
	
	[mGameAppView stopAnimation];
}


#pragma mark -
#pragma mark Memory management

- (void)applicationDidReceiveMemoryWarning:(UIApplication *)application
{
    /*
     Free up as much memory as possible by purging cached data objects that can be recreated (or reloaded from disk) later.
     */
}


- (void)dealloc
{
    [mUIWindow release];
    [mGameAppView release];
    [super dealloc];
}


@end

#endif //defined( platform_ios )
