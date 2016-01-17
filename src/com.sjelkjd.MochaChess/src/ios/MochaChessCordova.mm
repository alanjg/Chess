#import "MochaChessCordova.h"
#import <MobileCoreServices/UTCoreTypes.h>
#import <objc/message.h>
#import "pch.h"
#import "Board.h"
#import "Search.h"
#import "MoveGenerator.h"
#import "Types.h"
using namespace SjelkjdChessEngine;

@interface MochaChessEngine ()
@end

@implementation MochaChessEngine

std::auto_ptr<SjelkjdChessEngine::Board> board;
std::auto_ptr<SjelkjdChessEngine::Search> search;
std::auto_ptr<SjelkjdChessEngine::MoveGenerator> moveGenerator;

+ (void)initialize
{
    SjelkjdChessEngine::Board* b = new SjelkjdChessEngine::Board();
    board.reset(b);
    search.reset(new SjelkjdChessEngine::Search(*b));
    board->SetFEN(SjelkjdChessEngine::Board::startPosition);
    moveGenerator.reset(new SjelkjdChessEngine::MoveGenerator(*b));
}


- (void)getBoard:(CDVInvokedUrlCommand*)command
{
    std::string b = board->ToString();
    CDVPluginResult* pluginResult = nil;
    NSString* result = [NSString stringWithUTF8String:b.c_str()];
    pluginResult = [CDVPluginResult resultWithStatus:CDVCommandStatus_OK messageAsString:result];
    [self.commandDelegate sendPluginResult:pluginResult callbackId:command.callbackId];
}

- (void)getPiece:(CDVInvokedUrlCommand*)command
{
    NSDictionary* args = [command.arguments objectAtIndex:0];
    NSNumber* r = args[@"row"];
    NSNumber* c = args[@"col"];
    int row = [r intValue];
    int col = [c intValue];
    int piece = board->GetPiece(SjelkjdChessEngine::Square(row, col));
    std::string p;
    switch (piece)
    {
        case Pieces::BlackBishop: p = "b"; break;
        case Pieces::BlackKing: p = "k"; break;
        case Pieces::BlackKnight: p = "n"; break;
        case Pieces::BlackPawn: p = "p"; break;
        case Pieces::BlackQueen: p = "q"; break;
        case Pieces::BlackRook: p = "r"; break;
        case Pieces::None: p = ""; break;
        case Pieces::WhiteBishop: p = "B"; break;
        case Pieces::WhiteKing: p = "K"; break;
        case Pieces::WhiteKnight: p = "N"; break;
        case Pieces::WhitePawn: p = "P"; break;
        case Pieces::WhiteQueen: p = "Q"; break;
        case Pieces::WhiteRook: p = "R"; break;
    }
    CDVPluginResult* pluginResult = nil;
    NSString* result = [NSString stringWithUTF8String:p.c_str()];
    pluginResult = [CDVPluginResult resultWithStatus:CDVCommandStatus_OK messageAsString:result];
    [self.commandDelegate sendPluginResult:pluginResult callbackId:command.callbackId];
}



- (void)makeBestMove:(CDVInvokedUrlCommand*)command
{
    [self.commandDelegate runInBackground:^{
        int move = search->GetBestMove(1.0);
        board->MakeMove(move, true);
    
        CDVPluginResult* pluginResult = nil;
        pluginResult = [CDVPluginResult resultWithStatus:CDVCommandStatus_OK];
        [self.commandDelegate sendPluginResult:pluginResult callbackId:command.callbackId];
    }];
}

