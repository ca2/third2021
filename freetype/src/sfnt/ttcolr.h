﻿/****************************************************************************
 *
 * ttcolr.h
 *
 *   TrueType and OpenType colored glyph layer support (specification).
 *
 * Copyright (C) 2018-2020 by
 * David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 * Originally written by Shao Yu Zhang <shaozhang@fb.com>.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */


#ifndef __TTCOLR_H__
#define __TTCOLR_H__


#include <ft2build.h>
#include "ttload.h"


FT_BEGIN_HEADER


  FT_LOCAL( FT_Error )
  tt_face_load_colr( TT_Face    face,
                     FT_Stream  stream );

  FT_LOCAL( void )
  tt_face_free_colr( TT_Face  face );

  FT_LOCAL( FT_Bool )
  tt_face_get_colr_layer( TT_Face            face,
                          FT_UInt            base_glyph,
                          FT_UInt           *aglyph_index,
                          FT_UInt           *acolor_index,
                          FT_LayerIterator*  iterator );

  FT_LOCAL( FT_Error )
  tt_face_colr_blend_layer( TT_Face       face,
                            FT_UInt       color_index,
                            FT_GlyphSlot  dstSlot,
                            FT_GlyphSlot  srcSlot );


FT_END_HEADER


#endif /* __TTCOLR_H__ */

/* END */
