interface Window {
    chess: ChessEngine;
}

interface ChessEngine
{
getBoard(
	successCallback: (result: string) => void,
	errorCallback: () => void
	): void;

getPiece(
	successCallback: (result: string) => void,
	errorCallback: () => void,
	input: Square
	): void;

makeBestMove(
	successCallback: () => void,
	errorCallback: () => void
): void;

makeMove(
	successCallback: () => void,
	errorCallback: () => void,
	input: Move
	): void;
	
isValidMoveStart(
	successCallback: (result: boolean) => void,
	errorCallback: () => void,
	input: Square
	): void;
	
isValidMove(
	successCallback: (result: boolean) => void,
	errorCallback: () => void,
	input: Move
	): void;
	
setStartPosition(
	successCallback: () => void,
	errorCallback: () => void
	): void;
	
setPosition(
	successCallback: () => void,
	errorCallback: () => void,
	input: string
	): void;

}

interface Square {
	row: number;
	col: number;
}

interface Move {
	startRow: number;
	startCol: number;
	endRow: number;
	endCol: number;
}

interface StoredPosition {
    fen: string;
    move: string;
}

interface Position {
    fen: string;
    move: Move;
}