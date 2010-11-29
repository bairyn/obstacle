/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2000-2009 Darklegion Development

This file is part of Tremulous.

Tremulous is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Tremulous is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Tremulous; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
// tr_font.c
// 
//
// The font system uses FreeType 2.x to render TrueType fonts for use within the game.
// As of this writing ( Nov, 2000 ) Team Arena uses these fonts for all of the ui and 
// about 90% of the cgame presentation. A few areas of the CGAME were left uses the old 
// fonts since the code is shared with standard Q3A.
//
// If you include this font rendering code in a commercial product you MUST include the
// following somewhere with your product, see www.freetype.org for specifics or changes.
// The Freetype code also uses some hinting techniques that MIGHT infringe on patents 
// held by apple so be aware of that also.
//
// As of Q3A 1.25+ and Team Arena, we are shipping the game with the font rendering code
// disabled. This removes any potential patent issues and it keeps us from having to 
// distribute an actual TrueTrype font which is 1. expensive to do and 2. seems to require
// an act of god to accomplish. 
//
// What we did was pre-render the fonts using FreeType ( which is why we leave the FreeType
// credit in the credits ) and then saved off the glyph data and then hand touched up the 
// font bitmaps so they scale a bit better in GL.
//
// There are limitations in the way fonts are saved and reloaded in that it is based on 
// point size and not name. So if you pre-render Helvetica in 18 point and Impact in 18 point
// you will end up with a single 18 point data file and image set. Typically you will want to 
// choose 3 sizes to best approximate the scaling you will be doing in the ui scripting system
// 
// In the UI Scripting code, a scale of 1.0 is equal to a 48 point font. In Team Arena, we
// use three or four scales, most of them exactly equaling the specific rendered size. We 
// rendered three sizes in Team Arena, 12, 16, and 20. 
//
// To generate new font data you need to go through the following steps.
// 1. delete the fontImage_x_xx.tga files and fontImage_xx.dat files from the fonts path.
// 2. in a ui script, specificy a font, smallFont, and bigFont keyword with font name and 
//    point size. the original TrueType fonts must exist in fonts at this point.
// 3. run the game, you should see things normally.
// 4. Exit the game and there will be three dat files and at least three tga files. The 
//    tga's are in 256x256 pages so if it takes three images to render a 24 point font you 
//    will end up with fontImage_0_24.tga through fontImage_2_24.tga
// 5. You will need to flip the tga's in Photoshop as the tga output code writes them upside
//    down.
// 6. In future runs of the game, the system looks for these images and data files when a s
//    specific point sized font is rendered and loads them for use. 
// 7. Because of the original beta nature of the FreeType code you will probably want to hand
//    touch the font bitmaps.
// 
// Currently a define in the project turns on or off the FreeType code which is currently 
// defined out. To pre-render new fonts you need enable the define ( BUILD_FREETYPE ) and 
// uncheck the exclude from build check box in the FreeType2 area of the Renderer project. 


#include "tr_local.h"
#include "../qcommon/qcommon.h"

#ifdef BUILD_FREETYPE

#include <ft2build.h>
#include <freetype/fterrors.h>
#include <freetype/ftsystem.h>
#include <freetype/ftimage.h>
#include <freetype/freetype.h>
#include <freetype/ftoutln.h>

#define _FLOOR(x)  ((x) & -64)
#define _CEIL(x)   (((x)+63) & -64)
#define _TRUNC(x)  ((x) >> 6)

FT_Library ftLibrary = NULL;  
#endif

#define MAX_FONTS 12
static int registeredFontCount = 0;
static fontInfo_t registeredFont[MAX_FONTS];

#ifdef BUILD_FREETYPE
void R_GetGlyphInfo(FT_GlyphSlot glyph, int *left, int *right, int *width, int *top, int *bottom, int *height, int *pitch) {

  *left  = _FLOOR( glyph->metrics.horiBearingX );
  *right = _CEIL( glyph->metrics.horiBearingX + glyph->metrics.width );
  *width = _TRUNC(*right - *left);
    
  *top    = _CEIL( glyph->metrics.horiBearingY );
  *bottom = _FLOOR( glyph->metrics.horiBearingY - glyph->metrics.height );
  *height = _TRUNC( *top - *bottom );
  *pitch  = ( qtrue ? (*width+3) & -4 : (*width+7) >> 3 );
}


