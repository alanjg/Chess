var MochaChess = {};

MochaChess.getPiece = function(successCallback, errorCallback, input) {
	cordova.exec(successCallback, errorCallback, "MochaChess", "getPiece", [input]);
}

MochaChess.makeBestMove = function(successCallback, errorCallback, input) {
	cordova.exec(successCallback, errorCallback, "MochaChess", "makeBestMove", [input]);
}

MochaChess.makeMove = function(successCallback, errorCallback, input) {
	cordova.exec(successCallback, errorCallback, "MochaChess", "makeMove", [input]);
}

MochaChess.isValidMoveStart = function(successCallback, errorCallback, input) {
	cordova.exec(successCallback, errorCallback, "MochaChess", "isValidMoveStart", [input]);
}

MochaChess.isValidMove = function(successCallback, errorCallback, input) {
	cordova.exec(successCallback, errorCallback, "MochaChess", "isValidMove", [input]);
}

MochaChess.setStartPosition = function(successCallback, errorCallback, input) {
	cordova.exec(successCallback, errorCallback, "MochaChess", "setStartPosition", [input]);
}

MochaChess.setPosition = function(successCallback, errorCallback, input) {
	cordova.exec(successCallback, errorCallback, "MochaChess", "setPosition", [input]);
}
module.exports = MochaChess;