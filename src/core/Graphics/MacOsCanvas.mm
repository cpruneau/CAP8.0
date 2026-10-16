//
// MacOsCanvas.mm -- macOS backend for CAP::GRAPHICS::Canvas (AppKit + CoreGraphics + CoreText).
// Compiled as Objective-C++ with ARC (see CMakeLists.txt).
//
#import <Cocoa/Cocoa.h>
#import <CoreText/CoreText.h>
#include <cmath>
#include <cstdio>
#include <exception>
#include <vector>
#include "MacOsCanvas.hpp"
#include "Canvas.hpp"

using CAP::GRAPHICS::MacOsCanvas;

//------------------------------------------------------------------------------
// View: forwards drawRect: to the C++ painter
//------------------------------------------------------------------------------
@interface CAPCanvasView : NSView
@property (nonatomic, assign) MacOsCanvas * painter;
@end

@implementation CAPCanvasView
- (BOOL)isOpaque { return YES; }
- (void)drawRect:(NSRect)dirtyRect
{
  (void)dirtyRect;
  if (!self.painter) return;
  CGContextRef context = [[NSGraphicsContext currentContext] CGContext];
  NSRect bounds = self.bounds;
  self.painter->paint((void*)context, bounds.size.width, bounds.size.height);
}
@end

//------------------------------------------------------------------------------
// Window delegate: closing the window ends show()
//------------------------------------------------------------------------------
@interface CAPWindowDelegate : NSObject <NSWindowDelegate>
@end

@implementation CAPWindowDelegate
- (void)windowWillClose:(NSNotification *)notification
{
  (void)notification;
  [NSApp stop:nil];
  // stop: only takes effect after the next event; post one so show() returns right away
  NSEvent * wake = [NSEvent otherEventWithType:NSEventTypeApplicationDefined
                                      location:NSZeroPoint
                                 modifierFlags:0
                                     timestamp:0
                                  windowNumber:0
                                       context:nil
                                       subtype:0
                                         data1:0
                                         data2:0];
  [NSApp postEvent:wake atStart:YES];
}
@end

//------------------------------------------------------------------------------
// Helpers
//------------------------------------------------------------------------------
namespace
{
  const double kPi = 3.14159265358979323846;

  //! PostScript name for a ROOT font code (family = code/10).
  const char * fontName(int rootFont)
  {
    static const char * names[] =
      {
      "Helvetica",              //  0: not a ROOT family, default
      "Times-Italic",           //  1
      "Times-Bold",             //  2
      "Times-BoldItalic",       //  3
      "Helvetica",              //  4
      "Helvetica-Oblique",      //  5
      "Helvetica-Bold",         //  6
      "Helvetica-BoldOblique",  //  7
      "Courier",                //  8
      "Courier-Oblique",        //  9
      "Courier-Bold",           // 10
      "Courier-BoldOblique",    // 11
      "Symbol",                 // 12
      "Times-Roman",            // 13
      "ZapfDingbatsITC",        // 14
      "Symbol"                  // 15
      };
    int family = rootFont/10;
    if (family < 0 || family > 15) family = 0;
    return names[family];
  }

