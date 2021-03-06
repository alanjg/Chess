#import <Foundation/Foundation.h>
#import <CoreLocation/CoreLocation.h>
#import <CoreLocation/CLLocationManager.h>
#import <Cordova/CDVPlugin.h>

@interface MochaChessEngine : CDVPlugin
{}

- (void)getBoard:(CDVInvokedUrlCommand*)command;
- (void)getPiece:(CDVInvokedUrlCommand*)command;
- (void)makeBestMove:(CDVInvokedUrlCommand*)command;
- (void)makeMove:(CDVInvokedUrlCommand*)command;
- (void)isValidMoveStart:(CDVInvokedUrlCommand*)command;
- (void)isValidMove:(CDVInvokedUrlCommand*)command;
- (void)setPosition:(CDVInvokedUrlCommand*)command;
- (void)setStartPosition:(CDVInvokedUrlCommand*)command;

@end