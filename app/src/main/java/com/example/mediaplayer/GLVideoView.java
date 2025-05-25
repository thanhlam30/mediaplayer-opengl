package com.example.mediaplayer;

import android.content.Context;
import android.graphics.SurfaceTexture;
import android.opengl.GLES11Ext;
import android.opengl.GLES20;
import android.util.AttributeSet;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import androidx.annotation.NonNull;

public class GLVideoView extends SurfaceView implements SurfaceHolder.Callback {
    private int textureId;


    public GLVideoView(Context context, AttributeSet attrs) {
        super(context, attrs);
        getHolder().addCallback(this);
    }
    @Override
    public void surfaceCreated(@NonNull SurfaceHolder holder) {
        textureId = createOESTexture();
        SurfaceTexture st = new SurfaceTexture(textureId);
        ((MainActivity) getContext()).setupVideoPlayer(st);
        nativeInit(getHolder().getSurface(), textureId, st);
    }

    private int createOESTexture() {
        int[] tex = new int[1];
        GLES20.glGenTextures(1, tex, 0);
        GLES20.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, tex[0]);
        return tex[0];
    }

    @Override
    public void surfaceChanged(@NonNull SurfaceHolder holder, int format, int width, int height) {

    }

    @Override
    public void surfaceDestroyed(@NonNull SurfaceHolder holder) {

    }

    private native void nativeInit(Surface surface, int textureId, SurfaceTexture surfaceTexture);
}
