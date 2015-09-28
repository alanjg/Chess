module.exports = {
	getPiece: function(successCallback,errorCallback,input) {
		if(input) {
			var res = MochaChessPluginWindows.MochaChessPlugin.getPiece(input[0].row, input[0].col);
			successCallback(res);
	        }
        	else {
			errorCallback(input);
        	}	
	},


	makeBestMove: function(successCallback,errorCallback,input) {
		if(input) {
			var res = MochaChessPluginWindows.MochaChessPlugin.makeBestMove();
			successCallback(res);
	        }
        	else {
			errorCallback(input);
        	}	
	},


	makeMove: function(successCallback,errorCallback,input) {
		if(input) {
			var res = MochaChessPluginWindows.MochaChessPlugin.makeMove(input[0].r1, input[0].c1, input[0].r2, input[0].c2);
			successCallback(res);
	        }
        	else {
			errorCallback(input);
        	}	
	},



	isValidMoveStart: function(successCallback,errorCallback,input) {
		if(input) {
			var res = MochaChessPluginWindows.MochaChessPlugin.isValidMoveStart(input[0].row, input[0].col);
			successCallback(res);
	        }
        	else {
			errorCallback(input);
        	}	
	},



	isValidMove: function(successCallback,errorCallback,input) {
		if(input) {
			var res = MochaChessPluginWindows.MochaChessPlugin.isValidMove(input[0].r1, input[0].c1, input[0].r2, input[0].c2);
			successCallback(res);
	        }
        	else {
			errorCallback(input);
        	}	
	},



	initializeBoard: function(successCallback,errorCallback,input) {
		if(input) {
			var res = MochaChessPluginWindows.MochaChessPlugin.getPiece(input[0].row, input[0].col);
			successCallback(res);
	        }
        	else {
			errorCallback(input);
        	}	
	}
};

require("cordova/exec/proxy").add("MochaChess", module.exports);