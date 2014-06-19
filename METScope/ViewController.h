//
//  ViewController.h
//  METScope
//
//  Created by Jeff Gregorio on 6/17/14.
//  Copyright (c) 2014 Jeff Gregorio. All rights reserved.
//

#import <UIKit/UIKit.h>

#import "METScopeView.h"
#import "AudioController.h"
#import "NVLowpassFilter.h"

//#define scaleRatio 2

@interface ViewController : UIViewController <METScopeViewDelegate> {
    
    AudioController *audioController;
    
    IBOutlet METScopeView *scopeView;
    IBOutlet UISlider *inputGainSlider;
    
    int plotIdx;
    float *currentXBuffer;
    
    float *longScaleXBuffer;
    float *longScaleYBuffer;
    
    UITapGestureRecognizer *tapRecognizer;
    bool hold;
    
    UIPinchGestureRecognizer *pinchRecognizer;
    CGFloat previousPinchScale;
    UIPanGestureRecognizer *panRecognizer;
    CGPoint previousPanLoc;
}

@end
