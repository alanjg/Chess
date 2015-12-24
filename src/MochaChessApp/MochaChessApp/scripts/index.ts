// For an introduction to the Blank template, see the following documentation:
// http://go.microsoft.com/fwlink/?LinkID=397705
// To debug code on page load in Ripple or on Android devices/emulators: launch your app, set breakpoints, 
// and then run "window.location.reload()" in the JavaScript Console.
/// <reference path="../plugins/com.sjelkjd.MochaChess/www/MochaChess.d.ts" />
module MochaChessApp {
    "use strict";

    export module Application {
        export var engine: ChessEngine;
		export var pieces = []; 
		export var playerMoving: boolean = false;
		export var computerMoving: boolean = false;
		export var startR: number = 0;
		export var startC: number = 0;
		export var startX: number = 0;
		export var startY: number = 0;
        
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

            for (var i = 0; i < 8; i++) {
                Application.pieces[i] = [];
                for (var j = 0; j < 8; j++) {
                    var square = document.createElement('div');
                    var x = .125 * j * 100;
                    var y = .125 * i * 100;
                    square.style.left = x + "%";
                    square.style.top = y + "%";
                    square.classList.add('square');
                    if (((i + j) % 2) == 0) {
                        square.classList.add('darkSquare');
                    }
                    document.getElementById('board').appendChild(square);
                }
            }
            engine = window.chess;
            initializeBoard();
            document.body.onmousedown = onMouseDown;
            document.body.onmousemove = onMouseMove;

        }

        function onPause() {
            // TODO: This application has been suspended. Save application state here.
        }

        function onResume() {
            // TODO: This application has been reactivated. Restore application state here.
        }

        function onMouseMove(event: MouseEvent) {
            if (Application.computerMoving) return;
            if (Application.playerMoving) {
                var piece = Application.pieces[Application.startR][Application.startC];
                var L = (event.x - Application.startX).toString() + "px";
                var T = (event.y - Application.startY).toString() + "px";
                piece.style.margin = T + " 0 0 " + L;
            }
        }

        function onMouseDown(event: MouseEvent) {
            if (Application.computerMoving) return;

            var engine = Application.engine;
            var board = document.querySelector('#board');
            var x = event.x;
            var y = event.y;
            var rect = board.getBoundingClientRect();

            var xx = x - board.clientLeft;
            var yy = y - board.clientTop;
            var r = 7 - (Math.ceil((yy / rect.height) * 8) - 1);
			var c = Math.ceil((xx / rect.width) * 8) - 1;
            
            if (0 <= r && r <= 7 && 0 <= c && c <= 7) {
                if (Application.playerMoving) {
                    var move = {
                        startRow: Application.startR, startCol: Application.startC, endRow: r, endCol: c
                    };
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
            var x = .125 * col * 100;
            var y = .125 * (7-row) * 100;
            piece.style.left = x + "%";
            piece.style.top = y + "%";
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
            var engine = Application.engine;
            engine.initializeBoard(() => { }, () => { });
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
                            if (oldPiece != null && oldPiece.id != newPiece) {
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
