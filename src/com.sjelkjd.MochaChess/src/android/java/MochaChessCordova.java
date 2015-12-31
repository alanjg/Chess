package org.sjelkjd.MochaChess;

import android.os.Handler;
import android.os.HandlerThread;
import android.util.Log;
import android.os.Looper;
import android.os.Message;

import org.apache.cordova.CallbackContext;
import org.apache.cordova.CordovaPlugin;
import org.apache.cordova.PluginResult;
import org.apache.cordova.CordovaInterface;
import org.apache.cordova.CordovaWebView;

import org.json.JSONArray;
import org.json.JSONObject;
import org.json.JSONException;

import java.util.Timer;
import java.util.TimerTask;
import java.util.HashMap;
import java.util.List;
import java.util.ArrayList;

public class MochaChessCordova extends CordovaPlugin
{
    static
    {
        System.loadLibrary("MochaChessLib");
    }

	public static native String GetPiece(int row, int col);
	public static native void MakeBestMove();
	public static native void MakeMove(int startRow, int startCol, int endRow, int endCol);
	public static native boolean IsValidMoveStart(int row, int col);
	public static native boolean IsValidMove(int startRow, int startCol, int endRow, int endCol);
	public static native void InitializeBoard();
	
    @Override
    public void initialize(final CordovaInterface cordova, CordovaWebView webView)
    {
        super.initialize(cordova, webView);
    }

    @Override
    public boolean execute(String action, final JSONArray data, final CallbackContext callbackContext) throws JSONException
    {
        if (action.equals("getPiece"))
        {
			JSONObject square = data.getJSONObject(0);
            int row = square.getInt("row");
            int col = square.getInt("col");

			String result = GetPiece(row, col);
			callbackContext.success(result);
            return true;
        }
        if (action.equals("makeBestMove"))
        {
			MakeBestMove();
            callbackContext.success();
            return true;
        }
        else if (action.equals("makeMove"))
        {
			JSONObject move = data.getJSONObject(0);
            int sr = move.getInt("startRow");
            int sc = move.getInt("startCol");
			int er = move.getInt("endRow");
            int ec = move.getInt("endCol");
			MakeMove(sr, sc, er, ec);
     	    callbackContext.success();
            return true;
        }
        else if (action.equals("isValidMoveStart"))
        {
			JSONObject square = data.getJSONObject(0);
            int row = square.getInt("row");
            int col = square.getInt("col");
			boolean result = IsValidMoveStart(row, col);
			callbackContext.sendPluginResult(new PluginResult(PluginResult.Status.OK, result));
			return true;
        }
        else if (action.equals("isValidMove"))
        {
			JSONObject move = data.getJSONObject(0);
            int sr = move.getInt("startRow");
            int sc = move.getInt("startCol");
			int er = move.getInt("endRow");
            int ec = move.getInt("endCol");
     	    boolean result = IsValidMove(sr, sc, er, ec);
			callbackContext.sendPluginResult(new PluginResult(PluginResult.Status.OK, result));
        	return true;
        }
        else if (action.equals("initializeBoard"))
        {
			InitializeBoard();
     		callbackContext.success();
            return true;
        }
          
        return false;
    }
}
