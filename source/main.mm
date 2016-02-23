#include <string.h>
#include <3ds.h>

#import <Latte/Canvas.h>
#include <GL/gl.h>
#include <gfx_device.h>
#include <cstdio>
#include "Model.h"

#define CONFIG_3D_SLIDERSTATE (*(float*)0x1FF81080)

static void blitCanvas(Canvas *can, u8 *fb) {
  [can flushTo:fb withFormat:GX_TRANSFER_FMT_RGB8];
}

#include <cmath>

@interface OpenGLView : Canvas {
  GLuint right;
}

- (void)onDraw;
- (void)draw;

@end

struct Entity {
  float posX;
  float posY;
  float posZ;

  float rotX;
  float rotY;
  float rotZ;

  Model *model = nullptr;
};

struct Camera : Entity {};

static Model mod;
static Entity ent;
static bool left = true;

@implementation OpenGLView

- (void)onDraw {
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  float near = 0.1f;
  float far = 150.0f;
  float fov = 90.0f;
  float aspect = (float)_width / (float)_height;
  float t = tan(fov * 3.14159 / 360.0) * near;
  float b = -t;
  float l = aspect * b;
  float r = aspect * t;
  glFrustum(l, r, b, t, near, far);
  glRotatef(-90.0f, 0.0f, 0.0f, 1.0f);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glEnable(GL_DEPTH_TEST);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();
  float slider = CONFIG_3D_SLIDERSTATE;
  float interaxial=(6.5 * slider)/(float)_height;
  if (left) {
    glTranslatef(-interaxial, 0.0f, 0.0f);
  } else {
    glTranslatef(interaxial, 0.0f, 0.0f);
  }
  // apply camera
  glPushMatrix();
  glTranslatef(ent.posX, ent.posY, ent.posZ);
  glRotatef(ent.rotX, 1, 0, 0);
  glRotatef(ent.rotY, 0, 1, 0);
  glRotatef(ent.rotZ, 0, 0, 1);
  if (ent.model)
    render_model(*ent.model);
  glPopMatrix();

  glDisable(GL_DEPTH_TEST);

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
}

- (void)draw {
  void *saved = gfxMakeCurrent(_context);
  [self onDraw];
  gfxMakeCurrent(saved);
}

@end

int main() {
  gfxInitDefault();
  romfsInit();
  consoleInit(GFX_BOTTOM, nullptr);
  gfxSet3D(true);
  void *dummyContext = gfxCreateDevice(32, 32, CAELINA_SHARED_TEXTURES);
  gfxMakeCurrent(dummyContext);
  model_load_obj("rover_mesh.obj", mod);

  ent.model = &mod;
  ent.posZ = -5.0f;
  OpenGLView *can =
      [[[OpenGLView new] setPaint:[Paint purple]] setWidth:240 andHeight:400];

  while (aptMainLoop()) {
    gspWaitForVBlank();
    hidScanInput();

    u32 kDown = hidKeysHeld();
    if (kDown & KEY_START)
      break; // break in order to return to hbmenu

    if (kDown & KEY_DOWN) {
      ent.posZ -= 0.1f;
    }
    if (kDown & KEY_UP) {
      ent.posZ += 0.1f;
    }
    if (kDown & KEY_LEFT) {
      ent.rotY -= 2.5;
    }
    if (kDown & KEY_RIGHT) {
      ent.rotY += 2.5;
    }

    if (kDown & KEY_R) {
      ent.posY += 0.1f;
    }

    if (kDown & KEY_L) {
      ent.posY -= 0.1f;
    }

    u8 *fb = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
    left = true;
    Transform *trans = [can transform];
    [trans setIdentity];
    [can setPaint:[Paint black]];
    [can clear];
    [[can paint] set:0xFFFFFFFF];
    [trans setRotationX:0 andY:0 andZ:90.0];
    [can draw];
    [can drawString:"model viewer by machinamentum" atX:(20.0f) andY:(200.0)];

    blitCanvas(can, fb);


    fb = gfxGetFramebuffer(GFX_TOP, GFX_RIGHT, NULL, NULL);
    left = false;
    [trans setIdentity];
    [can setPaint:[Paint black]];
    [can clear];
    [[can paint] set:0xFFFFFFFF];
    [trans setRotationX:0 andY:0 andZ:90.0];
    [can draw];
    [can drawString:"model viewer by machinamentum" atX:(20.0f) andY:(200.0)];

    blitCanvas(can, fb);

    printf("\x1b[0;0HFree linear: %lu\n", linearSpaceFree());
    printf("Free VRAM: %lu\n", vramSpaceFree());
    // Flush and swap framebuffers
    gfxFlushBuffers();
    gfxSwapBuffersGpu();
  }
  gfxDestroyDevice(dummyContext);
  romfsExit();
  gfxExit();
  return 0;
}
