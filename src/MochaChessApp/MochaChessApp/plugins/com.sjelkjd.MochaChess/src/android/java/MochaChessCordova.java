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
    /* Load the native library. */
    static
    {
        System.loadLibrary("sjelkjd");
    }


    /**
     * Sets the context of the Command. This can then be used to do things like
     * get file paths associated with the Activity.
     *
     * @param cordova The context of the main Activity.
     * @param webView The CordovaWebView Cordova is running in.
     */
    @Override
    public void initialize(final CordovaInterface cordova, CordovaWebView webView)
    {
        super.initialize(cordova, webView);
    }

    /**
     * Executes the request and returns PluginResult.
     *
     * @param action            The action to execute.
     * @param args              JSONArray of arguments for the plugin.
     * @param callbackContext   The callback context used when calling back into JavaScript.
     * @return                  True when the action was valid, false otherwise.
     */
    @Override
    public boolean execute(String action, final JSONArray data, final CallbackContext callbackContext) throws JSONException
    {
        if (action.equals("getPiece"))
        {
            int row = data.getInt(0);
            int col = data.getInt(1);

            return true;
        }
        if (action.equals("makeBestMove"))
        {
            callbackContext.success("Disconnected");
            return true;
        }
        else if (action.equals("makeMove"))
        {
     	    int sr = data.getInt(0);
            int sc = data.getInt(1);
		int er = data.getInt(2);
            int ec = data.getInt(3);

            return true;
        }
        else if (action.equals("isValidMoveStart"))
        {
        }
        else if (action.equals("isValidMove"))
        {
                 return true;
        }
        else if (action.equals("initializeBoard"))
        {
     
            return true;
        }
          
        return false;
    }
}
