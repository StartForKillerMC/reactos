#include "virtio_gpu_icd.h"
#include "opengl.h"
#include "../../displays/virtio_gpu_disp/virtio_gpu_esc.h"
#include <stdio.h>

static ULONG ContextID = 0;

#define GL_UNIMPLEMENTED_FUNC(name) \
    static PVOID unimp_##name() \
    { \
        DPRINT("Unimplemented gl function: "#name);\
        return NULL; \
    }

GL_UNIMPLEMENTED_FUNC(glNewList);
GL_UNIMPLEMENTED_FUNC(glEndList);
GL_UNIMPLEMENTED_FUNC(glCallList);
GL_UNIMPLEMENTED_FUNC(glCallLists);
GL_UNIMPLEMENTED_FUNC(glDeleteLists);
GL_UNIMPLEMENTED_FUNC(glGenLists);
GL_UNIMPLEMENTED_FUNC(glListBase);
GL_UNIMPLEMENTED_FUNC(glBegin);
GL_UNIMPLEMENTED_FUNC(glBitmap);
GL_UNIMPLEMENTED_FUNC(glColor3b);
GL_UNIMPLEMENTED_FUNC(glColor3bv);
GL_UNIMPLEMENTED_FUNC(glColor3d);
GL_UNIMPLEMENTED_FUNC(glColor3dv);
GL_UNIMPLEMENTED_FUNC(glColor3f);
GL_UNIMPLEMENTED_FUNC(glColor3fv);
GL_UNIMPLEMENTED_FUNC(glColor3i);
GL_UNIMPLEMENTED_FUNC(glColor3iv);
GL_UNIMPLEMENTED_FUNC(glColor3s);
GL_UNIMPLEMENTED_FUNC(glColor3sv);
GL_UNIMPLEMENTED_FUNC(glColor3ub);
GL_UNIMPLEMENTED_FUNC(glColor3ubv);
GL_UNIMPLEMENTED_FUNC(glColor3ui);
GL_UNIMPLEMENTED_FUNC(glColor3uiv);
GL_UNIMPLEMENTED_FUNC(glColor3us);
GL_UNIMPLEMENTED_FUNC(glColor3usv);
GL_UNIMPLEMENTED_FUNC(glColor4b);
GL_UNIMPLEMENTED_FUNC(glColor4bv);
GL_UNIMPLEMENTED_FUNC(glColor4d);
GL_UNIMPLEMENTED_FUNC(glColor4dv);
GL_UNIMPLEMENTED_FUNC(glColor4f);
GL_UNIMPLEMENTED_FUNC(glColor4fv);
GL_UNIMPLEMENTED_FUNC(glColor4i);
GL_UNIMPLEMENTED_FUNC(glColor4iv);
GL_UNIMPLEMENTED_FUNC(glColor4s);
GL_UNIMPLEMENTED_FUNC(glColor4sv);
GL_UNIMPLEMENTED_FUNC(glColor4ub);
GL_UNIMPLEMENTED_FUNC(glColor4ubv);
GL_UNIMPLEMENTED_FUNC(glColor4ui);
GL_UNIMPLEMENTED_FUNC(glColor4uiv);
GL_UNIMPLEMENTED_FUNC(glColor4us);
GL_UNIMPLEMENTED_FUNC(glColor4usv);
GL_UNIMPLEMENTED_FUNC(glEdgeFlag);
GL_UNIMPLEMENTED_FUNC(glEdgeFlagv);
GL_UNIMPLEMENTED_FUNC(glEnd);
GL_UNIMPLEMENTED_FUNC(glIndexd);
GL_UNIMPLEMENTED_FUNC(glIndexdv);
GL_UNIMPLEMENTED_FUNC(glIndexf);
GL_UNIMPLEMENTED_FUNC(glIndexfv);
GL_UNIMPLEMENTED_FUNC(glIndexi);
GL_UNIMPLEMENTED_FUNC(glIndexiv);
GL_UNIMPLEMENTED_FUNC(glIndexs);
GL_UNIMPLEMENTED_FUNC(glIndexsv);
GL_UNIMPLEMENTED_FUNC(glNormal3b);
GL_UNIMPLEMENTED_FUNC(glNormal3bv);
GL_UNIMPLEMENTED_FUNC(glNormal3d);
GL_UNIMPLEMENTED_FUNC(glNormal3dv);
GL_UNIMPLEMENTED_FUNC(glNormal3f);
GL_UNIMPLEMENTED_FUNC(glNormal3fv);
GL_UNIMPLEMENTED_FUNC(glNormal3i);
GL_UNIMPLEMENTED_FUNC(glNormal3iv);
GL_UNIMPLEMENTED_FUNC(glNormal3s);
GL_UNIMPLEMENTED_FUNC(glNormal3sv);
GL_UNIMPLEMENTED_FUNC(glRasterPos2d);
GL_UNIMPLEMENTED_FUNC(glRasterPos2dv);
GL_UNIMPLEMENTED_FUNC(glRasterPos2f);
GL_UNIMPLEMENTED_FUNC(glRasterPos2fv);
GL_UNIMPLEMENTED_FUNC(glRasterPos2i);
GL_UNIMPLEMENTED_FUNC(glRasterPos2iv);
GL_UNIMPLEMENTED_FUNC(glRasterPos2s);
GL_UNIMPLEMENTED_FUNC(glRasterPos2sv);
GL_UNIMPLEMENTED_FUNC(glRasterPos3d);
GL_UNIMPLEMENTED_FUNC(glRasterPos3dv);
GL_UNIMPLEMENTED_FUNC(glRasterPos3f);
GL_UNIMPLEMENTED_FUNC(glRasterPos3fv);
GL_UNIMPLEMENTED_FUNC(glRasterPos3i);
GL_UNIMPLEMENTED_FUNC(glRasterPos3iv);
GL_UNIMPLEMENTED_FUNC(glRasterPos3s);
GL_UNIMPLEMENTED_FUNC(glRasterPos3sv);
GL_UNIMPLEMENTED_FUNC(glRasterPos4d);
GL_UNIMPLEMENTED_FUNC(glRasterPos4dv);
GL_UNIMPLEMENTED_FUNC(glRasterPos4f);
GL_UNIMPLEMENTED_FUNC(glRasterPos4fv);
GL_UNIMPLEMENTED_FUNC(glRasterPos4i);
GL_UNIMPLEMENTED_FUNC(glRasterPos4iv);
GL_UNIMPLEMENTED_FUNC(glRasterPos4s);
GL_UNIMPLEMENTED_FUNC(glRasterPos4sv);
GL_UNIMPLEMENTED_FUNC(glRectd);
GL_UNIMPLEMENTED_FUNC(glRectdv);
GL_UNIMPLEMENTED_FUNC(glRectf);
GL_UNIMPLEMENTED_FUNC(glRectfv);
GL_UNIMPLEMENTED_FUNC(glRecti);
GL_UNIMPLEMENTED_FUNC(glRectiv);
GL_UNIMPLEMENTED_FUNC(glRects);
GL_UNIMPLEMENTED_FUNC(glRectsv);
GL_UNIMPLEMENTED_FUNC(glTexCoord1d);
GL_UNIMPLEMENTED_FUNC(glTexCoord1dv);
GL_UNIMPLEMENTED_FUNC(glTexCoord1f);
GL_UNIMPLEMENTED_FUNC(glTexCoord1fv);
GL_UNIMPLEMENTED_FUNC(glTexCoord1i);
GL_UNIMPLEMENTED_FUNC(glTexCoord1iv);
GL_UNIMPLEMENTED_FUNC(glTexCoord1s);
GL_UNIMPLEMENTED_FUNC(glTexCoord1sv);
GL_UNIMPLEMENTED_FUNC(glTexCoord2d);
GL_UNIMPLEMENTED_FUNC(glTexCoord2dv);
GL_UNIMPLEMENTED_FUNC(glTexCoord2f);
GL_UNIMPLEMENTED_FUNC(glTexCoord2fv);
GL_UNIMPLEMENTED_FUNC(glTexCoord2i);
GL_UNIMPLEMENTED_FUNC(glTexCoord2iv);
GL_UNIMPLEMENTED_FUNC(glTexCoord2s);
GL_UNIMPLEMENTED_FUNC(glTexCoord2sv);
GL_UNIMPLEMENTED_FUNC(glTexCoord3d);
GL_UNIMPLEMENTED_FUNC(glTexCoord3dv);
GL_UNIMPLEMENTED_FUNC(glTexCoord3f);
GL_UNIMPLEMENTED_FUNC(glTexCoord3fv);
GL_UNIMPLEMENTED_FUNC(glTexCoord3i);
GL_UNIMPLEMENTED_FUNC(glTexCoord3iv);
GL_UNIMPLEMENTED_FUNC(glTexCoord3s);
GL_UNIMPLEMENTED_FUNC(glTexCoord3sv);
GL_UNIMPLEMENTED_FUNC(glTexCoord4d);
GL_UNIMPLEMENTED_FUNC(glTexCoord4dv);
GL_UNIMPLEMENTED_FUNC(glTexCoord4f);
GL_UNIMPLEMENTED_FUNC(glTexCoord4fv);
GL_UNIMPLEMENTED_FUNC(glTexCoord4i);
GL_UNIMPLEMENTED_FUNC(glTexCoord4iv);
GL_UNIMPLEMENTED_FUNC(glTexCoord4s);
GL_UNIMPLEMENTED_FUNC(glTexCoord4sv);
GL_UNIMPLEMENTED_FUNC(glVertex2d);
GL_UNIMPLEMENTED_FUNC(glVertex2dv);
GL_UNIMPLEMENTED_FUNC(glVertex2f);
GL_UNIMPLEMENTED_FUNC(glVertex2fv);
GL_UNIMPLEMENTED_FUNC(glVertex2i);
GL_UNIMPLEMENTED_FUNC(glVertex2iv);
GL_UNIMPLEMENTED_FUNC(glVertex2s);
GL_UNIMPLEMENTED_FUNC(glVertex2sv);
GL_UNIMPLEMENTED_FUNC(glVertex3d);
GL_UNIMPLEMENTED_FUNC(glVertex3dv);
GL_UNIMPLEMENTED_FUNC(glVertex3f);
GL_UNIMPLEMENTED_FUNC(glVertex3fv);
GL_UNIMPLEMENTED_FUNC(glVertex3i);
GL_UNIMPLEMENTED_FUNC(glVertex3iv);
GL_UNIMPLEMENTED_FUNC(glVertex3s);
GL_UNIMPLEMENTED_FUNC(glVertex3sv);
GL_UNIMPLEMENTED_FUNC(glVertex4d);
GL_UNIMPLEMENTED_FUNC(glVertex4dv);
GL_UNIMPLEMENTED_FUNC(glVertex4f);
GL_UNIMPLEMENTED_FUNC(glVertex4fv);
GL_UNIMPLEMENTED_FUNC(glVertex4i);
GL_UNIMPLEMENTED_FUNC(glVertex4iv);
GL_UNIMPLEMENTED_FUNC(glVertex4s);
GL_UNIMPLEMENTED_FUNC(glVertex4sv);
GL_UNIMPLEMENTED_FUNC(glClipPlane);
GL_UNIMPLEMENTED_FUNC(glColorMaterial);
GL_UNIMPLEMENTED_FUNC(glCullFace);
GL_UNIMPLEMENTED_FUNC(glFogf);
GL_UNIMPLEMENTED_FUNC(glFogfv);
GL_UNIMPLEMENTED_FUNC(glFogi);
GL_UNIMPLEMENTED_FUNC(glFogiv);
GL_UNIMPLEMENTED_FUNC(glFrontFace);
GL_UNIMPLEMENTED_FUNC(glHint);
GL_UNIMPLEMENTED_FUNC(glLightf);
GL_UNIMPLEMENTED_FUNC(glLightfv);
GL_UNIMPLEMENTED_FUNC(glLighti);
GL_UNIMPLEMENTED_FUNC(glLightiv);
GL_UNIMPLEMENTED_FUNC(glLightModelf);
GL_UNIMPLEMENTED_FUNC(glLightModelfv);
GL_UNIMPLEMENTED_FUNC(glLightModeli);
GL_UNIMPLEMENTED_FUNC(glLightModeliv);
GL_UNIMPLEMENTED_FUNC(glLineStipple);
GL_UNIMPLEMENTED_FUNC(glLineWidth);
GL_UNIMPLEMENTED_FUNC(glMaterialf);
GL_UNIMPLEMENTED_FUNC(glMaterialfv);
GL_UNIMPLEMENTED_FUNC(glMateriali);
GL_UNIMPLEMENTED_FUNC(glMaterialiv);
GL_UNIMPLEMENTED_FUNC(glPointSize);
GL_UNIMPLEMENTED_FUNC(glPolygonMode);
GL_UNIMPLEMENTED_FUNC(glPolygonStipple);
GL_UNIMPLEMENTED_FUNC(glScissor);
GL_UNIMPLEMENTED_FUNC(glShadeModel);
GL_UNIMPLEMENTED_FUNC(glTexParameterf);
GL_UNIMPLEMENTED_FUNC(glTexParameterfv);
GL_UNIMPLEMENTED_FUNC(glTexParameteri);
GL_UNIMPLEMENTED_FUNC(glTexParameteriv);
GL_UNIMPLEMENTED_FUNC(glTexImage1D);
GL_UNIMPLEMENTED_FUNC(glTexImage2D);
GL_UNIMPLEMENTED_FUNC(glTexEnvf);
GL_UNIMPLEMENTED_FUNC(glTexEnvfv);
GL_UNIMPLEMENTED_FUNC(glTexEnvi);
GL_UNIMPLEMENTED_FUNC(glTexEnviv);
GL_UNIMPLEMENTED_FUNC(glTexGend);
GL_UNIMPLEMENTED_FUNC(glTexGendv);
GL_UNIMPLEMENTED_FUNC(glTexGenf);
GL_UNIMPLEMENTED_FUNC(glTexGenfv);
GL_UNIMPLEMENTED_FUNC(glTexGeni);
GL_UNIMPLEMENTED_FUNC(glTexGeniv);
GL_UNIMPLEMENTED_FUNC(glFeedbackBuffer);
GL_UNIMPLEMENTED_FUNC(glSelectBuffer);
GL_UNIMPLEMENTED_FUNC(glRenderMode);
GL_UNIMPLEMENTED_FUNC(glInitNames);
GL_UNIMPLEMENTED_FUNC(glLoadName);
GL_UNIMPLEMENTED_FUNC(glPassThrough);
GL_UNIMPLEMENTED_FUNC(glPopName);
GL_UNIMPLEMENTED_FUNC(glPushName);
GL_UNIMPLEMENTED_FUNC(glDrawBuffer);
GL_UNIMPLEMENTED_FUNC(glClear);
GL_UNIMPLEMENTED_FUNC(glClearAccum);
GL_UNIMPLEMENTED_FUNC(glClearIndex);
GL_UNIMPLEMENTED_FUNC(glClearColor);
GL_UNIMPLEMENTED_FUNC(glClearStencil);
GL_UNIMPLEMENTED_FUNC(glClearDepth);
GL_UNIMPLEMENTED_FUNC(glStencilMask);
GL_UNIMPLEMENTED_FUNC(glColorMask);
GL_UNIMPLEMENTED_FUNC(glDepthMask);
GL_UNIMPLEMENTED_FUNC(glIndexMask);
GL_UNIMPLEMENTED_FUNC(glAccum);
GL_UNIMPLEMENTED_FUNC(glDisable);
GL_UNIMPLEMENTED_FUNC(glEnable);
GL_UNIMPLEMENTED_FUNC(glFinish);
GL_UNIMPLEMENTED_FUNC(glFlush);
GL_UNIMPLEMENTED_FUNC(glPopAttrib);
GL_UNIMPLEMENTED_FUNC(glPushAttrib);
GL_UNIMPLEMENTED_FUNC(glMap1d);
GL_UNIMPLEMENTED_FUNC(glMap1f);
GL_UNIMPLEMENTED_FUNC(glMap2d);
GL_UNIMPLEMENTED_FUNC(glMap2f);
GL_UNIMPLEMENTED_FUNC(glMapGrid1d);
GL_UNIMPLEMENTED_FUNC(glMapGrid1f);
GL_UNIMPLEMENTED_FUNC(glMapGrid2d);
GL_UNIMPLEMENTED_FUNC(glMapGrid2f);
GL_UNIMPLEMENTED_FUNC(glEvalCoord1d);
GL_UNIMPLEMENTED_FUNC(glEvalCoord1dv);
GL_UNIMPLEMENTED_FUNC(glEvalCoord1f);
GL_UNIMPLEMENTED_FUNC(glEvalCoord1fv);
GL_UNIMPLEMENTED_FUNC(glEvalCoord2d);
GL_UNIMPLEMENTED_FUNC(glEvalCoord2dv);
GL_UNIMPLEMENTED_FUNC(glEvalCoord2f);
GL_UNIMPLEMENTED_FUNC(glEvalCoord2fv);
GL_UNIMPLEMENTED_FUNC(glEvalMesh1);
GL_UNIMPLEMENTED_FUNC(glEvalPoint1);
GL_UNIMPLEMENTED_FUNC(glEvalMesh2);
GL_UNIMPLEMENTED_FUNC(glEvalPoint2);
GL_UNIMPLEMENTED_FUNC(glAlphaFunc);
GL_UNIMPLEMENTED_FUNC(glBlendFunc);
GL_UNIMPLEMENTED_FUNC(glLogicOp);
GL_UNIMPLEMENTED_FUNC(glStencilFunc);
GL_UNIMPLEMENTED_FUNC(glStencilOp);
GL_UNIMPLEMENTED_FUNC(glDepthFunc);
GL_UNIMPLEMENTED_FUNC(glPixelZoom);
GL_UNIMPLEMENTED_FUNC(glPixelTransferf);
GL_UNIMPLEMENTED_FUNC(glPixelTransferi);
GL_UNIMPLEMENTED_FUNC(glPixelStoref);
GL_UNIMPLEMENTED_FUNC(glPixelStorei);
GL_UNIMPLEMENTED_FUNC(glPixelMapfv);
GL_UNIMPLEMENTED_FUNC(glPixelMapuiv);
GL_UNIMPLEMENTED_FUNC(glPixelMapusv);
GL_UNIMPLEMENTED_FUNC(glReadBuffer);
GL_UNIMPLEMENTED_FUNC(glCopyPixels);
GL_UNIMPLEMENTED_FUNC(glReadPixels);
GL_UNIMPLEMENTED_FUNC(glDrawPixels);
GL_UNIMPLEMENTED_FUNC(glGetBooleanv);
GL_UNIMPLEMENTED_FUNC(glGetClipPlane);
GL_UNIMPLEMENTED_FUNC(glGetDoublev);
GL_UNIMPLEMENTED_FUNC(glGetError);
GL_UNIMPLEMENTED_FUNC(glGetFloatv);
GL_UNIMPLEMENTED_FUNC(glGetIntegerv);
GL_UNIMPLEMENTED_FUNC(glGetLightfv);
GL_UNIMPLEMENTED_FUNC(glGetLightiv);
GL_UNIMPLEMENTED_FUNC(glGetMapdv);
GL_UNIMPLEMENTED_FUNC(glGetMapfv);
GL_UNIMPLEMENTED_FUNC(glGetMapiv);
GL_UNIMPLEMENTED_FUNC(glGetMaterialfv);
GL_UNIMPLEMENTED_FUNC(glGetMaterialiv);
GL_UNIMPLEMENTED_FUNC(glGetPixelMapfv);
GL_UNIMPLEMENTED_FUNC(glGetPixelMapuiv);
GL_UNIMPLEMENTED_FUNC(glGetPixelMapusv);
GL_UNIMPLEMENTED_FUNC(glGetPolygonStipple);
GL_UNIMPLEMENTED_FUNC(glGetString);
GL_UNIMPLEMENTED_FUNC(glGetTexEnvfv);
GL_UNIMPLEMENTED_FUNC(glGetTexEnviv);
GL_UNIMPLEMENTED_FUNC(glGetTexGendv);
GL_UNIMPLEMENTED_FUNC(glGetTexGenfv);
GL_UNIMPLEMENTED_FUNC(glGetTexGeniv);
GL_UNIMPLEMENTED_FUNC(glGetTexImage);
GL_UNIMPLEMENTED_FUNC(glGetTexParameterfv);
GL_UNIMPLEMENTED_FUNC(glGetTexParameteriv);
GL_UNIMPLEMENTED_FUNC(glGetTexLevelParameterfv);
GL_UNIMPLEMENTED_FUNC(glGetTexLevelParameteriv);
GL_UNIMPLEMENTED_FUNC(glIsEnabled);
GL_UNIMPLEMENTED_FUNC(glIsList);
GL_UNIMPLEMENTED_FUNC(glDepthRange);
GL_UNIMPLEMENTED_FUNC(glFrustum);
GL_UNIMPLEMENTED_FUNC(glLoadIdentity);
GL_UNIMPLEMENTED_FUNC(glLoadMatrixf);
GL_UNIMPLEMENTED_FUNC(glLoadMatrixd);
GL_UNIMPLEMENTED_FUNC(glMatrixMode);
GL_UNIMPLEMENTED_FUNC(glMultMatrixf);
GL_UNIMPLEMENTED_FUNC(glMultMatrixd);
GL_UNIMPLEMENTED_FUNC(glOrtho);
GL_UNIMPLEMENTED_FUNC(glPopMatrix);
GL_UNIMPLEMENTED_FUNC(glPushMatrix);
GL_UNIMPLEMENTED_FUNC(glRotated);
GL_UNIMPLEMENTED_FUNC(glRotatef);
GL_UNIMPLEMENTED_FUNC(glScaled);
GL_UNIMPLEMENTED_FUNC(glScalef);
GL_UNIMPLEMENTED_FUNC(glTranslated);
GL_UNIMPLEMENTED_FUNC(glTranslatef);
GL_UNIMPLEMENTED_FUNC(glViewport);
GL_UNIMPLEMENTED_FUNC(glArrayElement);
GL_UNIMPLEMENTED_FUNC(glBindTexture);
GL_UNIMPLEMENTED_FUNC(glColorPointer);
GL_UNIMPLEMENTED_FUNC(glDisableClientState);
GL_UNIMPLEMENTED_FUNC(glDrawArrays);
GL_UNIMPLEMENTED_FUNC(glDrawElements);
GL_UNIMPLEMENTED_FUNC(glEdgeFlagPointer);
GL_UNIMPLEMENTED_FUNC(glEnableClientState);
GL_UNIMPLEMENTED_FUNC(glIndexPointer);
GL_UNIMPLEMENTED_FUNC(glIndexub);
GL_UNIMPLEMENTED_FUNC(glIndexubv);
GL_UNIMPLEMENTED_FUNC(glInterleavedArrays);
GL_UNIMPLEMENTED_FUNC(glNormalPointer);
GL_UNIMPLEMENTED_FUNC(glPolygonOffset);
GL_UNIMPLEMENTED_FUNC(glTexCoordPointer);
GL_UNIMPLEMENTED_FUNC(glVertexPointer);
GL_UNIMPLEMENTED_FUNC(glAreTexturesResident);
GL_UNIMPLEMENTED_FUNC(glCopyTexImage1D);
GL_UNIMPLEMENTED_FUNC(glCopyTexImage2D);
GL_UNIMPLEMENTED_FUNC(glCopyTexSubImage1D);
GL_UNIMPLEMENTED_FUNC(glCopyTexSubImage2D);
GL_UNIMPLEMENTED_FUNC(glDeleteTextures);
GL_UNIMPLEMENTED_FUNC(glGenTextures);
GL_UNIMPLEMENTED_FUNC(glGetPointerv);
GL_UNIMPLEMENTED_FUNC(glIsTexture);
GL_UNIMPLEMENTED_FUNC(glPrioritizeTextures);
GL_UNIMPLEMENTED_FUNC(glTexSubImage1D);
GL_UNIMPLEMENTED_FUNC(glTexSubImage2D);
GL_UNIMPLEMENTED_FUNC(glPopClientAttrib);
GL_UNIMPLEMENTED_FUNC(glPushClientAttrib);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
static const GLCLTPROCTABLE cpt =
{
    OPENGL_VERSION_110_ENTRIES,
    {
        &unimp_glNewList,
        &unimp_glEndList,
        &unimp_glCallList,
        &unimp_glCallLists,
        &unimp_glDeleteLists,
        &unimp_glGenLists,
        &unimp_glListBase,
        &unimp_glBegin,
        &unimp_glBitmap,
        &unimp_glColor3b,
        &unimp_glColor3bv,
        &unimp_glColor3d,
        &unimp_glColor3dv,
        &unimp_glColor3f,
        &unimp_glColor3fv,
        &unimp_glColor3i,
        &unimp_glColor3iv,
        &unimp_glColor3s,
        &unimp_glColor3sv,
        &unimp_glColor3ub,
        &unimp_glColor3ubv,
        &unimp_glColor3ui,
        &unimp_glColor3uiv,
        &unimp_glColor3us,
        &unimp_glColor3usv,
        &unimp_glColor4b,
        &unimp_glColor4bv,
        &unimp_glColor4d,
        &unimp_glColor4dv,
        &unimp_glColor4f,
        &unimp_glColor4fv,
        &unimp_glColor4i,
        &unimp_glColor4iv,
        &unimp_glColor4s,
        &unimp_glColor4sv,
        &unimp_glColor4ub,
        &unimp_glColor4ubv,
        &unimp_glColor4ui,
        &unimp_glColor4uiv,
        &unimp_glColor4us,
        &unimp_glColor4usv,
        &unimp_glEdgeFlag,
        &unimp_glEdgeFlagv,
        &unimp_glEnd,
        &unimp_glIndexd,
        &unimp_glIndexdv,
        &unimp_glIndexf,
        &unimp_glIndexfv,
        &unimp_glIndexi,
        &unimp_glIndexiv,
        &unimp_glIndexs,
        &unimp_glIndexsv,
        &unimp_glNormal3b,
        &unimp_glNormal3bv,
        &unimp_glNormal3d,
        &unimp_glNormal3dv,
        &unimp_glNormal3f,
        &unimp_glNormal3fv,
        &unimp_glNormal3i,
        &unimp_glNormal3iv,
        &unimp_glNormal3s,
        &unimp_glNormal3sv,
        &unimp_glRasterPos2d,
        &unimp_glRasterPos2dv,
        &unimp_glRasterPos2f,
        &unimp_glRasterPos2fv,
        &unimp_glRasterPos2i,
        &unimp_glRasterPos2iv,
        &unimp_glRasterPos2s,
        &unimp_glRasterPos2sv,
        &unimp_glRasterPos3d,
        &unimp_glRasterPos3dv,
        &unimp_glRasterPos3f,
        &unimp_glRasterPos3fv,
        &unimp_glRasterPos3i,
        &unimp_glRasterPos3iv,
        &unimp_glRasterPos3s,
        &unimp_glRasterPos3sv,
        &unimp_glRasterPos4d,
        &unimp_glRasterPos4dv,
        &unimp_glRasterPos4f,
        &unimp_glRasterPos4fv,
        &unimp_glRasterPos4i,
        &unimp_glRasterPos4iv,
        &unimp_glRasterPos4s,
        &unimp_glRasterPos4sv,
        &unimp_glRectd,
        &unimp_glRectdv,
        &unimp_glRectf,
        &unimp_glRectfv,
        &unimp_glRecti,
        &unimp_glRectiv,
        &unimp_glRects,
        &unimp_glRectsv,
        &unimp_glTexCoord1d,
        &unimp_glTexCoord1dv,
        &unimp_glTexCoord1f,
        &unimp_glTexCoord1fv,
        &unimp_glTexCoord1i,
        &unimp_glTexCoord1iv,
        &unimp_glTexCoord1s,
        &unimp_glTexCoord1sv,
        &unimp_glTexCoord2d,
        &unimp_glTexCoord2dv,
        &unimp_glTexCoord2f,
        &unimp_glTexCoord2fv,
        &unimp_glTexCoord2i,
        &unimp_glTexCoord2iv,
        &unimp_glTexCoord2s,
        &unimp_glTexCoord2sv,
        &unimp_glTexCoord3d,
        &unimp_glTexCoord3dv,
        &unimp_glTexCoord3f,
        &unimp_glTexCoord3fv,
        &unimp_glTexCoord3i,
        &unimp_glTexCoord3iv,
        &unimp_glTexCoord3s,
        &unimp_glTexCoord3sv,
        &unimp_glTexCoord4d,
        &unimp_glTexCoord4dv,
        &unimp_glTexCoord4f,
        &unimp_glTexCoord4fv,
        &unimp_glTexCoord4i,
        &unimp_glTexCoord4iv,
        &unimp_glTexCoord4s,
        &unimp_glTexCoord4sv,
        &unimp_glVertex2d,
        &unimp_glVertex2dv,
        &unimp_glVertex2f,
        &unimp_glVertex2fv,
        &unimp_glVertex2i,
        &unimp_glVertex2iv,
        &unimp_glVertex2s,
        &unimp_glVertex2sv,
        &unimp_glVertex3d,
        &unimp_glVertex3dv,
        &unimp_glVertex3f,
        &unimp_glVertex3fv,
        &unimp_glVertex3i,
        &unimp_glVertex3iv,
        &unimp_glVertex3s,
        &unimp_glVertex3sv,
        &unimp_glVertex4d,
        &unimp_glVertex4dv,
        &unimp_glVertex4f,
        &unimp_glVertex4fv,
        &unimp_glVertex4i,
        &unimp_glVertex4iv,
        &unimp_glVertex4s,
        &unimp_glVertex4sv,
        &unimp_glClipPlane,
        &unimp_glColorMaterial,
        &unimp_glCullFace,
        &unimp_glFogf,
        &unimp_glFogfv,
        &unimp_glFogi,
        &unimp_glFogiv,
        &unimp_glFrontFace,
        &unimp_glHint,
        &unimp_glLightf,
        &unimp_glLightfv,
        &unimp_glLighti,
        &unimp_glLightiv,
        &unimp_glLightModelf,
        &unimp_glLightModelfv,
        &unimp_glLightModeli,
        &unimp_glLightModeliv,
        &unimp_glLineStipple,
        &unimp_glLineWidth,
        &unimp_glMaterialf,
        &unimp_glMaterialfv,
        &unimp_glMateriali,
        &unimp_glMaterialiv,
        &unimp_glPointSize,
        &unimp_glPolygonMode,
        &unimp_glPolygonStipple,
        &unimp_glScissor,
        &unimp_glShadeModel,
        &unimp_glTexParameterf,
        &unimp_glTexParameterfv,
        &unimp_glTexParameteri,
        &unimp_glTexParameteriv,
        &unimp_glTexImage1D,
        &unimp_glTexImage2D,
        &unimp_glTexEnvf,
        &unimp_glTexEnvfv,
        &unimp_glTexEnvi,
        &unimp_glTexEnviv,
        &unimp_glTexGend,
        &unimp_glTexGendv,
        &unimp_glTexGenf,
        &unimp_glTexGenfv,
        &unimp_glTexGeni,
        &unimp_glTexGeniv,
        &unimp_glFeedbackBuffer,
        &unimp_glSelectBuffer,
        &unimp_glRenderMode,
        &unimp_glInitNames,
        &unimp_glLoadName,
        &unimp_glPassThrough,
        &unimp_glPopName,
        &unimp_glPushName,
        &unimp_glDrawBuffer,
        &unimp_glClear,
        &unimp_glClearAccum,
        &unimp_glClearIndex,
        &unimp_glClearColor,
        &unimp_glClearStencil,
        &unimp_glClearDepth,
        &unimp_glStencilMask,
        &unimp_glColorMask,
        &unimp_glDepthMask,
        &unimp_glIndexMask,
        &unimp_glAccum,
        &unimp_glDisable,
        &unimp_glEnable,
        &unimp_glFinish,
        &unimp_glFlush,
        &unimp_glPopAttrib,
        &unimp_glPushAttrib,
        &unimp_glMap1d,
        &unimp_glMap1f,
        &unimp_glMap2d,
        &unimp_glMap2f,
        &unimp_glMapGrid1d,
        &unimp_glMapGrid1f,
        &unimp_glMapGrid2d,
        &unimp_glMapGrid2f,
        &unimp_glEvalCoord1d,
        &unimp_glEvalCoord1dv,
        &unimp_glEvalCoord1f,
        &unimp_glEvalCoord1fv,
        &unimp_glEvalCoord2d,
        &unimp_glEvalCoord2dv,
        &unimp_glEvalCoord2f,
        &unimp_glEvalCoord2fv,
        &unimp_glEvalMesh1,
        &unimp_glEvalPoint1,
        &unimp_glEvalMesh2,
        &unimp_glEvalPoint2,
        &unimp_glAlphaFunc,
        &unimp_glBlendFunc,
        &unimp_glLogicOp,
        &unimp_glStencilFunc,
        &unimp_glStencilOp,
        &unimp_glDepthFunc,
        &unimp_glPixelZoom,
        &unimp_glPixelTransferf,
        &unimp_glPixelTransferi,
        &unimp_glPixelStoref,
        &unimp_glPixelStorei,
        &unimp_glPixelMapfv,
        &unimp_glPixelMapuiv,
        &unimp_glPixelMapusv,
        &unimp_glReadBuffer,
        &unimp_glCopyPixels,
        &unimp_glReadPixels,
        &unimp_glDrawPixels,
        &unimp_glGetBooleanv,
        &unimp_glGetClipPlane,
        &unimp_glGetDoublev,
        &unimp_glGetError,
        &unimp_glGetFloatv,
        &unimp_glGetIntegerv,
        &unimp_glGetLightfv,
        &unimp_glGetLightiv,
        &unimp_glGetMapdv,
        &unimp_glGetMapfv,
        &unimp_glGetMapiv,
        &unimp_glGetMaterialfv,
        &unimp_glGetMaterialiv,
        &unimp_glGetPixelMapfv,
        &unimp_glGetPixelMapuiv,
        &unimp_glGetPixelMapusv,
        &unimp_glGetPolygonStipple,
        &unimp_glGetString,
        &unimp_glGetTexEnvfv,
        &unimp_glGetTexEnviv,
        &unimp_glGetTexGendv,
        &unimp_glGetTexGenfv,
        &unimp_glGetTexGeniv,
        &unimp_glGetTexImage,
        &unimp_glGetTexParameterfv,
        &unimp_glGetTexParameteriv,
        &unimp_glGetTexLevelParameterfv,
        &unimp_glGetTexLevelParameteriv,
        &unimp_glIsEnabled,
        &unimp_glIsList,
        &unimp_glDepthRange,
        &unimp_glFrustum,
        &unimp_glLoadIdentity,
        &unimp_glLoadMatrixf,
        &unimp_glLoadMatrixd,
        &unimp_glMatrixMode,
        &unimp_glMultMatrixf,
        &unimp_glMultMatrixd,
        &unimp_glOrtho,
        &unimp_glPopMatrix,
        &unimp_glPushMatrix,
        &unimp_glRotated,
        &unimp_glRotatef,
        &unimp_glScaled,
        &unimp_glScalef,
        &unimp_glTranslated,
        &unimp_glTranslatef,
        &unimp_glViewport,
        &unimp_glArrayElement,
        &unimp_glBindTexture,
        &unimp_glColorPointer,
        &unimp_glDisableClientState,
        &unimp_glDrawArrays,
        &unimp_glDrawElements,
        &unimp_glEdgeFlagPointer,
        &unimp_glEnableClientState,
        &unimp_glIndexPointer,
        &unimp_glIndexub,
        &unimp_glIndexubv,
        &unimp_glInterleavedArrays,
        &unimp_glNormalPointer,
        &unimp_glPolygonOffset,
        &unimp_glTexCoordPointer,
        &unimp_glVertexPointer,
        &unimp_glAreTexturesResident,
        &unimp_glCopyTexImage1D,
        &unimp_glCopyTexImage2D,
        &unimp_glCopyTexSubImage1D,
        &unimp_glCopyTexSubImage2D,
        &unimp_glDeleteTextures,
        &unimp_glGenTextures,
        &unimp_glGetPointerv,
        &unimp_glIsTexture,
        &unimp_glPrioritizeTextures,
        &unimp_glTexSubImage1D,
        &unimp_glTexSubImage2D,
        &unimp_glPopClientAttrib,
        &unimp_glPushClientAttrib
    }
};
#pragma GCC diagnostic pop

BOOL
WINAPI
DrvCopyContext(
    HGLRC srcDhglrc,
    HGLRC dstDhglrc,
    UINT mask)
{
    DPRINT("DrvCopyContext\n");
    return FALSE;
}


HGLRC
APIENTRY
DrvCreateContext(
    HDC hdc)
{
    DPRINT("DrvCreateContext\n");

    PVIRGL_CONTEXT context = (PVIRGL_CONTEXT)HeapAlloc(GetProcessHeap(), 0, sizeof(VIRGL_CONTEXT));

    if(WindowFromDC(hdc) != NULL)
        context->hDC = GetDC(WindowFromDC(hdc));
    else
        context->hDC = hdc;

    VIRTIO_GPU_CREATE_CONTEXT createContext;
    sprintf((char *)createContext.Name, "virgl_%lu", ContextID++);

    VIRTIO_GPU_RESOURCE_ID resourceId;
    if(!ExtEscape(context->hDC, VIRTIO_GPU_ESC_CREATE_CONTEXT, sizeof(VIRTIO_GPU_CREATE_CONTEXT), (LPCSTR)&createContext, sizeof(VIRTIO_GPU_RESOURCE_ID), (LPSTR)&resourceId))
    {
        HeapFree(GetProcessHeap(), 0, (PVOID)context);
        return (HGLRC)NULL;
    }

    context->ResourceID = resourceId.ResourceID;

    return (HGLRC)context;
}

HGLRC
APIENTRY
DrvCreateLayerContext(
    HDC hdc,
    int iLayerPlane)
{
    DPRINT("DrvCreateLayerContext\n");
    return (HGLRC)NULL;
}

BOOL
APIENTRY
DrvDeleteContext(
    HGLRC dhglrc)
{
    DPRINT("DrvDeleteContext\n");
    return FALSE;
}

BOOL
APIENTRY
DrvDescribeLayerPlane(
    HDC hdc,
    INT iPixelFormat,
    INT iLayerPlane,
    UINT nBytes,
    LPLAYERPLANEDESCRIPTOR plpd)
{
    DPRINT("DrvDescribeLayerPlane\n");
    return FALSE;
}

INT
APIENTRY
DrvDescribePixelFormat(
    HDC hdc,
    INT ipfd,
    UINT cjpfd,
    PPIXELFORMATDESCRIPTOR ppfd)
{
    DPRINT("DrvDescribePixelFormat\n");

    if(ppfd && ipfd == 1) {
        if(cjpfd < sizeof(PIXELFORMATDESCRIPTOR))
            return 0;

        memset(ppfd, 0, sizeof(PIXELFORMATDESCRIPTOR));

        ppfd->nSize = sizeof(PIXELFORMATDESCRIPTOR);
        ppfd->nVersion = 1;
        ppfd->dwFlags = PFD_DRAW_TO_WINDOW | PFD_DRAW_TO_BITMAP | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        ppfd->iPixelType = PFD_TYPE_RGBA;

        ppfd->cColorBits = 32;
        ppfd->cRedBits = 8;
        ppfd->cRedShift = 16;
        ppfd->cGreenBits = 8;
        ppfd->cGreenShift = 8;
        ppfd->cBlueBits = 8;
        ppfd->cBlueShift = 0;
        ppfd->cAlphaBits = 8;
        ppfd->cAlphaShift = 24;

        ppfd->cAccumBits = 0;
        ppfd->cAccumRedBits = 0;
        ppfd->cAccumGreenBits = 0;
        ppfd->cAccumBlueBits = 0;
        ppfd->cAccumAlphaBits = 0;
        ppfd->cDepthBits = 0;
        ppfd->cStencilBits = 0;

        ppfd->dwVisibleMask = 0x00FFFFFF;
    }

    return 1;
}

INT
APIENTRY
DrvGetLayerPaletteEntries(
    HDC hdc,
    INT iLayerPlane,
    INT iStart,
    INT cEntries,
    COLORREF *pcr)
{
    DPRINT("DrvGetLayerPaletteEntries\n");
    return (INT)NULL;
}

PROC
APIENTRY
DrvGetProcAddress(
    LPCSTR lpProcName)
{
    DPRINT("DrvGetProcAddress\n");
    return (PROC)NULL;
}

VOID
APIENTRY
DrvReleaseContext(
    HGLRC hglrc)
{
    DPRINT("DrvReleaseContext\n");
}

BOOL
APIENTRY
DrvRealizeLayerPalette(
    HDC hdc,
    INT iLayerPlane,
    BOOL bRealize)
{
    DPRINT("DrvRealizeLayerPalette\n");
    return FALSE;
}

PGLCLTPROCTABLE
APIENTRY
DrvSetContext(
    HDC hdc,
    HGLRC hglrc,
    PFN_SETPROCTABLE callback)
{
    DPRINT("DrvSetContext\n");

    PVIRGL_CONTEXT context = (PVIRGL_CONTEXT)hglrc;
    if(context->hDC != hdc) return NULL;

    VIRTIO_GPU_RESOURCE_ID resourceId;
    resourceId.ResourceID = context->ResourceID;
    if(!ExtEscape(context->hDC, VIRTIO_GPU_ESC_CREATE_CONTEXT, sizeof(VIRTIO_GPU_RESOURCE_ID), (LPCSTR)&resourceId, 0, (LPSTR)NULL))
        return NULL;

    //Get table
    PGLCLTPROCTABLE result = (PGLCLTPROCTABLE)&cpt;

    return result;
}

INT
APIENTRY
DrvSetLayerPaletteEntries(
    HDC hdc,
    INT iLayerPlane,
    INT iStart,
    INT cEntries,
    CONST COLORREF *pcr)
{
    DPRINT("DrvSetLayerPaletteEntries\n");
    return (INT)NULL;
}

BOOL
APIENTRY
DrvSetPixelFormat(
    IN HDC hdc,
    IN INT format)
{
    DPRINT("DrvSetPixelFormat\n");
    return TRUE; //I only support one, so yeah, always set
}

BOOL
APIENTRY
DrvShareLists(
    HGLRC srcDhglrc,
    HGLRC dstDhglrc)
{
    DPRINT("DrvShareLists\n");
    return FALSE;
}

BOOL
APIENTRY
DrvSwapBuffers(
    HDC hdc)
{
    DPRINT("DrvSwapBuffers\n");
    return FALSE;
}

BOOL
APIENTRY
DrvSwapLayerBuffers(
    HDC hdc,
    UINT _iLayerPlane)
{
    DPRINT("DrvSwapLayerBuffers\n");
    return FALSE;
}