#if defined( platform_ios )
#ifndef __GameAppDelegate_ios__
#define __GameAppDelegate_ios__

#import <UIKit/UIKit.h>
#import "GameAppView_ios.hpp"

@interface GameAppDelegate : NSObject <UIApplicationDelegate> {
    UIWindow *mUIWindow;
    GameAppView *mGameAppView;
}

@property (nonatomic, retain) IBOutlet UIWindow *mUIWindow;

@end

#endif //ndef __GameAppDelegate_ios__
#endif // defined( platform_ios )