FT_Bitmap *R_RenderGlyph(FT_GlyphSlot glyph, glyphInfo_t* glyphOut) {

  FT_Bitmap  *bit2;
  int left, right, width, top, bottom, height, pitch, size;

  R_GetGlyphInfo(glyph, &left, &right, &width, &top, &bottom, &height, &pitch);

  if ( glyph->format == ft_glyph_format_outline ) {
    size   = pitch*height; 

    bit2 = Z_Malloc(sizeof(FT_Bitmap));

    bit2->width      = width;
    bit2->rows       = height;
    bit2->pitch      = pitch;
    bit2->pixel_mode = ft_pixel_mode_grays;
    //bit2->pixel_mode = ft_pixel_mode_mono;
    bit2->buffer     = Z_Malloc(pitch*height);
    bit2->num_grays = 256;

    Com_Memset( bit2->buffer, 0, size );

    FT_Outline_Translate( &glyph->outline, -left, -bottom );

    FT_Outline_Get_Bitmap( ftLibrary, &glyph->outline, bit2 );

    glyphOut->height = height;
    glyphOut->pitch = pitch;
    glyphOut->top = (glyph->metrics.horiBearingY >> 6) + 1;
    glyphOut->bottom = bottom;
    return bit2;
  }
  else {
    ri.Printf(PRINT_ALL, "Non-outline fonts are not supported\n");
  }
  return NULL;
}

static void WriteTGA(char *filename, byte * data, int width, int height)
{
	byte	*buffer;
	int		i, c;

	buffer = Z_Malloc(width*height*4 + 18);

	Com_Memset (buffer, 0, 18);
	buffer[2] = 2;		// uncompressed type
	buffer[12] = width&255;
	buffer[13] = width>>8;
	buffer[14] = height&255;
	buffer[15] = height>>8;
	buffer[16] = 32;	// pixel size

	// swap rgb to bgr
	c = 18 + width * height * 4;
	for (i=18 ; i<c ; i+=4)
	{
		buffer[i] = data[i-18+2];		// blue
		buffer[i+1] = data[i-18+1];		// green
		buffer[i+2] = data[i-18+0];		// red
		buffer[i+3] = data[i-18+3];		// alpha
	}

	// Tr3B: flip upside down
	{
		int             row;
		unsigned char  *flip;
		unsigned char  *src, *dst;

		flip = (unsigned char *)malloc(width * 4);
		for(row = 0; row < height / 2; row++)
		{
			src = buffer + 18 + (row * 4 * width);
			dst = buffer + 18 + ((height - row - 1) * 4 * width);

			memcpy(flip, src, width * 4);
			memcpy(src, dst, width * 4);
			memcpy(dst, flip, width * 4);
		}
		free(flip);
	}

	ri.FS_WriteFile(filename, buffer, c);

	Z_Free (buffer);
}

