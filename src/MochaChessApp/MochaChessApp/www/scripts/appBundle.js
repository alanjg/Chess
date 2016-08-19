var MochaChessApp;
(function (MochaChessApp) {
    "use strict";
    var Move = (function () {
        function Move() {
        }
        return Move;
    })();
    MochaChessApp.Move = Move;
})(MochaChessApp || (MochaChessApp = {}));
var MochaChessApp;
(function (MochaChessApp) {
    "use strict";
    var Board = (function () {
        function Board(boardElement) {
            this.hasTouch = false;
            this.acceptingPlayerInput = false;
            this.hasPickedStartSquare = false;
            this.squares = [];
            this.pieces = [];
            this.boardElement = boardElement;
            this.engine = window.chess;
        }
        Board.prototype.waitForInputMove = function (callback) {
            this.acceptingPlayerInput = true;
            this.hasPickedStartSquare = false;
            this.playerMovedCallback = callback;
        };
        Board.prototype.getPieceName = function (piece) {
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
        };
        Board.prototype.adjustRowForColor = function (index) {
            if (this.playerColor == MochaChessApp.Color.White) {
                return 7 - index;
            }
            else {
                return index;
            }
        };
        Board.prototype.adjustColForColor = function (index) {
            if (this.playerColor == MochaChessApp.Color.Black) {
                return 7 - index;
            }
            else {
                return index;
            }
        };
        Board.prototype.setupBoard = function (color) {
            var _this = this;
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
            board.onmousedown = function (e) { _this.onMouseDown(e); };
            board.ontouchstart = function (e) { _this.onTouchStart(e); };
        };
        Board.prototype.teardownBoard = function () {
            var board = document.getElementById('board');
            for (var i = board.children.length - 1; i >= 0; i--) {
                board.removeChild(board.children[i]);
            }
            board.removeEventListener('onmousedown', this.onMouseDown);
            board.removeEventListener('ontouchstart', this.onTouchStart);
        };
        Board.prototype.onMouseDown = function (event) {
            if (!this.hasTouch) {
                var board = document.querySelector('#board');
                this.processMove(event.x, event.y);
            }
        };
        Board.prototype.onTouchStart = function (event) {
            this.hasTouch = true;
            for (var i = 0; i < event.targetTouches.length; i++) {
                this.processMove(event.targetTouches[i].clientX, event.targetTouches[i].clientY);
            }
        };
        Board.prototype.processMove = function (x, y) {
            var _this = this;
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
                        this.engine.isValidMove(function (result) {
                            if (result) {
                                _this.engine.makeMove(function () {
                                    _this.updateBoard(function () {
                                        _this.acceptingPlayerInput = false;
                                        _this.playerMovedCallback(move);
                                    });
                                }, function () { }, move);
                            }
                            else {
                                // cancel
                                _this.hasPickedStartSquare = false;
                            }
                        }, function () { }, move);
                    }
                    else {
                        var square = { row: r, col: c };
                        if (this.engine.isValidMoveStart(function (result) {
                            if (result) {
                                _this.hasPickedStartSquare = true;
                                _this.startRow = r;
                                _this.startCol = c;
                                _this.squares[r][c].classList.add('moveSource');
                            }
                        }, function () { }, square))
                            ;
                    }
                }
            }
        };
        Board.prototype.addPiece = function (pieceName, row, col) {
            var engine = this.engine;
            var pieces = this.pieces;
            var board = document.querySelector('#board');
            var piece = document.querySelector('#' + this.getPieceName(pieceName)).cloneNode(true);
            var x = .125 * this.adjustColForColor(col) * 100;
            var y = .125 * this.adjustRowForColor(row) * 100;
            piece.style.left = x + "%";
            piece.style.top = y + "%";
            piece.style.visibility = "visible";
            piece.classList.add('piece');
            board.insertBefore(piece, null);
            pieces[row][col] = piece;
        };
        Board.prototype.removePiece = function (row, col) {
            var pieces = this.pieces;
            if (pieces[row][col] != null) {
                var board = document.querySelector('#board');
                board.removeChild(pieces[row][col]);
                pieces[row][col] = null;
            }
        };
        Board.prototype.setPiece = function (r, c) {
            var _this = this;
            var square = { row: r, col: c };
            this.engine.getPiece(function (piece) {
                if (piece != '') {
                    _this.addPiece(piece, r, c);
                }
            }, function () { }, square);
        };
        Board.prototype.initializeBoard = function () {
            var r = 0;
            var c = 0;
            for (r = 0; r < 8; r++) {
                for (c = 0; c < 8; c++) {
                    this.setPiece(r, c);
                }
            }
        };
        Board.prototype.updatePiece = function (r, c, newPiece) {
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
        };
        Board.prototype.updateBoard = function (complete) {
            var _this = this;
            this.engine.getBoard(function (result) {
                var r = 0;
                var c = 0;
                for (r = 0; r < 8; r++) {
                    for (c = 0; c < 8; c++) {
                        _this.updatePiece(r, c, result[(7 - r) * 9 + c]);
                    }
                }
                complete();
            }, function () { });
        };
        return Board;
    })();
    MochaChessApp.Board = Board;
})(MochaChessApp || (MochaChessApp = {}));
var MochaChessApp;
(function (MochaChessApp) {
    "use strict";
    (function (Color) {
        Color[Color["White"] = 0] = "White";
        Color[Color["Black"] = 1] = "Black";
    })(MochaChessApp.Color || (MochaChessApp.Color = {}));
    var Color = MochaChessApp.Color;
})(MochaChessApp || (MochaChessApp = {}));
var MochaChessApp;
(function (MochaChessApp) {
    "use strict";
    var Position = (function () {
        function Position() {
        }
        return Position;
    })();
    MochaChessApp.Position = Position;
})(MochaChessApp || (MochaChessApp = {}));
var MochaChessApp;
(function (MochaChessApp) {
    "use strict";
    var ChessGame = (function () {
        function ChessGame(boardElement, playerColor, onGameOver) {
            var _this = this;
            this.onGameOver = onGameOver;
            this.board = new MochaChessApp.Board(boardElement);
            this.board.setupBoard(playerColor);
            this.board.engine.setStartPosition(function () { }, function () { });
            this.board.initializeBoard();
            if (playerColor == MochaChessApp.Color.Black) {
                this.doComputerMove();
            }
            else {
                this.board.waitForInputMove(function (move) { _this.onInputMove(move); });
            }
        }
        ChessGame.prototype.onInputMove = function (move) {
            this.doComputerMove();
        };
        ChessGame.prototype.doComputerMove = function () {
            var _this = this;
            this.computerMoving = true;
            setTimeout(function () {
                _this.board.engine.makeBestMove(function () {
                    _this.board.updateBoard(function () {
                        _this.computerMoving = false;
                        _this.board.waitForInputMove(function (move) { _this.onInputMove(move); });
                    });
                }, function () { });
            }, 0);
        };
        return ChessGame;
    })();
    MochaChessApp.ChessGame = ChessGame;
})(MochaChessApp || (MochaChessApp = {}));
/// <reference path="../plugins/com.sjelkjd.MochaChess/www/MochaChess.d.ts" />
/// <reference path="move.ts"/> 
/// <reference path="color.ts"/> 
/// <reference path="position.ts"/> 
/// <reference path="playComputer.ts"/> 
var MochaChessApp;
(function (MochaChessApp) {
    "use strict";
    var ChessProblemList = (function () {
        function ChessProblemList() {
            this.positions = [];
        }
        ChessProblemList.prototype.initialize = function (data) {
            for (var i = 0; i < data.length; i++) {
                var position = new MochaChessApp.Position();
                position.fen = data[i].fen;
                var moveValue = data[i].move;
                var move = new MochaChessApp.Move();
                move.startRow = parseInt(moveValue[0]);
                move.startCol = parseInt(moveValue[1]);
                move.endRow = parseInt(moveValue[2]);
                move.endCol = parseInt(moveValue[3]);
                position.move = move;
                this.positions.push(position);
            }
        };
        ChessProblemList.prototype.getProblem = function (index) {
            var fen = this.positions[index].fen;
            var problem = new ChessProblem(fen, fen[fen.indexOf(' ') + 1] == 'w' ? MochaChessApp.Color.White : MochaChessApp.Color.Black, this.positions[index].move);
            return problem;
        };
        return ChessProblemList;
    })();
    MochaChessApp.ChessProblemList = ChessProblemList;
    var ChessProblem = (function () {
        function ChessProblem(fen, color, expectedMove) {
            this.fen = fen;
            this.color = color;
            this.expectedMove = expectedMove;
        }
        return ChessProblem;
    })();
    MochaChessApp.ChessProblem = ChessProblem;
    var ChessProblemSolver = (function () {
        function ChessProblemSolver(problem, boardElement) {
            var _this = this;
            this.problem = problem;
            this.board = new MochaChessApp.Board(boardElement);
            this.board.setupBoard(this.problem.color);
            this.board.engine.setPosition(function () {
                _this.board.initializeBoard();
            }, function () { }, this.problem.fen);
        }
        ChessProblemSolver.prototype.resetBoard = function (complete) {
            var _this = this;
            this.board.engine.setPosition(function () {
                _this.board.updateBoard(function () {
                    complete();
                });
            }, function () { }, this.problem.fen);
        };
        ChessProblemSolver.prototype.waitForInput = function (successCallback, failureCallback) {
            var _this = this;
            this.successCallback = successCallback;
            this.failureCallback = failureCallback;
            this.board.waitForInputMove(function (move) { return _this.onInputMove(move); });
        };
        ChessProblemSolver.prototype.onInputMove = function (move) {
            if (this.problem.expectedMove.startRow == move.startRow &&
                this.problem.expectedMove.startCol == move.startCol &&
                this.problem.expectedMove.endRow == move.endRow &&
                this.problem.expectedMove.endCol == move.endCol) {
                this.successCallback();
            }
            else {
                this.failureCallback();
            }
        };
        return ChessProblemSolver;
    })();
    MochaChessApp.ChessProblemSolver = ChessProblemSolver;
})(MochaChessApp || (MochaChessApp = {}));
// For an introduction to the Blank template, see the following documentation:
// http://go.microsoft.com/fwlink/?LinkID=397705
// To debug code on page load in Ripple or on Android devices/emulators: launch your app, set breakpoints, 
// and then run "window.location.reload()" in the JavaScript Console.
/// <reference path="../plugins/com.sjelkjd.MochaChess/www/MochaChess.d.ts" />
/// <reference path="move.ts"/> 
/// <reference path="color.ts"/> 
/// <reference path="position.ts"/> 
/// <reference path="playComputer.ts"/> 
/// <reference path="solveProblem.ts"/> 
var MochaChessApp;
(function (MochaChessApp) {
    "use strict";
    var Application;
    (function (Application) {
        function initialize() {
            document.addEventListener('deviceready', onDeviceReady, false);
        }
        Application.initialize = initialize;
        function onDeviceReady() {
            document.addEventListener('pause', onPause, false);
            document.addEventListener('resume', onResume, false);
            document.addEventListener('touchmove', function (e) { e.preventDefault(); }, false);
        }
        function setupApplication() {
            var mochaChessApp = angular.module('mochaChessApp', ['ngRoute']);
            mochaChessApp.config(function ($routeProvider) {
                $routeProvider
                    .when('/', {
                    templateUrl: 'pages/home.html',
                    controller: 'mainController'
                })
                    .when('/playComputerOptions', {
                    templateUrl: 'pages/playComputerOptions.html',
                    controller: 'playComputerOptionsController'
                })
                    .when('/playComputer', {
                    templateUrl: 'pages/playComputer.html',
                    controller: 'playComputerController'
                })
                    .when('/solveProblem', {
                    templateUrl: 'pages/solveProblem.html',
                    controller: 'solveProblemController'
                });
            });
            mochaChessApp.factory('PlayerData', function () {
                return { color: 'white' };
            });
            mochaChessApp.controller('mainController', function ($scope) {
            });
            mochaChessApp.controller('playComputerOptionsController', function ($scope, PlayerData) {
                $scope.player = PlayerData;
            });
            mochaChessApp.controller('playComputerController', function ($scope, PlayerData) {
                $scope.player = PlayerData;
                $scope.$on('$includeContentLoaded', function () {
                    var game = new MochaChessApp.ChessGame(document.getElementById('board'), $scope.player.color == "white" ? MochaChessApp.Color.White : MochaChessApp.Color.Black, function () {
                        //on game over
                    });
                });
            });
            mochaChessApp.controller('solveProblemController', function ($scope, $http, $location) {
                var setupSolver = function () {
                    var problemIndex = window.localStorage.getItem('problemIndex');
                    if (problemIndex == null) {
                        problemIndex = 0;
                        window.localStorage.setItem('problemIndex', "0");
                    }
                    else {
                        problemIndex = parseInt(problemIndex);
                    }
                    Application.chessProblemSolver = new MochaChessApp.ChessProblemSolver(Application.problemList.getProblem(problemIndex), document.getElementById('board'));
                };
                var solveProblem = function () {
                    Application.chessProblemSolver.waitForInput(function () { $scope.showCorrect = true; $scope.showNext = true; $scope.$apply(); }, function () { $scope.showWrong = true; $scope.showNext = true; $scope.showAnswer = true; $scope.showTryAgain = true; $scope.$apply(); });
                };
                var contentLoaded = false, dataLoaded = false;
                $http.get('moves.json').then(function (response) {
                    dataLoaded = true;
                    Application.problemList = new MochaChessApp.ChessProblemList();
                    Application.problemList.initialize(response.data);
                    if (contentLoaded && dataLoaded) {
                        setupSolver();
                        solveProblem();
                    }
                });
                $scope.$on('$includeContentLoaded', function () {
                    contentLoaded = true;
                    if (contentLoaded && dataLoaded) {
                        setupSolver();
                        solveProblem();
                    }
                });
                $scope.clickTryAgain = function () {
                    $scope.showCorrect = false;
                    $scope.showWrong = false;
                    $scope.showNext = false;
                    $scope.showAnswer = false;
                    $scope.showTryAgain = false;
                    Application.chessProblemSolver.resetBoard(function () {
                        solveProblem();
                    });
                };
                $scope.clickShowAnswer = function () {
                    Application.chessProblemSolver.resetBoard(function () {
                        Application.chessProblemSolver.board.engine.makeMove(function () {
                            Application.chessProblemSolver.board.updateBoard(function () {
                            });
                        }, function () { }, Application.chessProblemSolver.problem.expectedMove);
                    });
                };
                $scope.clickNext = function () {
                    Application.chessProblemSolver.board.teardownBoard();
                    var problemIndex = parseInt(window.localStorage.getItem('problemIndex'));
                    problemIndex++;
                    window.localStorage.setItem('problemIndex', problemIndex.toString());
                    $scope.showCorrect = false;
                    $scope.showWrong = false;
                    $scope.showNext = false;
                    $scope.showAnswer = false;
                    $scope.showTryAgain = false;
                    setupSolver();
                    solveProblem();
                };
                $scope.clickQuit = function () {
                    $location.path('/');
                };
            });
        }
        Application.setupApplication = setupApplication;
        function onPause() {
            // TODO: This application has been suspended. Save application state here.
        }
        function onResume() {
            // TODO: This application has been reactivated. Restore application state here.
        }
    })(Application = MochaChessApp.Application || (MochaChessApp.Application = {}));
    window.onload = function () {
        Application.initialize();
    };
})(MochaChessApp || (MochaChessApp = {}));
MochaChessApp.Application.setupApplication();
//# sourceMappingURL=appBundle.js.map