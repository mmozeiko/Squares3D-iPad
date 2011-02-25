//
//  ES1Renderer.h
//  Squares3D
//
//  Created by Martins Mozeiko on 8/14/10.
//  Copyright __MyCompanyName__ 2010. All rights reserved.
//

#import "ESRenderer.h"

#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>

@interface ES1Renderer : NSObject <ESRenderer>
{
@private
    EAGLContext *context;

    // The pixel dimensions of the CAEAGLLayer
    GLint backingWidth;
    GLint backingHeight;
    int firstTime;

    // The OpenGL ES names for the framebuffer and renderbuffer used to render to this view
    GLuint defaultFramebuffer, colorRenderbuffer, depthRenderbuffer;
}

- (void)render;
- (BOOL)resizeFromLayer:(CAEAGLLayer *)layer;

@end
