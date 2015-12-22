var engine = new ChessEngineUWP.ChessEngine();
module.exports = {
	getPiece: function(successCallback,errorCallback,input) {
		if(input) {
			var res = engine.getPiece(input[0].row, input[0].col);
			successCallback(res);
	        }
        	else {
			errorCallback(input);
        	}	
	},

	makeBestMove: function(successCallback,errorCallback,input) {
		if(input) {
			engine.makeComputerMove();
			successCallback();
	        }
        	else {
			errorCallback(input);
        	}	
	},

	makeMove: function(successCallback,errorCallback,input) {
		if(input) {
			engine.makeMove(input[0].startRow, input[0].startCol, input[0].endRow, input[0].endCol);
			successCallback();
	        }
        	else {
			errorCallback(input);
        	}	
	},

	isValidMoveStart: function(successCallback,errorCallback,input) {
		if(input) {
			var res = engine.isValidMoveStart(input[0].row, input[0].col);
			successCallback(res);
	        }
        	else {
			errorCallback(input);
        	}	
	},

	isValidMove: function(successCallback,errorCallback,input) {
		if(input) {
			var res = engine.isValidMove(input[0].startRow, input[0].startCol, input[0].endRow, input[0].endCol);
			successCallback(res);
	        }
        	else {
			errorCallback(input);
        	}	
	},

	initializeBoard: function(successCallback,errorCallback,input) {
		if(input) {
			engine.initializeBoard();
			successCallback();
	    }
	}
};

require("cordova/exec/proxy").add("MochaChess", module.exports);