  //! CoreText line for utf8 text; caller releases it. The text color comes from the context fill color.
  CTLineRef makeLine(const char * utf8, int rootFont, CGFloat size)
  {
    CFStringRef string = CFStringCreateWithCString(kCFAllocatorDefault, utf8, kCFStringEncodingUTF8);
    if (!string) return NULL;
    CFStringRef name = CFStringCreateWithCString(kCFAllocatorDefault, fontName(rootFont), kCFStringEncodingASCII);
    CTFontRef   font = CTFontCreateWithName(name, size, NULL);
    CFTypeRef keys[]   = { kCTFontAttributeName, kCTForegroundColorFromContextAttributeName };
    CFTypeRef values[] = { font, kCFBooleanTrue };
    CFDictionaryRef attributes = CFDictionaryCreate(kCFAllocatorDefault, keys, values, 2,
                                                    &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFAttributedStringRef text = CFAttributedStringCreate(kCFAllocatorDefault, string, attributes);
    CTLineRef line = CTLineCreateWithAttributedString(text);
    CFRelease(text);
    CFRelease(attributes);
    CFRelease(font);
    CFRelease(name);
    CFRelease(string);
    return line;
  }

  void setStroke(CGContextRef context, const CAP::GRAPHICS::LineAttributes & line)
  {
    float r, g, b;
    CAP::GRAPHICS::Painter::rgb(line.color(), r, g, b);
    CGContextSetRGBStrokeColor(context, r, g, b, line.alpha());
    CGFloat w = line.width() > 0 ? line.width() : 1.0;
    CGContextSetLineWidth(context, w);
    CGContextSetLineJoin(context, kCGLineJoinRound);
    CGContextSetLineCap(context, kCGLineCapButt);
    switch (line.style())
      {
      case 2: { CGFloat d[] = { 8*w, 6*w };           CGContextSetLineDash(context, 0, d, 2); break; } // dashed
      case 3: { CGFloat d[] = { 1.5*w, 3*w };         CGContextSetLineDash(context, 0, d, 2); break; } // dotted
      case 4: { CGFloat d[] = { 8*w, 4*w, 1.5*w, 4*w }; CGContextSetLineDash(context, 0, d, 4); break; } // dash-dot
      default: CGContextSetLineDash(context, 0, NULL, 0); break;                                         // solid
      }
  }

  void setFill(CGContextRef context, int color, float alpha)
  {
    float r, g, b;
    CAP::GRAPHICS::Painter::rgb(color, r, g, b);
    CGContextSetRGBFillColor(context, r, g, b, alpha);
  }

  //! Add a closed regular-ish shape centered on c to the current path.
  void addPolygonPath(CGContextRef context, const std::vector<CGPoint> & points)
  {
    if (points.empty()) return;
    CGContextMoveToPoint(context, points[0].x, points[0].y);
    for (size_t k=1; k<points.size(); k++) CGContextAddLineToPoint(context, points[k].x, points[k].y);
    CGContextClosePath(context);
  }
} // anonymous namespace

//------------------------------------------------------------------------------
// MacOsCanvas
//------------------------------------------------------------------------------
namespace CAP
{
  namespace GRAPHICS
  {

  MacOsCanvas::MacOsCanvas(Canvas * canvas)
  :
  Painter(),
  _canvas(canvas),
  _context(nullptr),
  _width(canvas ? canvas->width()  : 800),
  _height(canvas ? canvas->height() : 600)
  { }

  void MacOsCanvas::show()
  {
  if (!_canvas) return;
  @autoreleasepool
    {
    [NSApplication sharedApplication];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

    // minimal menu so Cmd-W / Cmd-Q close the canvas window (leave an existing menu, e.g. ROOT's, alone)
    if (!NSApp.mainMenu)
      {
      NSMenu * menuBar = [[NSMenu alloc] init];
      NSMenuItem * appItem = [[NSMenuItem alloc] init];
      [menuBar addItem:appItem];
      NSMenu * appMenu = [[NSMenu alloc] init];
      [appMenu addItemWithTitle:@"Close Window" action:@selector(performClose:) keyEquivalent:@"w"];
      [appMenu addItemWithTitle:@"Quit" action:@selector(performClose:) keyEquivalent:@"q"];
      appItem.submenu = appMenu;
      NSApp.mainMenu = menuBar;
      }

    NSRect frame = NSMakeRect(0, 0, _canvas->width(), _canvas->height());
    NSWindowStyleMask style = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable
                            | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable;
    NSWindow * window = [[NSWindow alloc] initWithContentRect:frame
                                                    styleMask:style
                                                      backing:NSBackingStoreBuffered
                                                        defer:NO];
    window.releasedWhenClosed = NO; // ARC owns it
    NSString * title = [NSString stringWithUTF8String:_canvas->title().data()];
    window.title = title ? title : @"Canvas";

    CAPCanvasView * view = [[CAPCanvasView alloc] initWithFrame:frame];
    view.painter = this;
    window.contentView = view;

    CAPWindowDelegate * delegate = [[CAPWindowDelegate alloc] init]; // kept alive by this scope
    window.delegate = delegate;

    [window center];
    [window makeKeyAndOrderFront:nil];
    if (@available(macOS 14.0, *))
      [NSApp activate];
    else
      [NSApp activateIgnoringOtherApps:YES];

    [NSApp run]; // returns when the window closes (see CAPWindowDelegate)

    view.painter    = nullptr;
    window.delegate = nil;
    }
  }