- (void)makeMove:(CDVInvokedUrlCommand*)command
{
    NSDictionary* args = [command.arguments objectAtIndex:0];
    NSNumber* startRow = args[@"startRow"];
    NSNumber* startCol = args[@"startCol"];
    NSNumber* endRow = args[@"endRow"];
    NSNumber* endCol = args[@"endCol"];
    
    int sr = [startRow intValue];
    int sc = [startCol intValue];
    int er = [endRow intValue];
    int ec = [endCol intValue];
    vector<int> moves;
    moveGenerator->GenerateAllMoves(moves);
    int foundMove = NullMove;
    for(int i=0;i<moves.size();i++)
    {
        int move = moves[i];
        int source = GetSourceFromMove(move);
        int dest = GetDestFromMove(move);
        if (sr == GetRow(source) && sc == GetCol(source) && er == GetRow(dest) && ec == GetCol(dest))
        {
            foundMove = move;
            break;
        }
    }
    if (foundMove != NullMove)
    {
        board->MakeMove(foundMove, true);
    }
    
    CDVPluginResult* pluginResult = nil;
    pluginResult = [CDVPluginResult resultWithStatus:CDVCommandStatus_OK];
    [self.commandDelegate sendPluginResult:pluginResult callbackId:command.callbackId];
}

- (void)isValidMoveStart:(CDVInvokedUrlCommand*)command
{
    BOOL result = false;
    
    NSDictionary* args = [command.arguments objectAtIndex:0];
    NSNumber* r = args[@"row"];
    NSNumber* c = args[@"col"];
    int row = [r intValue];
    int col = [c intValue];
    vector<int> moves;
    moveGenerator->GenerateAllMoves(moves);
    for(int i=0;i<moves.size();i++)
    {
        int move = moves[i];
        int source = GetSourceFromMove(move);
        int dest = GetDestFromMove(move);
        if (row == GetRow(source) && col == GetCol(source))
        {
            result = true;
            break;
        }
    }
    
    CDVPluginResult* pluginResult = nil;
    pluginResult = [CDVPluginResult resultWithStatus:CDVCommandStatus_OK messageAsBool:result];
    [self.commandDelegate sendPluginResult:pluginResult callbackId:command.callbackId];
}

- (void)isValidMove:(CDVInvokedUrlCommand*)command
{
    BOOL result = false;
    
    NSDictionary* args = [command.arguments objectAtIndex:0];
    NSNumber* startRow = args[@"startRow"];
    NSNumber* startCol = args[@"startCol"];
    NSNumber* endRow = args[@"endRow"];
    NSNumber* endCol = args[@"endCol"];
    
    int sr = [startRow intValue];
    int sc = [startCol intValue];
    int er = [endRow intValue];
    int ec = [endCol intValue];
    vector<int> moves;
    moveGenerator->GenerateAllMoves(moves);
    for(int i=0;i<moves.size();i++)
    {
        int move = moves[i];
        int source = GetSourceFromMove(move);
        int dest = GetDestFromMove(move);
        if (sr == GetRow(source) && sc == GetCol(source) && er == GetRow(dest) && ec == GetCol(dest))
        {
            result = true;
            break;
        }
    }
              
    CDVPluginResult* pluginResult = nil;
    pluginResult = [CDVPluginResult resultWithStatus:CDVCommandStatus_OK messageAsBool:result];
    [self.commandDelegate sendPluginResult:pluginResult callbackId:command.callbackId];

}

- (void)setStartPosition:(CDVInvokedUrlCommand*)command
{
    board->SetFEN(Board::startPosition);
    
    CDVPluginResult* pluginResult = nil;
    pluginResult = [CDVPluginResult resultWithStatus:CDVCommandStatus_OK];
    [self.commandDelegate sendPluginResult:pluginResult callbackId:command.callbackId];
}

- (void)setPosition:(CDVInvokedUrlCommand*)command
{
	NSString* fen = [command.arguments objectAtIndex:0];
    board->SetFEN(fen);
    
    CDVPluginResult* pluginResult = nil;
    pluginResult = [CDVPluginResult resultWithStatus:CDVCommandStatus_OK];
    [self.commandDelegate sendPluginResult:pluginResult callbackId:command.callbackId];
}

@end