static glyphInfo_t *RE_ConstructGlyphInfo(int max, unsigned char *imageOut, int *xOut, int *yOut, int *maxHeight, FT_Face face, const unsigned long c, qboolean calcHeight) {
  int i;
  static glyphInfo_t glyph;
  unsigned char *src, *dst;
  float scaled_width, scaled_height;
  FT_Bitmap *bitmap = NULL;

  Com_Memset(&glyph, 0, sizeof(glyphInfo_t));
  // make sure everything is here
  if (face != NULL) {
    FT_UInt ci = FT_Get_Char_Index( face, c );
    if( !ci )
    {
      ri.Printf(PRINT_ALL, "Unrecognized character\n");
      return &glyph;
    }
    FT_Load_Glyph(face, ci, FT_LOAD_DEFAULT );
    bitmap = R_RenderGlyph(face->glyph, &glyph);
    if (bitmap) {
      glyph.xSkip = (face->glyph->metrics.horiAdvance >> 6) + 1;
    } else {
      return &glyph;
    }

    if (glyph.height > *maxHeight) {
      *maxHeight = glyph.height;
    }

    if (calcHeight) {
      Z_Free(bitmap->buffer);
      Z_Free(bitmap);
      return &glyph;
    }

/*
    // need to convert to power of 2 sizes so we do not get 
    // any scaling from the gl upload
  	for (scaled_width = 1 ; scaled_width < glyph.pitch ; scaled_width<<=1)
	  	;
  	for (scaled_height = 1 ; scaled_height < glyph.height ; scaled_height<<=1)
	  	;
*/

    scaled_width = glyph.pitch;
    scaled_height = glyph.height;

    // we need to make sure we fit
    if (*xOut + scaled_width + 1 >= max - 1) {
      if (*yOut + *maxHeight + 1 >= max - 1) {
        *yOut = -1;
        *xOut = -1;
        Z_Free(bitmap->buffer);
        Z_Free(bitmap);
        return &glyph;
      } else {
        *xOut = 0;
        *yOut += *maxHeight + 1;
      }
    } else if (*yOut + *maxHeight + 1 >= max - 1) {
      *yOut = -1;
      *xOut = -1;
      Z_Free(bitmap->buffer);
      Z_Free(bitmap);
      return &glyph;
    }


    src = bitmap->buffer;
    dst = imageOut + (*yOut * max) + *xOut;

		if (bitmap->pixel_mode == ft_pixel_mode_mono) {
			for (i = 0; i < glyph.height; i++) {
				int j;
				unsigned char *_src = src;
				unsigned char *_dst = dst;
				unsigned char mask = 0x80;
				unsigned char val = *_src;
				for (j = 0; j < glyph.pitch; j++) {
					if (mask == 0x80) {
						val = *_src++;
					}
					if (val & mask) {
						*_dst = 0xff;
					}
					mask >>= 1;
        
					if ( mask == 0 ) {
						mask = 0x80;
					}
					_dst++;
				}

				src += glyph.pitch;
				dst += max;

			}
		} else {
	    for (i = 0; i < glyph.height; i++) {
		    Com_Memcpy(dst, src, glyph.pitch);
			  src += glyph.pitch;
				dst += max;
	    }
		}

    // we now have an 8 bit per pixel grey scale bitmap 
    // that is width wide and pf->ftSize->metrics.y_ppem tall

    glyph.imageHeight = scaled_height;
    glyph.imageWidth = scaled_width;
    glyph.s = (float)*xOut / max;
    glyph.t = (float)*yOut / max;
    glyph.s2 = glyph.s + (float)scaled_width / max;
    glyph.t2 = glyph.t + (float)scaled_height / max;

    *xOut += scaled_width + 1;
  }

  Z_Free(bitmap->buffer);
  Z_Free(bitmap);

  return &glyph;
}
#endif

static int fdOffset;
static byte	*fdFile;

int readInt( void ) {
	int i = fdFile[fdOffset]+(fdFile[fdOffset+1]<<8)+(fdFile[fdOffset+2]<<16)+(fdFile[fdOffset+3]<<24);
	fdOffset += 4;
	return i;
}

typedef union {
	byte	fred[4];
	float	ffred;
} poor;

float readFloat( void ) {
	poor	me;
#if defined Q3_BIG_ENDIAN
	me.fred[0] = fdFile[fdOffset+3];
	me.fred[1] = fdFile[fdOffset+2];
	me.fred[2] = fdFile[fdOffset+1];
	me.fred[3] = fdFile[fdOffset+0];
#elif defined Q3_LITTLE_ENDIAN
	me.fred[0] = fdFile[fdOffset+0];
	me.fred[1] = fdFile[fdOffset+1];
	me.fred[2] = fdFile[fdOffset+2];
	me.fred[3] = fdFile[fdOffset+3];
#endif
	fdOffset += 4;
	return me.ffred;
}

#ifdef BUILD_FREETYPE

