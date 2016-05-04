module MochaChessApp {
    "use strict";
    export class ChessGame {
        board: Board;
        computerMoving: boolean;
        onGameOver: any;
        constructor(boardElement: HTMLElement, playerColor: Color, onGameOver) {
            this.onGameOver = onGameOver;
            this.board = new Board(boardElement);
            this.board.setupBoard(playerColor);
            this.board.engine.setStartPosition(() => { }, () => { });
            this.board.initializeBoard();
            if (playerColor == Color.Black) {
                this.doComputerMove();
            } else {
                this.board.waitForInputMove((move) => { this.onInputMove(move); });
            }    
        }

        onInputMove(move: Move) {
            this.doComputerMove();
        }

        doComputerMove() {
            this.computerMoving = true;
            setTimeout(() => {
                this.board.engine.makeBestMove(() => {
                    this.board.updateBoard(() => {
                        this.computerMoving = false;
                        this.board.waitForInputMove((move) => { this.onInputMove(move); });
                    });                    
                }, () => { })
            }, 0);
        }
    }
}