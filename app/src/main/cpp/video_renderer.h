//
// Created by ADMIN on 5/25/2025.
//

#ifndef MEDIAPLAYER_VIDEO_RENDERER_H
#define MEDIAPLAYER_VIDEO_RENDERER_H
#include <android/native_window.h>
#include <GLES2/gl2.h>
#include <jni.h>

void startVideoRenderer(ANativeWindow *window, GLuint textureId, _JavaVM *pVm, _jobject *pJobject,
                        _jmethodID *pId);

void stopVideoRenderer();
#endif //MEDIAPLAYER_VIDEO_RENDERER_H