  void MacOsCanvas::paint(void * cgContext, float width, float height)
  {
  if (!_canvas || !cgContext) return;
  CGContextRef context = (CGContextRef) cgContext;
  _context = cgContext;
  _width   = width;
  _height  = height;
  CGContextSaveGState(context);
  CGContextSetShouldAntialias(context, true);
  try
    {
    _canvas->draw(*this);
    }
  catch (const std::exception & e)
    {
    std::fprintf(stderr, "MacOsCanvas: error while drawing: %s\n", e.what());
    }
  catch (...)
    {
    std::fprintf(stderr, "MacOsCanvas: unknown error while drawing\n");
    }
  CGContextRestoreGState(context);
  _context = nullptr;
  }

  void MacOsCanvas::drawLine(const std::vector<Point2D> & points, const LineAttributes & line)
  {
  CGContextRef context = (CGContextRef) _context;
  if (!context || points.size() < 2 || line.style() == 0) return;
  setStroke(context, line);
  CGContextBeginPath(context);
  CGContextMoveToPoint(context, points[0].x()*_width, points[0].y()*_height);
  for (size_t k=1; k<points.size(); k++)
    CGContextAddLineToPoint(context, points[k].x()*_width, points[k].y()*_height);
  CGContextStrokePath(context);
  }

  void MacOsCanvas::drawPolygon(const std::vector<Point2D> & points, const FillAttributes & fill, const LineAttributes & line)
  {
  CGContextRef context = (CGContextRef) _context;
  if (!context || points.size() < 3) return;
  bool doFill   = fill.style() != 0;
  bool doStroke = line.style() != 0;
  if (!doFill && !doStroke) return;
  if (doFill)   setFill(context, fill.color(), fill.alpha());
  if (doStroke) setStroke(context, line);
  std::vector<CGPoint> device;
  for (const auto & p : points) device.push_back(CGPointMake(p.x()*_width, p.y()*_height));
  CGContextBeginPath(context);
  addPolygonPath(context, device);
  CGContextDrawPath(context, doFill ? (doStroke ? kCGPathFillStroke : kCGPathFill) : kCGPathStroke);
  }

