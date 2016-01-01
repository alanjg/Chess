// For an introduction to the Blank template, see the following documentation:
// http://go.microsoft.com/fwlink/?LinkID=397705
// To debug code on page load in Ripple or on Android devices/emulators: launch your app, set breakpoints, 
// and then run "window.location.reload()" in the JavaScript Console.
/// <reference path="../plugins/com.sjelkjd.MochaChess/www/MochaChess.d.ts" />
module MochaChessApp {
    "use strict";

    class Position {
        fen: string;
        move: Move;
        constructor() { }
    }

    class Move {
        startRow: number;
        startCol: number;
        endRow: number;
        endCol: number;
        constructor() { }
    }

    enum Color {
        White, Black
    }

    export module Application {
        export var positions: Position[] = [];
        export var engine: ChessEngine;
        export var pieces = [];
        export var squares: HTMLElement[][] = [];
	    export var playerMoving: boolean = false;
        export var computerMoving: boolean = false;
        export var color: Color;
	    export var startR: number = 0;
    	export var startC: number = 0;
	    export var startX: number = 0;
        export var startY: number = 0;
        export var hasTouch: boolean = false;

        export function initialize() {
            document.addEventListener('deviceready', onDeviceReady, false);
        }

        function getPieceName(piece: string): string {
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

        function onDeviceReady() {
            document.addEventListener('pause', onPause, false);
            document.addEventListener('resume', onResume, false);        
            document.addEventListener('touchmove', function (e) { e.preventDefault(); }, false);

            Application.engine = window.chess;
        }

        export function setPositions(data: StoredPosition[]) {
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
                Application.positions.push(position);
            }
        }

        export function solveProblem(index: number) {
            var fen: string = Application.positions[index].fen;
            var color: Color = fen[fen.indexOf(' ') + 1] == 'w' ? Color.White : Color.Black;
            Application.color = color;
            setupBoard(color);
            Application.engine.setPosition(() => { }, () => { }, fen);
            initializeBoard();
        }

        export function startGame(white: boolean) {
            Application.color = white ? Color.White : Color.Black;
            setupBoard(color);
            Application.engine.setStartPosition(() => { }, () => { });
            initializeBoard();
            if (color == Color.Black) {
                Application.computerMoving = true;
                setTimeout(function () {
                    engine.makeBestMove(() => {
                        updateBoard();
                        Application.computerMoving = false;
                    }, () => { })
                }, 0);
            }
        }

        function adjustRowForColor(index: number) {
            if (Application.color == Color.White) {
                return 7 - index;
            }
            else {
                return index;
            }
        }
        function adjustColForColor(index: number) {
            if (Application.color == Color.Black) {
                return 7 - index;
            }
            else {
                return index;
            }
        }

        function setupBoard(color: Color) {
            var board = document.getElementById('board');

            for (var i = 0; i < 8; i++) {
                Application.pieces[i] = [];
                Application.squares[i] = [];
                for (var j = 0; j < 8; j++) {
                    var square = document.createElement('div');
                    var x = .125 * adjustColForColor(j) * 100;
                    var y = .125 * adjustRowForColor(i) * 100;
                    square.style.left = x + "%";
                    square.style.top = y + "%";
                    square.classList.add('square');
                    if (((i + j) % 2) == 1) {
                        square.classList.add('darkSquare');
                    }
                    board.appendChild(square);
                    squares[i][j] = square;
                }
            }

            board.onmousedown = onMouseDown;
            board.onmousemove = onMouseMove;
            board.ontouchstart = onTouchStart;
        }

        function onPause() {
            // TODO: This application has been suspended. Save application state here.
        }

        function onResume() {
            // TODO: This application has been reactivated. Restore application state here.
        }

        function onMouseMove(event: MouseEvent) {
            /*
            if (Application.computerMoving) return;
            if (Application.playerMoving) {
                var piece = Application.pieces[Application.startR][Application.startC];
                var L = (event.x - Application.startX).toString() + "px";
                var T = (event.y - Application.startY).toString() + "px";
                piece.style.margin = T + " 0 0 " + L;
            }
            */
        }

        function onMouseDown(event: MouseEvent) {
            if (!Application.hasTouch) {

                var board = document.querySelector('#board');
                processMove(event.x, event.y);
            }
        }

