//
//  Squares3DAppDelegate.m
//  Squares3D
//
//  Created by Martins Mozeiko on 8/14/10.
//  Copyright __MyCompanyName__ 2010. All rights reserved.
//

#import "Squares3DAppDelegate.h"
#import "EAGLView.h"

@implementation Squares3DAppDelegate

@synthesize window;
@synthesize glView;

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions   
{
	application.idleTimerDisabled = YES;
    
    window = [[UIWindow alloc] initWithFrame:[UIScreen mainScreen].bounds];
    glView = [[EAGLView alloc] initWithFrame:window.bounds];
    [window addSubview:glView];
    [window makeKeyAndVisible];

    [glView startAnimation];
    return YES;
}

- (void)applicationWillResignActive:(UIApplication *)application
{
    [glView stopAnimation];
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
    [glView startAnimation];
}

- (void)applicationWillTerminate:(UIApplication *)application
{
    [glView stopAnimation];
}

- (void)dealloc
{
    [window release];
    [glView release];

    [super dealloc];
}

@end