static unsigned long stream_read(FT_Stream stream, unsigned long offset, unsigned char *buffer, unsigned long count )
{
  if( count == 0 )
  {
    int r = FS_Seek( (fileHandle_t) stream->descriptor.value, (long) offset, FS_SEEK_SET );
    if( r == offset )
      return 0;
    else
      return r;
  }

  if( !buffer )
  {
    ri.Printf(PRINT_ALL, "stream_read: buffer is NULL\n");
    return 0;
  }
  if( !stream->descriptor.value )
  {
    ri.Printf(PRINT_ALL, "stream_read: stream->descriptor.value is zero\n");
    return 0;
  }

  if( FS_FTell( ( fileHandle_t ) stream->descriptor.value ) != (long) offset )
    FS_Seek( (fileHandle_t) stream->descriptor.value, (long) offset,  FS_SEEK_SET );

  return FS_Read( (char *) buffer, (long) count, (fileHandle_t) stream->descriptor.value );
}

static void stream_close(FT_Stream stream)
{
  FS_FCloseFile( (fileHandle_t) stream->descriptor.value );
}

static void *memory_alloc(FT_Memory memory, long size)
{
  return malloc( size );
}

static void memory_free(FT_Memory memory, void *block)
{
  free( block );
}

static void *memory_realloc(FT_Memory memory, long cur_size, long new_size, void *block)
{
  return realloc( block, new_size );
}

void RE_LoadFace(const char *fileName, int pointSize, const char *name, face_t *face)
{
  fileHandle_t h;
  static struct FT_MemoryRec_ memory =
  { NULL
  , memory_alloc
  , memory_free
  , memory_realloc
  };
  FT_Stream stream;
  FT_Open_Args oa =
  { FT_OPEN_STREAM
  , NULL
  , 0L
  , NULL
  , NULL  // stream
  , NULL  // driver
  , 0
  , NULL
  };
  FT_Face ftFace;
	float dpi = 72.0f;
	float glyphScale =  72.0f / dpi; 		// change the scale to be relative to 1 based on 72 dpi ( so dpi of 144 means a scale of .5 )
  int ec;

  if( !face || !fileName || !name )
    return;

  if( FS_ReadFile( fileName, NULL ) <= 0 )
    return;

  oa.stream = face->mem = stream = malloc(sizeof(*stream));

  FS_FOpenFileRead(fileName, &h, qtrue);
  stream->base = NULL;
  stream->size = ri.FS_ReadFile(fileName, NULL);
  stream->pos = 0;
  stream->descriptor.value = h;
  stream->pathname.pointer = (void *) fileName;
  stream->read = (FT_Stream_IoFunc) stream_read;
  stream->close = (FT_Stream_CloseFunc) stream_close;
  stream->memory = &memory;
  stream->cursor = NULL;
  stream->limit = NULL;

  if (ftLibrary == NULL) {
    ri.Printf(PRINT_ALL, "RE_LoadFace: FreeType not initialized.\n");
    return;
  }

  if( ( ec = FT_Open_Face( ftLibrary, &oa, 0, (FT_Face *) &( face->opaque ) ) ) != 0 )
  {
    ri.Printf(PRINT_ALL, "RE_LoadFace: FreeType2, Unable to open face; error code: %d\n", ec);
    return;
  }

  ftFace = (FT_Face) face->opaque;

  if( !ftFace )
  {
    ri.Printf(PRINT_ALL, "RE_LoadFace: face handle is NULL");
    return;
  }

	if (pointSize <= 0) {
		pointSize = 12;
	}
	// we also need to adjust the scale based on point size relative to 48 points as the ui scaling is based on a 48 point font
	glyphScale *= 48.0f / pointSize;

  face->glyphScale = glyphScale;
  strncpy(face->name, name, MAX_QPATH);

  if((ec = FT_Set_Char_Size( ftFace, pointSize << 6, pointSize << 6, dpi, dpi)) != 0)
  {
    ri.Printf(PRINT_ALL, "RE_LoadFace: FreeType2, Unable to set face char size; error code: %d\n", ec);
    return;
  }
}
void RE_FreeFace(face_t *face)
{
  FT_Face ftFace;

  if( !face )
    return;

  ftFace = face->opaque;

  if( !ftFace )
  {
    if( face->mem )
    {
      free( face->mem );
      face->mem = NULL;
    }

	  return;
  }

  if( ftLibrary )
  {
    FT_Done_Face( (FT_Face) ftFace );
    face->opaque = NULL;
  }

  if( face->mem )
  {
    free( face->mem );
    face->mem = NULL;
  }
}

