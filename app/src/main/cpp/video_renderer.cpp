//
// Created by ADMIN on 5/25/2025.
//
#include "video_renderer.h"
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <thread>
#include <chrono>
#include <atomic>
#include <android/log.h>
#include <jni.h>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "VideoRenderer", __VA_ARGS__)

static std::atomic<bool> gRunning(false);
static std::thread gRenderThread;
static ANativeWindow* gWindow = nullptr;

static const char *vertexShaderSrc = R"(
attribute vec4 aPosition;
attribute vec2 aTexCoord;
varying vec2 vTexCoord;
void main() {
    gl_Position = aPosition;
    vTexCoord = aTexCoord;
}
)";

static const char *fragmentShaderSrc = R"(
#extension GL_OES_EGL_image_external : require
precision mediump float;
uniform samplerExternalOES uTexture;
varying vec2 vTexCoord;
void main() {
    gl_FragColor = texture2D(uTexture, vTexCoord);
}
)";

static const GLfloat vertices[] = {
        -1.0f, -0.5625f, 0.0f, 1.0f,
        1.0f, -0.5625f, 1.0f, 1.0f,
        -1.0f,  0.5625f, 0.0f, 0.0f,
        1.0f,  0.5625f, 1.0f, 0.0f,
};

GLuint loadShader(GLenum type, const char *src) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);
    GLint compiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        char log[512];
        glGetShaderInfoLog(shader, 512, nullptr, log);
        LOGI("Shader compile failed: %s", log);
        return 0;
    }
    return shader;
}

GLuint createProgram() {
    GLuint vs = loadShader(GL_VERTEX_SHADER, vertexShaderSrc);
    GLuint fs = loadShader(GL_FRAGMENT_SHADER, fragmentShaderSrc);
    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    GLint linked;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked) {
        char log[512];
        glGetProgramInfoLog(program, 512, nullptr, log);
        LOGI("Program link failed: %s", log);
        return 0;
    }
    return program;
}

void stopVideoRenderer() {
    LOGI("Stopping renderer thread...");
    if (!gRunning) return;

    gRunning = false;

    if (gRenderThread.joinable()) {
        gRenderThread.join();
        LOGI("Renderer thread joined.");
    }

    if (gWindow) {
        ANativeWindow_release(gWindow);
        gWindow = nullptr;
        LOGI("ANativeWindow released.");
    }
}

void startVideoRenderer(ANativeWindow *window, GLuint textureId, JavaVM* vm, jobject surfaceTexture, jmethodID updateTexImageMethod) {
    if (gRunning) {
        LOGI("Renderer already running");
        return;
    }

    gRunning = true;

    if (gWindow != nullptr) {
        ANativeWindow_release(gWindow);
    }
    gWindow = window;

    gRenderThread = std::thread([window, textureId, vm, surfaceTexture, updateTexImageMethod]() {
        JNIEnv* env;
        if (vm->AttachCurrentThread(&env, nullptr) != JNI_OK) {
            LOGI("Failed to attach thread to JVM");
            return;
        }

        EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        eglInitialize(display, nullptr, nullptr);

        const EGLint configAttribs[] = {
                EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
                EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8,
                EGL_NONE
        };
        EGLConfig config;
        EGLint numConfigs;
        eglChooseConfig(display, configAttribs, &config, 1, &numConfigs);

        const EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
        EGLContext context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribs);

        EGLSurface surface = eglCreateWindowSurface(display, config, window, nullptr);
        eglMakeCurrent(display, surface, surface, context);

        GLuint program = createProgram();
        glUseProgram(program);

        GLint aPosition = glGetAttribLocation(program, "aPosition");
        GLint aTexCoord = glGetAttribLocation(program, "aTexCoord");
        GLint uTexture = glGetUniformLocation(program, "uTexture");

        glViewport(0, 0, ANativeWindow_getWidth(window), ANativeWindow_getHeight(window));
        glClearColor(0, 0, 0, 1);

        while (gRunning) {
            // Gọi updateTexImage() trên SurfaceTexture mỗi frame
            env->CallVoidMethod(surfaceTexture, updateTexImageMethod);

            glClear(GL_COLOR_BUFFER_BIT);

            glUseProgram(program);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_EXTERNAL_OES, textureId);
            glUniform1i(uTexture, 0);

            glEnableVertexAttribArray(aPosition);
            glEnableVertexAttribArray(aTexCoord);
            glVertexAttribPointer(aPosition, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), vertices);
            glVertexAttribPointer(aTexCoord, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), vertices + 2);

            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

            eglSwapBuffers(display, surface);

            std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60fps
        }

        eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglDestroySurface(display, surface);
        eglDestroyContext(display, context);
        eglTerminate(display);

        vm->DetachCurrentThread();

        LOGI("Renderer thread exited cleanly.");
    });
}
