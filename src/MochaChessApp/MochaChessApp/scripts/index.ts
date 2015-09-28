// For an introduction to the Blank template, see the following documentation:
// http://go.microsoft.com/fwlink/?LinkID=397705
// To debug code on page load in Ripple or on Android devices/emulators: launch your app, set breakpoints, 
// and then run "window.location.reload()" in the JavaScript Console.
module MochaChessApp {
    "use strict";

    export module Application {
		export var engine: ChessEngine = new ChessEngine();
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

        function onDeviceReady() {
            // Handle the Cordova pause and resume events
            document.addEventListener('pause', onPause, false);
            document.addEventListener('resume', onResume, false);

            for (var i = 0; i < 8; i++) {
                Application.pieces[i] = [];
                for (var j = 0; j < 8; j++) {
                    var square = document.createElement('div');
                    var x = .125 * j * 100;
                    var y = .125 * i * 100;
                    square.style.position = 'absolute';
                    square.style.left = x + "%";
                    square.style.top = y + "%";
                    square.style.width = "12.5%";
                    square.style.height = "12.5%";

                    if (((i + j) % 2) == 0) {
                        square.className = 'darkSquare';
                    }
                    document.getElementById('board').appendChild(square);
                }
            }


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
            var r = Math.ceil((yy / rect.height) * 8) - 1;
			var c = Math.ceil((xx / rect.width) * 8) - 1;


            if (0 <= r && r <= 7 && 0 <= c && c <= 7) {
                if (Application.playerMoving) {

                    if (engine.isValidMove(Application.startR, Application.startC, r, c)) {
                        engine.makeMove(Application.startR, Application.startC, r, c);
                        updateBoard();
                        Application.playerMoving = false;
                        Application.computerMoving = true;
                        engine.makeBestMove();
                        updateBoard();
                        Application.computerMoving = false;
                    }
                    else {
                        // cancel
                        Application.playerMoving = false;
                        var piece = Application.pieces[Application.startR][Application.startC];
                        piece.style.removeProperty("margin");
                    }
                }
                else {
                    if (engine.isValidMoveStart(r, c)) {
                        Application.playerMoving = true;
                        Application.startR = r;
                        Application.startC = c;
                        Application.startX = x;
                        Application.startY = y;
                    }
                }
            }
        }

        function addPiece(pieceName, row, col) {
            var engine = Application.engine;
            var pieces = Application.pieces;
            var board = document.querySelector('#board');
            var piece: HTMLElement = <HTMLElement>document.querySelector('#' + pieceName).cloneNode(true);
            piece.style.visibility = 'visible';
			var x = .125 * col * 100;
            var y = .125 * row * 100;
            piece.style.position = 'absolute';
            piece.style.left = x + "%";
            piece.style.top = y + "%";
            piece.style.width = "12.5%";
            piece.style.height = "12.5%";
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

        function initializeBoard() {
            var engine = Application.engine;
            engine.initializeBoard();
            var r = 0;
            var c = 0;
            for (r = 0; r < 8; r++) {
                for (c = 0; c < 8; c++) {
                    var piece = engine.getPiece(r, c);
                    if (piece != '') {
                        addPiece(piece, r, c);
                    }
                }
            }
        }

        function updateBoard() {
            var engine = Application.engine;
            var pieces = Application.pieces;

            var r = 0;
            var c = 0;
            for (r = 0; r < 8; r++) {
                for (c = 0; c < 8; c++) {
                    var newPiece = engine.getPiece(r, c);
                    var oldPiece = pieces[r][c];
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
                }
            }
        }

    }

    window.onload = function () {
        Application.initialize();
    }
}
