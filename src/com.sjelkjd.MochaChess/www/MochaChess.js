var MochaChess = {};

MochaChess.getPiece = function(input, successCallback, errorCallback) {
	cordova.exec(successCallback, errorCallback, "MochaChess", "getPiece", [input]);
}

MochaChess.makeBestMove = function(input, successCallback, errorCallback) {
	cordova.exec(successCallback, errorCallback, "MochaChess", "makeBestMove", [input]);
}

MochaChess.makeMove = function(input, successCallback, errorCallback) {
	cordova.exec(successCallback, errorCallback, "MochaChess", "makeMove", [input]);
}

MochaChess.isValidMoveStart = function(input, successCallback, errorCallback) {
	cordova.exec(successCallback, errorCallback, "MochaChess", "isValidMoveStart", [input]);
}

MochaChess.isValidMove = function(input, successCallback, errorCallback) {
	cordova.exec(successCallback, errorCallback, "MochaChess", "isValidMove", [input]);
}

MochaChess.initializeBoard = function(input, successCallback, errorCallback) {
	cordova.exec(successCallback, errorCallback, "MochaChess", "initializeBoard", [input]);
}

module.exports = MochaChess;