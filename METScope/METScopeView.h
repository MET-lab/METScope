//
//  METScopeView.h
//  METScopeViewTest
//
//  Created by Jeff Gregorio on 5/7/14.
//  Copyright (c) 2014 Jeff Gregorio. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <Accelerate/Accelerate.h>
#import <pthread.h>

/* Idea: 
 
    Have METScopeView store full-resolution waveforms so it can dynamically re-sample when pinch zooming. 
 
    Also implement a method to re-set plot data within a certain range. This way, in the longer time-scale scope view we can set a small range (i.e. one audio buffer) at a time and have it sample and scale only that part, rather than sampling and scaling the full time scale on every update. Can also save the indices of the scaled plot data that was modified so we don't draw the full time scale waveform on each update, only the modified part.
 */

#pragma mark Defaults

#define METScopeView_Default_PlotResolution 512
#define METScopeview_Default_MaxRefreshRate 0.02
/* Time-domain mode defaults */
#define METScopeView_Default_XMin_TD (-0.0001)
#define METScopeView_Default_XMax_TD 0.023      // For length 1024 buffer at 44.1kHz
#define METScopeView_Default_YMin_TD (-1.25)
#define METScopeView_Default_YMax_TD 1.25
#define METScopeView_Default_XTick_TD 0.005
#define METScopeView_Default_YTick_TD 0.5
#define METScopeView_Default_xLabelFormatString_TD @"%5.3f"
#define METScopeView_Default_yLabelFormatString_TD @"%3.2f"
/* Frequency-domain mode defaults */
#define METScopeView_Default_SamplingRate 44100 // For x-axis scaling
#define METScopeView_Default_XMin_FD (-20)
#define METScopeView_Default_XMax_FD 20000.0    // For sampling rate 44.1kHz
#define METScopeView_Default_YMin_FD (-0.04)
#define METScopeView_Default_YMax_FD 1.0
#define METScopeView_Default_XTick_FD 4000
#define METScopeView_Default_YTick_FD 0.25
#define METScopeView_Default_xLabelFormatString_FD @"%5.0f"
#define METScopeView_Default_yLabelFormatString_FD @"%3.2f"
/* Auto grid scaling defaults */
#define METScopeView_AutoGrid_MaxXTicksInFrame 6.0
#define METScopeView_AutoGrid_MinXTicksInFrame 4.0
#define METScopeView_AutoGrid_MaxYTicksInFrame 5.0
#define METScopeView_AutoGrid_MinYTicksInFrame 3.0

@protocol METScopeViewDelegate <NSObject>
@required
- (void)finishedPinchZoom;
@end

/* Forward declaration of subview classes */
@class METScopeAxisView;
@class METScopeGridView;
@class METScopeLabelView;
@class METScopePlotDataView;

/* Whether we're sampling a time-domain waveform or doing an FFT */
typedef enum DisplayMode {
    kMETScopeViewTimeDomainMode,
    kMETScopeViewFrequencyDomainMode,
} DisplayMode;

#pragma mark -
#pragma mark METScopeView
@interface METScopeView : UIView <UIGestureRecognizerDelegate> {
    
    NSMutableArray *plotDataSubviews;   // Subview array of plot waveforms
    METScopeAxisView *axesSubview;      // Subview that draws axes
    METScopeGridView *gridSubview;      // Subveiw that draws grid
    METScopeLabelView *labelsSubview;   // Subview that draws labels
    
    /* Pinch zoom */
    UIPinchGestureRecognizer *pinchRecognizer;
    CGPoint previousPinchTouches[2];
    int previousNumPinchTouches;
    bool pinchZoomEnabled;
    
    /* Spectrum mode FFT parameters */
    int fftSize;                // Length of FFT, 2*nBins
    int windowSize;             // Length of Hann window
    float *inRealBuffer;        // Input buffer
    float *outRealBuffer;       // Output buffer
    float *window;              // Hann window
    float scale;                // Normalization constant
    FFTSetup fftSetup;          // vDSP FFT struct
    COMPLEX_SPLIT splitBuffer;  // Buffer holding real and complex parts
}

#pragma mark -
#pragma mark Properties
@property id <METScopeViewDelegate> delegate;       /* Delegate called after pinch zoom */

@property (readonly) int plotResolution;            /* Default number of values sampled
                                                       from incoming waveforms */
