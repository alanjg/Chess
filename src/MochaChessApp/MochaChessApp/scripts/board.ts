module MochaChessApp {
    "use strict";

    export class Board {
        hasTouch: boolean = false;
        acceptingPlayerInput: boolean = false;
        hasPickedStartSquare: boolean = false;
        playerMovedCallback: any;
        startRow: number;
        startCol: number;
        boardElement: HTMLElement;
        squares: HTMLElement[][] = [];
        pieces = [];
        engine: ChessEngine;
        playerColor: Color;
        constructor(boardElement: HTMLElement) {
            this.boardElement = boardElement;
            this.engine = window.chess;
        }

        waitForInputMove(callback): any {
            this.acceptingPlayerInput = true;
            this.hasPickedStartSquare = false;
            this.playerMovedCallback = callback;
        }

        getPieceName(piece: string): string {
            switch (piece) {
                case "r": return "darkRook";
                case "R": return "lightRook";
                case "n": return "darkKnight";
                case "N": return "lightKnight";
                case "b": return "darkBishop";
                case "B": return "lightBishop";
                case "k": return "darkKing";
                case "K": return "lightKing";
                case "q": return "darkQueen";
                case "Q": return "lightQueen";
                case "p": return "darkPawn";
                case "P": return "lightPawn";
                default: return "";
            }
        }

        adjustRowForColor(index: number): number {
            if (this.playerColor == Color.White) {
                return 7 - index;
            }
            else {
                return index;
            }
        }

        adjustColForColor(index: number): number {
            if (this.playerColor == Color.Black) {
                return 7 - index;
            }
            else {
                return index;
            }
        }

        setupBoard(color: Color): void {
            this.playerColor = color;
            var board = document.getElementById('board');

            for (var i = 0; i < 8; i++) {
                this.pieces[i] = [];
                this.squares[i] = [];
                for (var j = 0; j < 8; j++) {
                    var square = document.createElement('div');
                    var x = .125 * this.adjustColForColor(j) * 100;
                    var y = .125 * this.adjustRowForColor(i) * 100;
                    square.style.left = x + "%";
                    square.style.top = y + "%";
                    square.classList.add('square');
                    if (((i + j) % 2) == 1) {
                        square.classList.add('darkSquare');
                    }
                    board.appendChild(square);
                    this.squares[i][j] = square;
                }
            }
            
            board.onmousedown = (e) => { this.onMouseDown(e) };
            board.ontouchstart = (e) => { this.onTouchStart(e) };
        }

        teardownBoard(): void {
            var board = document.getElementById('board');
            for (var i = board.children.length - 1; i >= 0; i--) {
                board.removeChild(board.children[i]);
            }
            board.removeEventListener('onmousedown', this.onMouseDown);
            board.removeEventListener('ontouchstart', this.onTouchStart);
        }

        onMouseDown(event: MouseEvent): void {
            if (!this.hasTouch) {
                var board = document.querySelector('#board');
                this.processMove(event.x, event.y);
            }
        }

        onTouchStart(event: TouchEvent): void {
            this.hasTouch = true;
            for (var i = 0; i < event.targetTouches.length; i++) {
                this.processMove(event.targetTouches[i].clientX, event.targetTouches[i].clientY);
            }
        }

        processMove(x, y): void {
            if (this.acceptingPlayerInput) {
                var rect = this.boardElement.getBoundingClientRect();
                var xx = x - rect.left;
                var yy = y - rect.top;
                var r = this.adjustRowForColor(Math.ceil((yy / rect.height) * 8) - 1);
                var c = this.adjustColForColor(Math.ceil((xx / rect.width) * 8) - 1);

                if (0 <= r && r <= 7 && 0 <= c && c <= 7) {
                    if (this.hasPickedStartSquare) {
                        var move = {
                            startRow: this.startRow, startCol: this.startCol, endRow: r, endCol: c
                        };
                        this.squares[this.startRow][this.startCol].classList.remove('moveSource');
                        this.engine.isValidMove((result: boolean) => {
                            if (result) {
                                this.engine.makeMove(() => {
                                    this.updateBoard(() => {
                                        this.acceptingPlayerInput = false;
                                        this.playerMovedCallback(move);
                                    });                                    
                                }, () => { }, move);
                            }
                            else {
                                // cancel
                                this.hasPickedStartSquare = false;
                            }
                        }, () => { }, move);
                    }
                    else {
                        var square = { row: r, col: c };

                        if (this.engine.isValidMoveStart((result: boolean) => {
                            if (result) {
                                this.hasPickedStartSquare = true;
                                this.startRow = r;
                                this.startCol = c;
                                this.squares[r][c].classList.add('moveSource');
                            }
                        }, () => { }, square));
                    }
                }
            }
        }

        addPiece(pieceName, row, col): void {
            var engine = this.engine;
            var pieces = this.pieces;
            var board = document.querySelector('#board');
            var piece : HTMLElement = <HTMLElement>document.querySelector('#' + this.getPieceName(pieceName)).cloneNode(true);
            var x = .125 * this.adjustColForColor(col) * 100;
            var y = .125 * this.adjustRowForColor(row) * 100;
            piece.style.left = x + "%";
            piece.style.top = y + "%";
            piece.style.visibility = "visible";
            piece.classList.add('piece');
            board.insertBefore(piece, null);
            pieces[row][col] = piece;
        }

        removePiece(row, col): void {
            var pieces = this.pieces;
            if (pieces[row][col] != null) {
                var board = document.querySelector('#board');
                board.removeChild(pieces[row][col]);
                pieces[row][col] = null;
            }
        }

        setPiece(r, c): void {
            var square = { row: r, col: c };
            this.engine.getPiece((piece: string) => {
                if (piece != '') {
                    this.addPiece(piece, r, c);
                }
            }, () => { }, square);
        }

        initializeBoard(): void {
            var r = 0;
            var c = 0;
            for (r = 0; r < 8; r++) {
                for (c = 0; c < 8; c++) {
                    this.setPiece(r, c);
                }
            }
        }

        updatePiece(r, c, newPiece): void {
            var square = { row: r, col: c };
            var oldPiece = this.pieces[r][c];
            if (newPiece != '' && newPiece != '.') {
                if (oldPiece != null && oldPiece.id != this.getPieceName(newPiece)) {
                    this.removePiece(r, c);
                    this.addPiece(newPiece, r, c);
                }
                else if (oldPiece == null) {
                    this.addPiece(newPiece, r, c);
                }
            }
            else {
                if (oldPiece != null) {
                    this.removePiece(r, c);
                }
            }
        }

        updateBoard(complete): void {
            this.engine.getBoard((result) => {
                var r = 0;
                var c = 0;
                for (r = 0; r < 8; r++) {
                    for (c = 0; c < 8; c++) {
                        this.updatePiece(r, c, result[(7-r)*9+c]);
                    }
                }
                complete();
            }, () => { });
        }
    }
}