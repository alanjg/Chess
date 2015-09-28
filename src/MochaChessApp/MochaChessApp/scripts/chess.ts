class Piece {
	public name: string;
	public row: number;
	public col: number;
	public color: string;
}

class ChessEngine
{
    isValidMove(sr: number, sc: number, er: number, ec: number): boolean {
        return true;
    }
    makeMove(sr: number, sc: number, er: number, ec: number) {
    }

    makeBestMove() {
    }

    isValidMoveStart(sr: number, sc: number): boolean {
        return true;
    }

    initializeBoard() {

    }

    getPiece(r: number, c: number): string {
        
        return "lightKing";
    }
};


