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
        surfaceTexture = texture;
        mediaPlayer = MediaPlayer.create(this, R.raw.sample);
        Surface surface = new Surface(texture);
        mediaPlayer.setSurface(surface);
        surface.release();

        mediaPlayer.setLooping(true);
        mediaPlayer.start();
    }

    @Override
    protected void onPause() {
        super.onPause();
        if (mediaPlayer != null && mediaPlayer.isPlaying()) {
            mediaPlayer.pause();   // Tạm dừng khi ẩn app
        }
        if (glVideoView != null) {
            glVideoView.stopVideoRenderer();
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        if (mediaPlayer != null) {
            mediaPlayer.setSurface(null);  // Ngắt kết nối surface trước
            mediaPlayer.stop();
            mediaPlayer.reset();
            mediaPlayer.release();
            mediaPlayer = null;
        }

        if (glVideoView != null) {
            glVideoView.stopVideoRenderer();  // Dừng native renderer
        }

        if (surfaceTexture != null) {
            surfaceTexture.release();          // Giải phóng SurfaceTexture sau cùng
            surfaceTexture = null;
        }
    }
}