void RE_LoadGlyph(face_t *face, const char *str, int img, glyphInfo_t *glyphInfo)
{
  FT_Face ftFace = face ? (FT_Face) face->opaque : NULL;
  glyphInfo_t *tmp;
  int i;
  int x = 0, y = 0, maxHeight = 0;
  int left, max, satLevels;
  static char name[MAX_QPATH];
  image_t *image;
  qhandle_t h;
  static unsigned char buf[8*256*256];
  static unsigned char imageBuf[4*256*256];

  if( !face || !ftFace )
    return;

  if( img > MAX_FACE_GLYPHS )
  {
    ri.Printf(PRINT_ALL, "RE_LoadGlyph: img > MAX_FACE_GLYPHS\n");

    return;
  }

  Com_Memset(buf, 0, sizeof(buf));
  tmp = RE_ConstructGlyphInfo(32, buf, &x, &y, &maxHeight, ftFace, Q_UTF8CodePoint( str ), qfalse);
  if( x == -1 || y == -1 )
  {
    Com_Memset(buf, 0, sizeof(buf));
    Com_Memset(imageBuf, 0, sizeof(imageBuf));
    return;
  }
  Com_Memcpy(glyphInfo, tmp, sizeof(glyphInfo_t));

  left = 0;
  max = 0;
  satLevels = 255;
  for(i = 0; i < 32*32; i++)
  {
    if(max < buf[i])
    {
      max = buf[i];
    }
  }

  if(max > 0)
  {
    max = 255/max;
  }

  //for(i = 0; i < (scaledSize); i++)
  for(i = 0; i < (32*32); i++)
  {
    imageBuf[left++] = 255;
    imageBuf[left++] = 255;
    imageBuf[left++] = 255;

    imageBuf[left++] = ((float)buf[i] * max);
  }

  Com_sprintf( name, sizeof(name), "./../._FONT_%d", img );
  //{static int n = 0; Com_sprintf( name, sizeof(name), "./../._FONT_%d", n++ );}

  image = R_CreateImage(name, imageBuf, 32, 32, qfalse, qfalse, GL_CLAMP_TO_EDGE);
  face->images[ img ] = (void *) image;
  h = RE_RegisterShaderFromImage(name, LIGHTMAP_2D, image, qfalse);

  Com_Memset(buf, 0, sizeof(buf));
  Com_Memset(imageBuf, 0, sizeof(imageBuf));

  glyphInfo->glyph = h;
  strncpy( glyphInfo->shaderName, name, sizeof( glyphInfo->shaderName ) );
}
void RE_FreeGlyph(face_t *face, int img, glyphInfo_t *glyphInfo)
{
  image_t *image;

  if( !face || !glyphInfo )
    return;

  if( img > MAX_FACE_GLYPHS )
  {
    ri.Printf(PRINT_ALL, "RE_LoadGlyph: img > MAX_FACE_GLYPHS\n");

    return;
  }

  image = (image_t *) face->images[ img ];

  if( !image )
    return;

  R_FreeImage( image );
  face->images[ img ] = NULL;
}

typedef struct
{ qboolean    used;
  char        str[5];
  glyphInfo_t glyph;
  face_t      *face;
} glyphCache_t;

static glyphCache_t glyphCache[MAX_FACE_GLYPHS] = {{0}}, *nextCache = glyphCache;

