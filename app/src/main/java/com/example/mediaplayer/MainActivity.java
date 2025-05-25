package com.example.mediaplayer;

import android.graphics.SurfaceTexture;
import android.media.MediaPlayer;
import android.os.Bundle;
import android.view.Surface;

import androidx.appcompat.app.AppCompatActivity;

public class MainActivity extends AppCompatActivity {
    private MediaPlayer mediaPlayer;
    private SurfaceTexture surfaceTexture;
    private GLVideoView glVideoView;

    static {
        System.loadLibrary("mediaplayer");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        glVideoView = findViewById(R.id.gl_video_view);
    }

    public void setupVideoPlayer(SurfaceTexture texture) {
        mediaPlayer = MediaPlayer.create(this, R.raw.sample);
        Surface surface = new Surface(texture);
        mediaPlayer.setSurface(surface);
        surface.release();

        mediaPlayer.setLooping(true);
        mediaPlayer.start();
    }
}