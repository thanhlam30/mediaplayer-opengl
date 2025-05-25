//
// Created by ADMIN on 5/25/2025.
//
#include <jni.h>
#include <android/native_window_jni.h>
#include "video_renderer.h"
#include <thread>
#include <chrono>
#include <android/log.h>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "VideoRenderer", __VA_ARGS__)

static JavaVM* g_vm = nullptr;
static jobject g_surfaceTexture = nullptr;
static jmethodID g_updateTexImageMethod = nullptr;

extern "C"
JNIEXPORT void JNICALL
Java_com_example_mediaplayer_GLVideoView_nativeInit(JNIEnv *env, jobject thiz, jobject surface,
                                                    jint texture_id, jobject surfaceTexture) {
    if (g_vm == nullptr) {
        env->GetJavaVM(&g_vm);
    }

    if (g_surfaceTexture != nullptr) {
        env->DeleteGlobalRef(g_surfaceTexture);
        g_surfaceTexture = nullptr;
    }

    g_surfaceTexture = env->NewGlobalRef(surfaceTexture);

    jclass cls = env->GetObjectClass(surfaceTexture);
    g_updateTexImageMethod = env->GetMethodID(cls, "updateTexImage", "()V");

    ANativeWindow *window = ANativeWindow_fromSurface(env, surface);

    // Thay đổi hàm startVideoRenderer để nhận thêm JNIEnv*, jobject surfaceTexture, methodID
    startVideoRenderer(window, static_cast<GLuint>(texture_id), g_vm, g_surfaceTexture,
                       g_updateTexImageMethod);
}