void RE_Glyph( fontInfo_t *font, face_t *face, const char *str, glyphInfo_t *glyph )
{
  int    i;
  int    width;

  if( !str || !*str || !face || ( width = Q_UTF8Width( str ) ) <= 1 )
  {
    memcpy( glyph, &font->glyphs[ (int)*str ], sizeof( *glyph ) );
    return;
  }

  for( i = 0; i < MAX_FACE_GLYPHS; i++ )
  {
    glyphCache_t *c = &glyphCache[ i ];

    if( c->used && c->face == face )
    {
      if( !face->images[ c - glyphCache ] )
      {
        // was freed
        c->used = qfalse;
      }
      else
      {
        const char *s, *cs;

        for( s = str, cs = c->str; *s && *cs && *s == *cs && s - str < width ; s++, cs++ );

        if( !*cs && s - str == width)
        {
          memcpy( glyph, &c->glyph, sizeof( *glyph ) );
          return;
        }
      }
    }
  }

  if( nextCache->used )
  {
	  RE_FreeGlyph( nextCache->face, nextCache - glyphCache, &nextCache->glyph );
  }

  RE_LoadGlyph( face, str, nextCache - glyphCache, &nextCache->glyph );
  nextCache->face = face;

  strncpy( nextCache->str, str, width );  // This should never cause an overflow since Q_UTF8Width never returns a width larger than 4
  nextCache->used = qtrue;

  if( ++nextCache - glyphCache >= MAX_FACE_GLYPHS )
    nextCache = glyphCache;

  memcpy( glyph, &nextCache->glyph, sizeof( *glyph ) );
}

void RE_FreeCachedGlyphs( face_t *face )
{
  int i;

  for( i = 0; i < MAX_FACE_GLYPHS; i++ )
  {
    glyphCache_t *c = &glyphCache[ i ];

    if( c->used )
    {
      if( c->face == face )
      {
        c->used = qfalse;

        RE_FreeGlyph( face, i, &c->glyph );
      }
    }
  }
}

static void RE_ClearGlyphCache( void )
{
  int i;

  for( i = 0; i < MAX_FACE_GLYPHS; i++ )
  {
    glyphCache_t *c = &glyphCache[ i ];

    if( c->used )
    {
      c->used = qfalse;

      RE_FreeGlyph( c->face, i, &c->glyph );
    }
  }
}
#else
void RE_LoadFace(const char *fileName, int pointSize, const char *name, face_t *face)
{
}
void RE_FreeFace(face_t *face)
{
}
void RE_LoadGlyph(face_t *face, const char *str, glyphInfo_t *glyphInfo)
{
}
void RE_FreeGlyph(face_t *face, glyphInfo_t *glyphInfo)
{
}
void RE_Glyph( fontInfo_t *font, face_t *face, const char *str, glyphInfo_t *glyph )
{
  memcpy( glyph, &font->glyphs[ (int)*str ], sizeof( *glyph ) );
}
void RE_FreeCachedGlyphs( face_t *face )
{
}
static void RE_ClearGlyphCache( void )
{
}
#endif

