interface Window {
    chess: ChessEngine;
}

interface ChessEngine
{

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
	
initializeBoard(
	successCallback: () => void,
	errorCallback: () => void
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