@property (readonly) DisplayMode displayMode;       // Time or frequency domain
@property (readonly) CGPoint visiblePlotMin;        // Visible bounds in plot units
@property (readonly) CGPoint visiblePlotMax;
@property (readonly) CGPoint minPlotMin;            // Hard limits constraining pinch zoom
@property (readonly) CGPoint maxPlotMax;
@property (readonly) CGPoint tickUnits;             // Grid/tick spacing in plot units
@property (readonly) CGPoint tickPixels;            // Grid/tick spacing in pixels
@property (readonly) CGPoint originPixel;           // Plot origin location in pixels
@property (readonly) CGPoint unitsPerPixel;         // Plot unit <-> pixel conversion factor
@property (readonly) bool axesOn;                   // Drawing axes subview
@property (readonly) bool gridOn;                   // Drawing grid subview
@property (readonly) bool labelsOn;                 // Drawing labels subview

@property int samplingRate;                     /* Set for proper x-axis scaling in
                                                   frequency domain mode (default 44.1kHz) */

@property NSString *xLabelFormatString;     // Format specifiers for numerical labels
@property NSString *yLabelFormatString;

@property bool xLabelsOn;               // Labels subview drawing x/y labels
@property bool yLabelsOn;
@property bool xGridAutoScale;          // Keep a specified number of grid squares
@property bool yGridAutoScale;
@property bool xPinchZoomEnabled;       // Enable/disable built-in pinch zoom
@property bool yPinchZoomEnabled;

@property CGPoint minimumPlotRange;     // Range constraints on pinch zooming (in plot units)
@property CGPoint maximumPlotRange;

#pragma mark -
#pragma mark Interface Methods
/* Set the number of points sampled from incoming waveforms */
- (void)setPlotResolution:(int)res;

/* Set the display mode to time/frequency domain and automatically rescale to default limits */
- (void)setDisplayMode:(DisplayMode)mode;

/* Initialize a vDSP FFT object */
- (void)setUpFFTWithSize:(int)size;

/* Hard axislimits constraining pinch zoom; update */
- (void)setHardXLim:(float)xMin max:(float)xMax;
- (void)setHardYLim:(float)yMin max:(float)yMax;

/* Set the visible ranges of the axes in plot units; update */
- (void)setVisibleXLim:(float)xMin max:(float)xMax;
- (void)setVisibleYLim:(float)yMin max:(float)yMax;

/* Set ticks and grid scale by specifying the input magnitude per tick/grid block; update */
- (void)setPlotUnitsPerXTick:(float)xTick;
- (void)setPlotUnitsPerYTick:(float)yTick;
- (void)setPlotUnitsPerTick:(float)xTick vertical:(float)yTick;

/* Add/remove subviews for axes, labels, and grid */
- (void)setAxesOn:(bool)pAxesOn;
- (void)setGridOn:(bool)pGridOn;
- (void)setLabelsOn:(bool)pLabelsOn;

/* Set colors of axes, grid, and labels */
- (void)setAxisColor:(UIColor *)color;
- (void)setGridColor:(UIColor *)color;
- (void)setLabelColor:(UIColor *)color;
- (void)setLabelSize:(int)size;

/* Allocate a subview for new plot data with specified resolution/color/linewidth, return the index */
- (int)addPlotWithColor:(UIColor *)color lineWidth:(float)width;
- (int)addPlotWithResolution:(int)res color:(UIColor *)color lineWidth:(float)width;

/* Set the plot data for a subview at a specified index */
- (void)setPlotDataAtIndex:(int)idx withLength:(int)len xData:(float *)xx yData:(float *)yy;

/* Set/update plot resolution/color/linewidth for a waveform at a specified index */
- (void)setPlotColor:(UIColor *)color atIndex:(int)idx;
- (void)setLineWidth:(float)width atIndex:(int)idx;
- (void)setPlotResolution:(int)res atIndex:(int)idx;
- (void)setVisiblity:(bool)visible atIndex:(int)idx;
- (void)setFillMode:(bool)doFill atIndex:(int)idx;

/* Utility methods: convert pixel values to plot scales and vice-versa */
- (CGPoint)plotScaleToPixel:(float)pX y:(float)pY;
- (CGPoint)plotScaleToPixel:(CGPoint)plotScale;
- (CGPoint)pixelToPlotScale:(CGPoint)pixel;
- (CGPoint)pixelToPlotScale:(CGPoint)pixel withOffset:(CGPoint)pixelOffset;

@end