void RE_RegisterFont(const char *fontName, int pointSize, fontInfo_t *font) {
#ifdef BUILD_FREETYPE
  FT_Face face;
  int j, k, xOut, yOut, lastStart, imageNumber;
  int scaledSize, newSize, maxHeight, left, satLevels;
  unsigned char *out, *imageBuff;
  glyphInfo_t *glyph;
  image_t *image;
  qhandle_t h;
	float max;
#endif
  void *faceData;
	int i, len;
	char  name[MAX_QPATH];
	char  strippedName[MAX_QPATH];
	float dpi = 72;											//
	float glyphScale =  72.0f / dpi; 		// change the scale to be relative to 1 based on 72 dpi ( so dpi of 144 means a scale of .5 )


  if (!fontName) {
    ri.Printf(PRINT_ALL, "RE_RegisterFont: called with empty name\n");
    return;
  }

	if (pointSize <= 0) {
		pointSize = 12;
	}
	// we also need to adjust the scale based on point size relative to 48 points as the ui scaling is based on a 48 point font
	glyphScale *= 48.0f / pointSize;

	// make sure the render thread is stopped
	R_SyncRenderThread();

  if (registeredFontCount >= MAX_FONTS) {
    ri.Printf(PRINT_ALL, "RE_RegisterFont: Too many fonts registered already.\n");
    return;
  }

	COM_StripExtension(fontName, strippedName, sizeof(strippedName));
    //Compatibility: If there is no extension, assume this is loading one of the legacy fonts
    if(!Q_stricmp(strippedName, fontName))
    {
        Com_sprintf(name, sizeof(name), "fonts/fontImage_%i.dat",pointSize);
    }
    else
    {
        Com_sprintf(name, sizeof(name), "%s_%i.dat", strippedName, pointSize);
    }

    for (i = 0; i < registeredFontCount; i++) {
        if (Q_stricmp(name, registeredFont[i].name) == 0) {
            Com_Memcpy(font, &registeredFont[i], sizeof(fontInfo_t));
            return;
        }
    }

	len = ri.FS_ReadFile(name, NULL);
	if (len == sizeof(fontInfo_t)) {
		ri.FS_ReadFile(name, &faceData);
		fdOffset = 0;
		fdFile = faceData;
		for(i=0; i<GLYPHS_PER_FONT; i++) {
			font->glyphs[i].height		= readInt();
			font->glyphs[i].top			= readInt();
			font->glyphs[i].bottom		= readInt();
			font->glyphs[i].pitch		= readInt();
			font->glyphs[i].xSkip		= readInt();
			font->glyphs[i].imageWidth	= readInt();
			font->glyphs[i].imageHeight = readInt();
			font->glyphs[i].s			= readFloat();
			font->glyphs[i].t			= readFloat();
			font->glyphs[i].s2			= readFloat();
			font->glyphs[i].t2			= readFloat();
			font->glyphs[i].glyph		= readInt();
			Com_Memcpy(font->glyphs[i].shaderName, &fdFile[fdOffset], 32);
			fdOffset += 32;
		}
		font->glyphScale = readFloat();
		Com_Memcpy(font->name, &fdFile[fdOffset], MAX_QPATH);

//		Com_Memcpy(font, faceData, sizeof(fontInfo_t));
		Q_strncpyz(font->name, name, sizeof(font->name));
		for (i = GLYPH_START; i < GLYPH_END; i++) {
			font->glyphs[i].glyph = RE_RegisterShaderNoMip(font->glyphs[i].shaderName);
		}
	  Com_Memcpy(&registeredFont[registeredFontCount++], font, sizeof(fontInfo_t));
		return;
	}

#ifndef BUILD_FREETYPE
    ri.Printf(PRINT_ALL, "RE_RegisterFont: FreeType code not available\n");
#else
  if (ftLibrary == NULL) {
    ri.Printf(PRINT_ALL, "RE_RegisterFont: FreeType not initialized.\n");
    return;
  }

  Com_sprintf(name, sizeof(name), "%s", fontName);

  len = ri.FS_ReadFile(fontName, &faceData);
  if (len <= 0) {
    ri.Printf(PRINT_ALL, "RE_RegisterFont: Unable to read font file %s\n", name);
    return;
  }

  // allocate on the stack first in case we fail
  if (FT_New_Memory_Face( ftLibrary, faceData, len, 0, &face )) {
    ri.Printf(PRINT_ALL, "RE_RegisterFont: FreeType2, unable to allocate new face.\n");
    return;
  }


  if (FT_Set_Char_Size( face, pointSize << 6, pointSize << 6, dpi, dpi)) {
    ri.Printf(PRINT_ALL, "RE_RegisterFont: FreeType2, Unable to set face char size.\n");
    return;
  }


  //*font = &registeredFonts[registeredFontCount++];

  // make a 256x256 image buffer, once it is full, register it, clean it and keep going 
  // until all glyphs are rendered

  out = Z_Malloc(1024*1024);
  if (out == NULL) {
    ri.Printf(PRINT_ALL, "RE_RegisterFont: Z_Malloc failure during output image creation.\n");
    return;
  }
  Com_Memset(out, 0, 1024*1024);

  maxHeight = 0;

  for (i = GLYPH_START; i < GLYPH_END; i++) {
    glyph = RE_ConstructGlyphInfo(256, out, &xOut, &yOut, &maxHeight, face, (unsigned char)i, qtrue);
  }

  xOut = 0;
  yOut = 0;
  i = GLYPH_START;
  lastStart = i;
  imageNumber = 0;

  while(i <= GLYPH_END)
  {
	  glyph = RE_ConstructGlyphInfo(256, out, &xOut, &yOut, &maxHeight, face, (unsigned char)i, qfalse);

	  if(xOut == -1 || yOut == -1 || i == GLYPH_END)
	  {
		  if(xOut == -1 || yOut == -1)
		  {
			  //ri.Printf(PRINT_WARNING, "RE_RegisterFont: character %c does not fit image number %i\n", (unsigned char) i, imageNumber);
		  }

		  // ran out of room
		  // we need to create an image from the bitmap, set all the handles in the glyphs to this point
		  scaledSize = 256*256;
		  newSize = scaledSize * 4;
		  imageBuff = Z_Malloc(newSize);
		  left = 0;
		  max = 0;
		  satLevels = 255;
		  for(k = 0; k < (scaledSize); k++)
		  {
			  if(max < out[k])
			  {
				  max = out[k];
			  }
		  }

		  if(max > 0)
		  {
			  max = 255/max;
		  }

		  for(k = 0; k < (scaledSize); k++)
		  {
			  imageBuff[left++] = 255;
			  imageBuff[left++] = 255;
			  imageBuff[left++] = 255;

			  imageBuff[left++] = ((float)out[k] * max);
		  }

		  Com_sprintf(name, sizeof(name), "%s_%i_%i.tga", strippedName, imageNumber++, pointSize);
		  if(!ri.FS_FileExists(name) && r_saveFontData->integer)  
		  {
			  WriteTGA(name, imageBuff, 256, 256);
		  }

		  /*`image = R_CreateImage(name, imageBuff, 256, 256, IF_NOPICMIP, FT_LINEAR, WT_CLAMP);`*/
		  image = R_CreateImage(name, imageBuff, 256, 256, qfalse, qfalse, GL_CLAMP_TO_EDGE);
		  /*`h = RE_RegisterShaderFromImage(name, image, qfalse);`*/
		  h = RE_RegisterShaderFromImage(name, LIGHTMAP_2D, image, qfalse);

		  Com_Memset(out, 0, 1024*1024);
		  xOut = 0;
		  yOut = 0;
		  Z_Free(imageBuff);

		  if(i == GLYPH_END)
		  {
			  for(j = lastStart; j <= GLYPH_END; j++)
			  {
				  font->glyphs[j].glyph = h;
				  Q_strncpyz(font->glyphs[j].shaderName, name, sizeof(font->glyphs[j].shaderName));
			  }
			  break;
		  }
		  else
		  {
			  for(j = lastStart; j < i; j++)
			  {
				  font->glyphs[j].glyph = h;
				  Q_strncpyz(font->glyphs[j].shaderName, name, sizeof(font->glyphs[j].shaderName));
			  }
			  lastStart = i;
		  }
	  }
	  else
	  {
		  Com_Memcpy(&font->glyphs[i], glyph, sizeof(glyphInfo_t));
		  i++;
	  }
  }

	registeredFont[registeredFontCount].glyphScale = glyphScale;
	font->glyphScale = glyphScale;

	Com_sprintf(name, sizeof(name), "%s_%i.dat", strippedName, pointSize);
	Q_strncpyz(font->name, name, sizeof(font->name));
	Com_Memcpy(&registeredFont[registeredFontCount++], font, sizeof(fontInfo_t));

	if(!ri.FS_FileExists(name) && r_saveFontData->integer) 
	{
        ri.FS_WriteFile(name, font, sizeof(fontInfo_t));
	}

  Z_Free(out);
  
  ri.FS_FreeFile(faceData);
#endif
}



void R_InitFreeType(void) {
#ifdef BUILD_FREETYPE
  if (FT_Init_FreeType( &ftLibrary )) {
    ri.Printf(PRINT_ALL, "R_InitFreeType: Unable to initialize FreeType.\n");
  }
#endif
  registeredFontCount = 0;
}


void R_DoneFreeType(void) {
#ifdef BUILD_FREETYPE
  if (ftLibrary) {
    RE_ClearGlyphCache();
    FT_Done_FreeType( ftLibrary );
    ftLibrary = NULL;
  }
#endif
	registeredFontCount = 0;
}