        function onTouchStart(event: TouchEvent) {
            Application.hasTouch = true;
            for (var i = 0; i < event.targetTouches.length; i++) {
                processMove(event.targetTouches[i].clientX, event.targetTouches[i].clientY);
            }
        }

        function processMove(x, y) {
            if (Application.computerMoving) return;

            var engine = Application.engine;
            var board = document.querySelector('#board');
            var rect = board.getBoundingClientRect();

            var xx = x - rect.left;
            var yy = y - rect.top;
            var r = adjustRowForColor(Math.ceil((yy / rect.height) * 8) - 1);
			var c = adjustColForColor(Math.ceil((xx / rect.width) * 8) - 1);
            
            if (0 <= r && r <= 7 && 0 <= c && c <= 7) {
                if (Application.playerMoving) {
                    var move = {
                        startRow: Application.startR, startCol: Application.startC, endRow: r, endCol: c
                    };
                    Application.squares[Application.startR][Application.startC].classList.remove('moveSource');
                    engine.isValidMove((result: boolean) => {
                        if (result) {
                            engine.makeMove(() => {
                                updateBoard();
                                Application.playerMoving = false;
                                Application.computerMoving = true;
                                setTimeout(function () {
                                    engine.makeBestMove(() => {
                                        updateBoard();
                                        Application.computerMoving = false;
                                    }, () => { })
                                }, 0);
                            }, () => { }, move);
                        }
                        else {
                            // cancel
                            Application.playerMoving = false;
                            var piece = Application.pieces[Application.startR][Application.startC];
                            piece.style.removeProperty("margin");
                        }
                    }, () => {}, move);
                }
                else {
                    var square = { row: r, col: c };

                    if (engine.isValidMoveStart((result: boolean) => {
                        if (result) {
                            Application.playerMoving = true;
                            Application.startR = r;
                            Application.startC = c;
                            Application.startX = x;
                            Application.startY = y;
                            Application.squares[r][c].classList.add('moveSource');
                        }
                    }, () => { }, square));
                }
            }
        }

        function addPiece(pieceName, row, col) {
            var engine = Application.engine;
            var pieces = Application.pieces;
            var board = document.querySelector('#board');
            var piece: HTMLElement = <HTMLElement>document.querySelector('#' + getPieceName(pieceName)).cloneNode(true);
            var x = .125 * adjustColForColor(col) * 100;
            var y = .125 * adjustRowForColor(row) * 100;
            piece.style.left = x + "%";
            piece.style.top = y + "%";
            piece.style.visibility = "visible";
            piece.classList.add('piece');
            board.insertBefore(piece, null);
            pieces[row][col] = piece;
        }

        function removePiece(row, col) {
            var pieces = Application.pieces;
            if (pieces[row][col] != null) {
                var board = document.querySelector('#board');
                board.removeChild(pieces[row][col]);
                pieces[row][col] = null;
            }
        }

        function setPiece(r, c) {
            var square = { row: r, col: c };
            engine.getPiece((piece: string) => {
                if (piece != '') {
                    addPiece(piece, r, c);
                }
            }, () => { }, square);
        }
        
        function initializeBoard() {
            var r = 0;
            var c = 0;
            for (r = 0; r < 8; r++) {
                for (c = 0; c < 8; c++) {
                    setPiece(r, c);
                }
            }
        }

        function updatePiece(r,  c) {
            var square = { row: r, col: c };
                    Application.engine.getPiece((newPiece: string) => {
                        var oldPiece = Application.pieces[r][c];
                        if (newPiece != '') {
                            if (oldPiece != null && oldPiece.id != getPieceName(newPiece)) {
                                removePiece(r, c);
                                addPiece(newPiece, r, c);
                            }
                            else if (oldPiece == null) {
                                addPiece(newPiece, r, c);
                            }
                        }
                        else {
                            if (oldPiece != null) {
                                removePiece(r, c);
                            }
                        }
                    }, () => { }, square);
        }
        
        function updateBoard() {
            var r = 0;
            var c = 0;
            for (r = 0; r < 8; r++) {
                for (c = 0; c < 8; c++) {
                    updatePiece(r, c);
                }
            }
        }

    }

    window.onload = function () {
        Application.initialize();
    }
}
