var engine = new ChessEngineUWP.ChessEngine();
module.exports = {
	getPiece: function(successCallback,errorCallback,input) {
		if(input) {
			var res = engine.getPiece(input[0].row, input[0].col);
			successCallback(res);
	        }
        	else {
			errorCallback();
        	}	
	},

	makeBestMove: function(successCallback,errorCallback,input) {
		if(input) {
			engine.makeComputerMove();
			successCallback();
	        }
        	else {
			errorCallback();
        	}	
	},

	makeMove: function(successCallback,errorCallback,input) {
		if(input) {
			engine.makeMove(input[0].startRow, input[0].startCol, input[0].endRow, input[0].endCol);
			successCallback();
	        }
        	else {
			errorCallback();
        	}	
	},

	isValidMoveStart: function(successCallback,errorCallback,input) {
		if(input) {
			var res = engine.isValidMoveStart(input[0].row, input[0].col);
			successCallback(res);
	        }
        	else {
			errorCallback();
        	}	
	},

	isValidMove: function(successCallback,errorCallback,input) {
		if(input) {
			var res = engine.isValidMove(input[0].startRow, input[0].startCol, input[0].endRow, input[0].endCol);
			successCallback(res);
	        }
        	else {
			errorCallback();
        	}	
	},

	setStartPosition: function(successCallback,errorCallback,input) {
		if(input) {
			engine.setStartPosition();
			successCallback();
	    }
	},
	
	setPosition: function(successCallback,errorCallback,input) {
		if(input) {
			engine.setPosition(input[0]);
			successCallback();
	    }
	}
};

require("cordova/exec/proxy").add("MochaChess", module.exports);