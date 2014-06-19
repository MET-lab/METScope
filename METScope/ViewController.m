//
//  ViewController.m
//  METScope
//
//  Created by Jeff Gregorio on 6/17/14.
//  Copyright (c) 2014 Jeff Gregorio. All rights reserved.
//

#import "ViewController.h"

@interface ViewController ()

@end

@implementation ViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    [self.view setBackgroundColor:[UIColor whiteColor]];
    
    /* Set up audio */
    audioController = [[AudioController alloc] init];
    
    /* Set up the METScopeView */
    [scopeView setPlotResolution:512];
    [scopeView setHardXLim:-0.001 max:kMaxPlotBufferTime];
    [scopeView setHardYLim:-1.1 max:1.1];
    [scopeView setVisibleXLim:0.0 max:[audioController getBufferLength]/kAudioSampleRate];
    [scopeView setPlotUnitsPerTick:0.005 vertical:0.5];
    [scopeView setXGridAutoScale:true];
    [scopeView setYGridAutoScale:true];
    [scopeView setXPinchZoomEnabled:false];
    [scopeView setYPinchZoomEnabled:false];
    [scopeView setDelegate:self];
    
    plotIdx = [scopeView addPlotWithColor:[UIColor blueColor] lineWidth:2.0];
    currentXBuffer = (float *)malloc([audioController getBufferLength] * sizeof(float));
    for (int i = 0; i < [audioController getBufferLength]; i++)
        currentXBuffer[i] = i / kAudioSampleRate;
    
    longScaleXBuffer = (float *)malloc([audioController getLongScaleBufferLength] * sizeof(float));
    longScaleYBuffer = (float *)calloc([audioController getLongScaleBufferLength], sizeof(float));
    for (int i = 0; i < [audioController getLongScaleBufferLength]; i++)
        longScaleXBuffer[i] = i / kAudioSampleRate;
    
    /* Set up tap recognizer to start recording */
    tapRecognizer = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(handleTap:)];
    [tapRecognizer setNumberOfTapsRequired:2];
    [scopeView addGestureRecognizer:tapRecognizer];
    hold = false;
    
    /* Update the scope views on a timer by querying AudioController's internal buffers */
    [NSTimer scheduledTimerWithTimeInterval:0.002 target:self selector:@selector(plotCurrentBuffer) userInfo:nil repeats:YES];
    
    pinchRecognizer = [[UIPinchGestureRecognizer alloc] initWithTarget:self action:@selector(handlePinch:)];
    [scopeView addGestureRecognizer:pinchRecognizer];
    
    panRecognizer = [[UIPanGestureRecognizer alloc] initWithTarget:self action:@selector(handlePan:)];
    [panRecognizer setMinimumNumberOfTouches:1];
    [panRecognizer setMinimumNumberOfTouches:1];
    [scopeView addGestureRecognizer:panRecognizer];
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}


- (void)plotCurrentBuffer {
    
    if (hold)
        return;
    
    /* Allocate current signal buffer */
    float *currentYBuffer = (float *)malloc([audioController getBufferLength] * sizeof(float));
    
    /* Get current buffer values from the audio controller */
    [audioController getCurrentBuffer:currentYBuffer];
    
    /* Update the plots */
    [scopeView setPlotDataAtIndex:plotIdx
                       withLength:[audioController getBufferLength]
                            xData:currentXBuffer
                            yData:currentYBuffer];
    
    free(currentYBuffer);
}

/* Plot visible portion of waveform based on updated plot limits */
- (void)plotVisible {
    
    /* Get the visible portion of the long-scale buffer */
    int startIdx = fmax(scopeView.visiblePlotMin.x, 0.0) * kAudioSampleRate;
    int endIdx = scopeView.visiblePlotMax.x * kAudioSampleRate;
    int visibleBufferLength = endIdx - startIdx;
    
    float *visibleXBuffer = (float *)malloc(visibleBufferLength * sizeof(float));
    float *visibleYBuffer = (float *)malloc(visibleBufferLength * sizeof(float));
    for (int i = startIdx, j = 0; i < endIdx; i++, j++) {
        visibleXBuffer[j] = i / kAudioSampleRate;
        visibleYBuffer[j] = longScaleYBuffer[i];
    }
    
    int audioFramesPerPlotFrame = round(((float)visibleBufferLength / (float)scopeView.plotResolution));
    
    if (audioFramesPerPlotFrame > 10) {
    
        [scopeView setFillMode:true atIndex:plotIdx];
        
        float *amplitudeXBuffer = (float *)malloc(scopeView.plotResolution * sizeof(float));
        [self linspace:scopeView.visiblePlotMin.x
                   max:scopeView.visiblePlotMax.x
           numElements:scopeView.plotResolution
                 array:amplitudeXBuffer
         ];
        
        float maxInWindow;
        float *maxAmpYBuffer = (float *)malloc(scopeView.plotResolution * sizeof(float));
        for (int i = 0; i < scopeView.plotResolution; i++) {
            
            maxInWindow = 0.0;
            for (int j = 0; j < audioFramesPerPlotFrame; j++) {
                if (visibleYBuffer[i*audioFramesPerPlotFrame+j] > maxInWindow)
                    maxInWindow = visibleYBuffer[i*audioFramesPerPlotFrame+j];
            }
            
            maxAmpYBuffer[i] = maxInWindow;
        }
        
        /* Update the plot */
        [scopeView setPlotDataAtIndex:plotIdx
                           withLength:scopeView.plotResolution
                                xData:amplitudeXBuffer
                                yData:maxAmpYBuffer
         ];
        
        free(amplitudeXBuffer);
        free(maxAmpYBuffer);
    }
    else {
        
        [scopeView setFillMode:false atIndex:plotIdx];
    
        /* Update the plot */
        [scopeView setPlotDataAtIndex:plotIdx
                           withLength:visibleBufferLength
                                xData:visibleXBuffer
                                yData:visibleYBuffer];
    }
    free(visibleYBuffer);
    free(visibleXBuffer);
}

