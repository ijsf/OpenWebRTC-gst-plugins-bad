/*
 * GStreamer
 * Copyright (C) 2008 Filippo Argiolas <filippo.argiolas@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "../gstgleffects.h"
#include "gstgleffectlumatocurve.h"

void
gst_gl_effects_luma_to_curve (GstGLEffects * effects,
    const GstGLEffectsCurve * curve,
    gint curve_index, gint width, gint height, GLuint texture)
{
  GstGLShader *shader;
  GstGLFilter *filter = GST_GL_FILTER (effects);
  GstGLContext *context = GST_GL_BASE_FILTER (filter)->context;
  GstGLFuncs *gl = context->gl_vtable;

  shader = gst_gl_effects_get_fragment_shader (effects, "luma_to_curve",
      luma_to_curve_fragment_source_gles2,
      luma_to_curve_fragment_source_opengl);

  if (!shader)
    return;

#if GST_GL_HAVE_OPENGL
  if (USING_OPENGL (context)) {
    gl->MatrixMode (GL_PROJECTION);
    gl->LoadIdentity ();
  }
#endif

  gst_gl_shader_use (shader);

  if (effects->curve[curve_index] == 0) {
    /* this parameters are needed to have a right, predictable, mapping */
    gl->GenTextures (1, &effects->curve[curve_index]);

#if GST_GL_HAVE_OPENGL
    if (USING_OPENGL (context)) {
      gl->Enable (GL_TEXTURE_1D);
      gl->BindTexture (GL_TEXTURE_1D, effects->curve[curve_index]);
      gl->TexParameteri (GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      gl->TexParameteri (GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      gl->TexParameteri (GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP);
      gl->TexParameteri (GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP);

      gl->TexImage1D (GL_TEXTURE_1D, 0, curve->bytes_per_pixel,
          curve->width, 0, GL_RGB, GL_UNSIGNED_BYTE, curve->pixel_data);

      gl->Disable (GL_TEXTURE_1D);
    }
#endif
    if (USING_GLES2 (context) || USING_OPENGL3 (context)) {
      gl->Enable (GL_TEXTURE_2D);
      gl->BindTexture (GL_TEXTURE_2D, effects->curve[curve_index]);
      gl->TexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      gl->TexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      gl->TexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      gl->TexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

      gl->TexImage2D (GL_TEXTURE_2D, 0, GL_RGB,
          curve->width, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, curve->pixel_data);
      gl->Disable (GL_TEXTURE_2D);
    }
  }

  gl->ActiveTexture (GL_TEXTURE2);
  gl->Enable (GL_TEXTURE_2D);
  gl->BindTexture (GL_TEXTURE_2D, texture);

  gst_gl_shader_set_uniform_1i (shader, "tex", 2);

  gl->Disable (GL_TEXTURE_2D);

#if GST_GL_HAVE_OPENGL
  if (USING_OPENGL (context)) {
    gl->ActiveTexture (GL_TEXTURE1);
    gl->Enable (GL_TEXTURE_1D);
    gl->BindTexture (GL_TEXTURE_1D, effects->curve[curve_index]);

    gst_gl_shader_set_uniform_1i (shader, "curve", 1);

    gl->Disable (GL_TEXTURE_1D);
  }
#endif
  if (USING_GLES2 (context) || USING_OPENGL3 (context)) {
    gl->ActiveTexture (GL_TEXTURE1);
    gl->Enable (GL_TEXTURE_2D);
    gl->BindTexture (GL_TEXTURE_2D, effects->curve[curve_index]);

    gst_gl_shader_set_uniform_1i (shader, "curve", 1);

    gl->Disable (GL_TEXTURE_2D);
  }

  gst_gl_filter_draw_texture (filter, texture, width, height);
}

static void
gst_gl_effects_heat_callback (gint width, gint height, guint texture,
    gpointer data)
{
  GstGLEffects *effects = GST_GL_EFFECTS (data);

  gst_gl_effects_luma_to_curve (effects, &heat_curve, GST_GL_EFFECTS_CURVE_HEAT,
      width, height, texture);
}

void
gst_gl_effects_heat (GstGLEffects * effects)
{
  GstGLFilter *filter = GST_GL_FILTER (effects);

  gst_gl_filter_render_to_target (filter, TRUE, effects->intexture,
      effects->outtexture, gst_gl_effects_heat_callback, effects);
}

static void
gst_gl_effects_sepia_callback (gint width, gint height, guint texture,
    gpointer data)
{
  GstGLEffects *effects = GST_GL_EFFECTS (data);

  gst_gl_effects_luma_to_curve (effects, &sepia_curve,
      GST_GL_EFFECTS_CURVE_SEPIA, width, height, texture);
}

void
gst_gl_effects_sepia (GstGLEffects * effects)
{
  GstGLFilter *filter = GST_GL_FILTER (effects);

  gst_gl_filter_render_to_target (filter, TRUE, effects->intexture,
      effects->outtexture, gst_gl_effects_sepia_callback, effects);
}

static void
gst_gl_effects_luma_xpro_callback (gint width, gint height, guint texture,
    gpointer data)
{
  GstGLEffects *effects = GST_GL_EFFECTS (data);

  gst_gl_effects_luma_to_curve (effects, &luma_xpro_curve,
      GST_GL_EFFECTS_CURVE_LUMA_XPRO, width, height, texture);
}

void
gst_gl_effects_luma_xpro (GstGLEffects * effects)
{
  GstGLFilter *filter = GST_GL_FILTER (effects);

  gst_gl_filter_render_to_target (filter, TRUE, effects->intexture,
      effects->outtexture, gst_gl_effects_luma_xpro_callback, effects);
}