  //!
  //! Marker styles follow ROOT: 1 dot, 2 plus, 3 asterisk, 5 cross, 6/7 small/medium dot,
  //! 8/20 full circle, 21 full square, 22 full triangle up, 23 full triangle down, 29 full star,
  //! 33 full diamond, 4/24 open circle, 25 open square, 26 open triangle up, 32 open triangle down,
  //! 27 open diamond, 30 open star. Other styles are drawn as full circles.
  //!
  void MacOsCanvas::drawMarker(const Point2D & position, const MarkerAttributes & marker)
  {
  CGContextRef context = (CGContextRef) _context;
  if (!context) return;
  CGFloat s = 4.0*marker.size(); // half-size; size 1 is 8 points across
  if (s <= 0) return;
  CGFloat x = position.x()*_width;
  CGFloat y = position.y()*_height;

  CGContextSaveGState(context);
  setFill(context, marker.color(), marker.alpha());
  float r, g, b;
  Painter::rgb(marker.color(), r, g, b);
  CGContextSetRGBStrokeColor(context, r, g, b, marker.alpha());
  CGContextSetLineWidth(context, 1.0);
  CGContextSetLineDash(context, 0, NULL, 0);

  enum Shape { kCircle, kSquare, kTriangleUp, kTriangleDown, kDiamond, kStar, kPlus, kCross, kAsterisk };
  Shape shape = kCircle;
  bool  open  = false;
  switch (marker.style())
    {
    case 1:  shape = kCircle; s = 0.75; break;
    case 6:  shape = kCircle; s = 1.0;  break;
    case 7:  shape = kCircle; s = 1.5;  break;
    case 2:  shape = kPlus;         break;
    case 3:  shape = kAsterisk;     break;
    case 5:  shape = kCross;        break;
    case 4:  case 24: shape = kCircle;       open = true; break;
    case 25: shape = kSquare;       open = true; break;
    case 26: shape = kTriangleUp;   open = true; break;
    case 32: shape = kTriangleDown; open = true; break;
    case 27: shape = kDiamond;      open = true; break;
    case 30: shape = kStar;         open = true; break;
    case 21: shape = kSquare;       break;
    case 22: shape = kTriangleUp;   break;
    case 23: shape = kTriangleDown; break;
    case 29: shape = kStar;         break;
    case 33: shape = kDiamond;      break;
    default: shape = kCircle;       break; // 8, 20 and anything else
    }

  CGContextBeginPath(context);
  switch (shape)
    {
    case kCircle:
      CGContextAddEllipseInRect(context, CGRectMake(x-s, y-s, 2*s, 2*s));
      break;
    case kSquare:
      CGContextAddRect(context, CGRectMake(x-s, y-s, 2*s, 2*s));
      break;
    case kTriangleUp:
      addPolygonPath(context, { CGPointMake(x-s, y-s), CGPointMake(x+s, y-s), CGPointMake(x, y+s) });
      break;
    case kTriangleDown:
      addPolygonPath(context, { CGPointMake(x-s, y+s), CGPointMake(x+s, y+s), CGPointMake(x, y-s) });
      break;
    case kDiamond:
      addPolygonPath(context, { CGPointMake(x, y-s), CGPointMake(x+s, y), CGPointMake(x, y+s), CGPointMake(x-s, y) });
      break;
    case kStar:
      {
      std::vector<CGPoint> star;
      for (int k=0; k<10; k++)
        {
        double phi = kPi/2 + k*kPi/5;
        double rad = (k % 2 == 0) ? s : 0.4*s;
        star.push_back(CGPointMake(x + rad*std::cos(phi), y + rad*std::sin(phi)));
        }
      addPolygonPath(context, star);
      break;
      }
    case kPlus:
    case kCross:
    case kAsterisk:
      {
      if (shape != kCross)
        {
        CGContextMoveToPoint(context, x-s, y); CGContextAddLineToPoint(context, x+s, y);
        CGContextMoveToPoint(context, x, y-s); CGContextAddLineToPoint(context, x, y+s);
        }
      if (shape != kPlus)
        {
        CGFloat d = (shape == kAsterisk) ? 0.7*s : s;
        CGContextMoveToPoint(context, x-d, y-d); CGContextAddLineToPoint(context, x+d, y+d);
        CGContextMoveToPoint(context, x-d, y+d); CGContextAddLineToPoint(context, x+d, y-d);
        }
      CGContextStrokePath(context);
      CGContextRestoreGState(context);
      return;
      }
    }
  CGContextDrawPath(context, open ? kCGPathStroke : kCGPathFill);
  CGContextRestoreGState(context);
  }

  void MacOsCanvas::drawText(const Point2D & position, const String & text, const TextAttributes & attributes, float angle)
  {
  CGContextRef context = (CGContextRef) _context;
  if (!context) return;
  const char * utf8 = text.data();
  if (!utf8 || !*utf8) return;
  CGFloat fontSize = attributes.size()*_height;
  if (fontSize <= 0) return;
  CTLineRef line = makeLine(utf8, attributes.font(), fontSize);
  if (!line) return;

  CGFloat ascent = 0, descent = 0, leading = 0;
  double w = CTLineGetTypographicBounds(line, &ascent, &descent, &leading);
  CGFloat dx = 0;
  if      (attributes.justification() == TextAttributes::kCenter) dx = -0.5*w;
  else if (attributes.justification() == TextAttributes::kRight)  dx = -w;
  CGFloat dy = -0.5*(ascent - descent); // vertically centered on position

  CGContextSaveGState(context);
  setFill(context, attributes.color(), attributes.alpha());
  CGContextTranslateCTM(context, position.x()*_width, position.y()*_height);
  CGContextRotateCTM(context, angle*kPi/180.0);
  CGContextSetTextMatrix(context, CGAffineTransformIdentity);
  CGContextSetTextPosition(context, dx, dy);
  CTLineDraw(line, context);
  CGContextRestoreGState(context);
  CFRelease(line);
  }

  float MacOsCanvas::textWidth(const String & text, const TextAttributes & attributes) const
  {
  const char * utf8 = text.data();
  if (!utf8 || !*utf8) return 0.0;
  CGFloat fontSize = attributes.size()*_height;
  if (fontSize <= 0) return 0.0;
  CTLineRef line = makeLine(utf8, attributes.font(), fontSize);
  if (!line) return Painter::textWidth(text, attributes);
  double w = CTLineGetTypographicBounds(line, NULL, NULL, NULL);
  CFRelease(line);
  return w;
  }

  } // namespace GRAPHICS
} // namespace CAP