/* Take a snapshot of the long-scale buffer and enable pinch-zoom */
- (void)handleTap:(UITapGestureRecognizer *)sender {
    
    if (hold) {
        
        hold = false;
        [scopeView setXPinchZoomEnabled:false];
        [scopeView setVisibleXLim:0.0 max:[audioController getBufferLength]/kAudioSampleRate];
        [scopeView setPlotUnitsPerTick:0.005 vertical:0.5];
        [scopeView setVisibleYLim:-1.1 max:1.1];
        [scopeView setFillMode:false atIndex:plotIdx];
    }
    else {
        
        hold = true;
        
        /* Get long-scale buffer at time of snapshot */
        [audioController getLongScaleBuffer:longScaleYBuffer];
        
        [scopeView setVisibleXLim:([audioController getLongScaleBufferLength] - [audioController getBufferLength]) / kAudioSampleRate
                              max:[audioController getLongScaleBufferLength]/kAudioSampleRate];
        [scopeView setPlotUnitsPerTick:0.005 vertical:0.5];
        
        [self plotVisible];
    }
    
    /* Flash animation */
    UIView *flashView = [[UIView alloc] initWithFrame:scopeView.frame];
    [flashView setBackgroundColor:[UIColor blackColor]];
    [flashView setAlpha:0.5f];
    [[self view] addSubview:flashView];
    [UIView animateWithDuration:0.5f
                     animations:^{
                         [flashView setAlpha:0.0f];
                     }
                     completion:^(BOOL finished) {
                         [flashView removeFromSuperview];
                     }
     ];
}

- (void)handlePinch:(UIPinchGestureRecognizer *)sender {
    
    if (sender.state == UIGestureRecognizerStateBegan) {
        previousPinchScale = sender.scale;
    }
    
    CGFloat scaleChange;
    scaleChange = sender.scale - previousPinchScale;
    scaleChange /= 2.0;
    
    [scopeView setVisibleXLim:(scopeView.visiblePlotMin.x + scaleChange*(scopeView.visiblePlotMin.x))
                          max:scopeView.visiblePlotMax.x];
    
    [scopeView setVisibleXLim:scopeView.visiblePlotMin.x
                          max:(scopeView.visiblePlotMax.x - scaleChange*scopeView.visiblePlotMax.x)];
    
    if (sender.state == UIGestureRecognizerStateEnded)
        [self plotVisible];
    
    previousPinchScale = sender.scale;
}

- (void)handlePan:(UIPanGestureRecognizer *)sender {
    
    CGPoint touchLoc = [sender locationInView:sender.view];
    
    if (sender.state == UIGestureRecognizerStateBegan) {
        previousPanLoc = touchLoc;
    }
    
    CGPoint locChange;
    locChange.x = previousPanLoc.x - touchLoc.x;
    locChange.y = previousPanLoc.y - touchLoc.y;
    
    locChange.x *= scopeView.unitsPerPixel.x;
    
    [scopeView setVisibleXLim:(scopeView.visiblePlotMin.x+locChange.x) max:(scopeView.visiblePlotMax.x+locChange.x)];
    [self plotVisible];
    
    previousPanLoc = touchLoc;
}

- (IBAction)updateGain:(UISlider *)sender {
    [audioController setGain:sender.value];
}

#pragma mark -
#pragma mark Utility
/* Generate a linearly-spaced set of indices for sampling an incoming waveform */
- (void)linspace:(float)minVal max:(float)maxVal numElements:(int)size array:(float*)array {
    
    float step = (maxVal-minVal)/(size-1);
    array[0] = minVal;
    int i;
    for (i = 1;i<size-1;i++) {
        array[i] = array[i-1]+step;
    }
    array[size-1] = maxVal;
}

@end




















