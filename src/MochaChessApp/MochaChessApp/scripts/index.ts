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
module MochaChessApp {
    "use strict"; 

    export module Application {
        export var problemList: ChessProblemList;
        export var chessProblemSolver: ChessProblemSolver;
        export function initialize() {
            document.addEventListener('deviceready', onDeviceReady, false);
        }
        
        function onDeviceReady() {
            document.addEventListener('pause', onPause, false);
            document.addEventListener('resume', onResume, false);        
            document.addEventListener('touchmove', function (e) { e.preventDefault(); }, false);


        }

        export function setupApplication() {

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
            })

            mochaChessApp.controller('mainController', function ($scope) {

            });

            mochaChessApp.controller('playComputerOptionsController', function ($scope, PlayerData) {
                $scope.player = PlayerData;
            });


            mochaChessApp.controller('playComputerController', function ($scope, PlayerData) {
                $scope.player = PlayerData;
                $scope.$on('$includeContentLoaded', function () {
                    var game = new ChessGame(document.getElementById('board'), $scope.player.color == "white" ? Color.White : Color.Black, () => {
                        //on game over
                    });

                });
            });

            mochaChessApp.controller('solveProblemController', function ($scope, $http, $location) {
                var setupSolver = function () {
                    var problemIndexStr = window.localStorage.getItem('problemIndex');
                    var problemIndex: number;
                    if (problemIndexStr == null) {
                        problemIndex = 0;
                        window.localStorage.setItem('problemIndex', "0");
                    }
                    else {
                        problemIndex = parseInt(problemIndexStr);
                    }
                    Application.chessProblemSolver = new ChessProblemSolver(Application.problemList.getProblem(problemIndex), document.getElementById('board'));
                };

                var solveProblem = function () {
                    Application.chessProblemSolver.waitForInput(
                        () => { $scope.showCorrect = true; $scope.showNext = true; $scope.$apply(); },
                        () => { $scope.showWrong = true; $scope.showNext = true; $scope.showAnswer = true; $scope.showTryAgain = true; $scope.$apply(); });
                };

                var contentLoaded = false, dataLoaded = false;
                $http.get('moves.json').then(function (response) {
                    dataLoaded = true;

                    Application.problemList = new ChessProblemList();
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
                    Application.chessProblemSolver.resetBoard(() => {
                        solveProblem();
                    });
                    
                };

                $scope.clickShowAnswer = function () {
                    Application.chessProblemSolver.resetBoard(() => {
                        Application.chessProblemSolver.board.engine.makeMove(() => {
                            Application.chessProblemSolver.board.updateBoard(() => {
                            });
                        }, () => { }, Application.chessProblemSolver.problem.expectedMove);
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

        function onPause() {
            // TODO: This application has been suspended. Save application state here.
        }

        function onResume() {
            // TODO: This application has been reactivated. Restore application state here.
        }

    }

    window.onload = function () {
        Application.initialize();
    }
}

MochaChessApp.Application.setupApplication();