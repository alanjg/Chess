/// <reference path="../plugins/com.sjelkjd.MochaChess/www/MochaChess.d.ts" />
/// <reference path="move.ts"/> 
/// <reference path="color.ts"/> 
/// <reference path="position.ts"/> 
/// <reference path="playComputer.ts"/> 
module MochaChessApp {
    "use strict";

    export class ChessProblemList {
        positions: Position[] = [];
        initialize(data: StoredPosition[]) {
            for (var i = 0; i < data.length; i++) {
                var position: Position = new Position();
                position.fen = data[i].fen;
                var moveValue: string = data[i].move;
                var move: Move = new Move();
                move.startRow = parseInt(moveValue[0]);
                move.startCol = parseInt(moveValue[1]);
                move.endRow = parseInt(moveValue[2]);
                move.endCol = parseInt(moveValue[3]);
                position.move = move;
                this.positions.push(position);
            }
        }

        getProblem(index: number): ChessProblem {
            var fen = this.positions[index].fen;
            var problem = new ChessProblem(fen, fen[fen.indexOf(' ') + 1] == 'w' ? Color.White : Color.Black, this.positions[index].move);

            return problem;
        }
    }

    export class ChessProblem {
        fen: string;
        color: Color;
        expectedMove: Move;
        constructor(fen: string, color: Color, expectedMove: Move) {
            this.fen = fen;
            this.color = color;
            this.expectedMove = expectedMove;
        }
    }

    export class ChessProblemSolver {
        problem: ChessProblem;
        board: Board;
        successCallback: any;
        failureCallback: any;
        constructor(problem: ChessProblem, boardElement: HTMLElement) {
            this.problem = problem;
            this.board = new Board(boardElement);
            this.board.setupBoard(this.problem.color);
            this.board.engine.setPosition(() => {
                this.board.initializeBoard();
            }, () => { }, this.problem.fen);
            
        }

        resetBoard(complete) {
            this.board.engine.setPosition(() => {
                this.board.updateBoard(() => {
                    complete();
                });
            }, () => { }, this.problem.fen);
            
        }

        waitForInput(successCallback, failureCallback) {
            this.successCallback = successCallback;
            this.failureCallback = failureCallback;
            this.board.waitForInputMove((move) => this.onInputMove(move));
        }

        onInputMove(move: Move) {
            if (this.problem.expectedMove.startRow == move.startRow &&
                this.problem.expectedMove.startCol == move.startCol &&
                this.problem.expectedMove.endRow == move.endRow &&
                this.problem.expectedMove.endCol == move.endCol) {
                this.successCallback();
            } else {
                this.failureCallback();
            }
        }

    }
}