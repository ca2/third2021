#pragma once
#include <obs-module.h>
#define PSAPI_VERSION 1
#define WIN32_LEAN_AND_MEAN
#include <obs.h>
#include <util/dstr.h>
#include <stdlib.h>
#include <util/dstr.h>
#include <stdbool.h>
#include <windows.h>
#include <psapi.h>
#include <util/windows/win-version.h>
#include <util/platform.h>

#include <util/threading.h>

void ca2_plugin_frame_capture(struct bitmap_source * pbitmapsource);
const char * file_path_name(const char * path);
void fill_bitmap_source_list(obs_property_t *p);
#pragma once

#include <stdint.h>
#include <inttypes.h>

#if defined(_UNICODE)
#define _T(x) L ##x
#else
#define _T(x) x
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
CHAR szFrameCaptureFolder[] = "%s\\ca2\\bitmap-source";
CHAR szFrameCaptureFile[] = "%s\\ca2\\bitmap-source\\%s";
CHAR szFrameCaptureMutex[] = "Local\\ca2-bitmap-source:%s";
#pragma comment(lib, "psapi.lib")
#else
char szName[] = "$HOME/.ca2/ca2screen-%" PRIu64;
char szNameMutex[] = "$HOME/.ca2/ca2screen-%" PRIu64 "-mutex";
#endif


const char * file_path_name(const char * path)
{

	const char * name1 = strrchr(path, '\\');

	const char * name2 = strrchr(path, '/');

	if (!name1)
	{

		if (!name2)
		{

			return path;

		}
		else
		{

			return name2 + 1;

		}

	}
	else
	{

		if (name1 > name2) // || !name2 not needed
		{

			return name1 + 1;

		}
		else
		{

			return name2 + 1;

		}

	}

}


/* this is a workaround to A/Vs going crazy whenever certain functions (such as
* OpenProcess) are used */
extern void *get_obfuscated_func(HMODULE module, const char *str, uint64_t val);

#ifdef __cplusplus
}
#endif

#define NUM_TEXTURES 2

struct bitmap_source_buffer
{

   int          cur_tex;
   gs_texture_t *textures[NUM_TEXTURES];
   bool         textures_written[NUM_TEXTURES];
   int          x, y;
   uint32_t     width;
   uint32_t     height;
   int          num_textures;

   bool         compatibility;
   HDC          hdc;
   HBITMAP      bmp, old_bmp;
   BYTE         *bits;

   bool         valid;
   COLORREF *   pcolorref;

};


struct bitmap_source
{

   obs_source_t *	               source;
   obs_property_t *              plist;

   bool                          compatibility;

   struct bitmap_source_buffer   m_buffer;

   float                         resize_timer;

   char *		                  m_pszNameNew;
   RECT                          last_rect;
   pthread_mutex_t               m_mutex;


   int64_t                       m_iWidth;
   int64_t                       m_iHeight;
   int64_t                       m_iScan;
   uint32_t *                    m_pBitmapData;

   HANDLE                        m_hmutex;
   char *		                  m_pszName;
   HANDLE                        m_hfile;
   HANDLE                        m_hMapFile;
   void *	                     m_pMemoryMap;


};



extern void bitmap_source_buffer_init(struct bitmap_source * pbitmapsource, struct bitmap_source_buffer *pbuffer, int x, int y,
                            uint32_t width, uint32_t height, bool compatibility);
extern void bitmap_source_buffer_free(struct bitmap_source_buffer *pbuffer);


extern void bitmap_source_buffer_capture(struct bitmap_source * pbitmapsource, struct bitmap_source_buffer *pbuffer, HWND window);
extern void bitmap_source_buffer_render(struct bitmap_source_buffer *pbuffer, gs_effect_t *effect);


#include <util/dstr.h>


static obs_source_t * get_transition(struct bitmap_source *ss)
{

   obs_source_t *tr;

   pthread_mutex_lock(&ss->m_mutex);

   tr = ss->source;

   obs_source_addref(tr);

   pthread_mutex_unlock(&ss->m_mutex);

   return tr;

}


#define TEXT_BITMAP_SOURCE obs_module_text("BitmapSource")
#define TEXT_BITMAP_SOURCE_BITMAP_SOURCE         obs_module_text("BitmapSource.BitmapSource")


static inline void init_textures(struct bitmap_source_buffer *pbuffer)
{

   for (int i = 0; i < pbuffer->num_textures; i++)
   {

      if (pbuffer->compatibility)
      {

         pbuffer->textures[i] = gs_texture_create(pbuffer->width, pbuffer->height, GS_BGRA, 1, NULL, GS_DYNAMIC);

      }
      else
      {

         pbuffer->textures[i] = gs_texture_create_gdi(pbuffer->width, pbuffer->height);

      }

      if (!pbuffer->textures[i])
      {

         blog(LOG_WARNING, "[bitmap_source_buffer_init] Failed to create textures");

         return;

      }

   }

   pbuffer->valid = true;

}


void bitmap_source_buffer_init(struct bitmap_source * pbitmapsource, struct bitmap_source_buffer * pbuffer, int x, int y, uint32_t width, uint32_t height, bool compatibility)
{

   memset(pbuffer, 0, sizeof(struct bitmap_source_buffer));

   pbuffer->x = x;
   pbuffer->y = y;
   pbuffer->width = width;
   pbuffer->height = height;

   obs_enter_graphics();

   compatibility = true;

   pbuffer->compatibility = compatibility;
   pbuffer->num_textures = compatibility ? 1 : 2;

   init_textures(pbuffer);

   obs_leave_graphics();

   if (!pbuffer->valid)
   {

      return;

   }

   if (compatibility)
   {

      BITMAPINFO bi = { 0 };
      BITMAPINFOHEADER *bih = &bi.bmiHeader;
      bih->biSize = sizeof(BITMAPINFOHEADER);
      bih->biBitCount = 32;
      bih->biWidth = width;
      bih->biHeight = height;
      bih->biPlanes = 1;

      pbuffer->hdc = CreateCompatibleDC(NULL);
      pbuffer->bmp = CreateDIBSection(pbuffer->hdc, &bi,
                                      DIB_RGB_COLORS, (void**)&pbuffer->bits,
                                      NULL, 0);
      pbuffer->pcolorref = (COLORREF *)malloc(width * height * 4);
      pbuffer->old_bmp = SelectObject(pbuffer->hdc, pbuffer->bmp);

   }

}


void bitmap_source_buffer_free(struct bitmap_source_buffer *pbuffer)
{

   if (pbuffer->hdc)
   {

      free(pbuffer->pcolorref);
      SelectObject(pbuffer->hdc, pbuffer->old_bmp);
      DeleteDC(pbuffer->hdc);
      DeleteObject(pbuffer->bmp);

   }

   obs_enter_graphics();

   for (int i = 0; i < pbuffer->num_textures; i++)
   {

      gs_texture_destroy(pbuffer->textures[i]);

   }

   obs_leave_graphics();

   memset(pbuffer, 0, sizeof(struct bitmap_source_buffer));

}


static inline HDC bitmap_source_buffer_get_dc(struct bitmap_source_buffer *pbuffer)
{

   if (!pbuffer->valid)
   {

      return NULL;

   }

   if (pbuffer->compatibility)
   {

      return pbuffer->hdc;

   }
   else
   {

      return gs_texture_get_dc(pbuffer->textures[pbuffer->cur_tex]);

   }

}


static inline void bitmap_source_buffer_release_dc(struct bitmap_source_buffer *pbuffer)
{

   if (pbuffer->compatibility)
   {

      gs_texture_set_image(pbuffer->textures[pbuffer->cur_tex], pbuffer->bits, pbuffer->width * 4, false);

   }
   else
   {

      gs_texture_release_dc(pbuffer->textures[pbuffer->cur_tex]);

   }

}


static void draw_texture(struct bitmap_source_buffer *pbuffer, int id, gs_effect_t * effect)
{

   gs_texture_t * texture = pbuffer->textures[id];

   gs_technique_t * tech = gs_effect_get_technique(effect, "Draw");

   gs_eparam_t * image = gs_effect_get_param_by_name(effect, "image");

   size_t passes;

   gs_effect_set_texture(image, texture);

   passes = gs_technique_begin(tech);

   for (size_t i = 0; i < passes; i++)
   {

      if (gs_technique_begin_pass(tech, i))
      {

         if (pbuffer->compatibility)
         {

            gs_blend_state_push();
            // gs_blend_function_separate(GS_BLEND_ONE, GS_BLEND_INVSRCALPHA, GS_BLEND_ONE, GS_BLEND_INVSRCALPHA);
            gs_blend_function(GS_BLEND_SRCALPHA, GS_BLEND_INVSRCALPHA);
            // gs_blend_function(GS_BLEND_SRCALPHA, GS_BLEND_SRCALPHA);
            // gs_blend_function_separate(GS_BLEND_SRCALPHA, GS_BLEND_ONE, GS_BLEND_SRCALPHA, GS_BLEND_INVSRCALPHA);
            gs_enable_blending(true);
            // gs_reset_blend_state();
            gs_draw_sprite(texture, GS_FLIP_V, 0, 0);
            gs_blend_state_pop();

         }
         else
         {

            gs_draw_sprite(texture, 0, 0, 0);

         }

         gs_technique_end_pass(tech);

      }

   }

   gs_technique_end(tech);

}


void bitmap_source_buffer_render(struct bitmap_source_buffer *pbuffer, gs_effect_t *effect)
{

   int last_tex = (pbuffer->cur_tex > 0) ? pbuffer->cur_tex - 1 : pbuffer->num_textures - 1;

   if (!pbuffer->valid)
   {

      return;

   }

   if (pbuffer->textures_written[last_tex])
   {

      draw_texture(pbuffer, last_tex, effect);

   }

}


static void update_settings(struct bitmap_source *pbitmapsource, obs_data_t *s)
{

   const char * pszBitmapSource = obs_data_get_string(s, "bitmap-source");

   char * pszNameNewOld = pbitmapsource->m_pszNameNew;

   pbitmapsource->m_pszNameNew = strdup(pszBitmapSource);

   if(pszNameNewOld)
   {

	   free(pszNameNewOld);

   }

}


static void bitmap_source_update(void *data, obs_data_t *settings)
{

   struct bitmap_source * pbitmapsource = data;

   update_settings(pbitmapsource, settings);

}


static uint32_t bitmap_source_width(void *data)
{

   struct bitmap_source *pbitmapsource = data;

   return pbitmapsource->m_buffer.width;

}


static uint32_t bitmap_source_height(void *data)
{

   struct bitmap_source *pbitmapsource = data;

   return pbitmapsource->m_buffer.height;

}


static obs_properties_t *bitmap_source_properties(void *unused)
{

   UNUSED_PARAMETER(unused);

   obs_properties_t *ppts = obs_properties_create();

   obs_property_t *p;

   p = obs_properties_add_list(ppts, "bitmap-source", TEXT_BITMAP_SOURCE_BITMAP_SOURCE, OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);

   fill_bitmap_source_list(p);

   return ppts;

}


#define RESIZE_CHECK_TIME 0.2f


static void bitmap_source_tick(void *data, float seconds)
{

   struct bitmap_source * pbitmapsource = data;

   bool reset_capture = false;

   if (!obs_source_showing(pbitmapsource->source))
   {

      return;

   }

   if (!pbitmapsource->m_pszNameNew)
   {

	   return;

   }

   if (pbitmapsource->m_pszNameNew && (!pbitmapsource->m_pszName || strcmp(pbitmapsource->m_pszName, pbitmapsource->m_pszNameNew)))
   {

      if(pbitmapsource->m_pszName)
      {

         free(pbitmapsource->m_pszName);

      }

      pbitmapsource->m_pszName = strdup(pbitmapsource->m_pszNameNew);

      if (pbitmapsource->m_hmutex != NULL)
      {

         if (WaitForSingleObject(pbitmapsource->m_hmutex, INFINITE) == WAIT_OBJECT_0)
         {

            if (pbitmapsource->m_pMemoryMap != NULL)
            {

               UnmapViewOfFile(pbitmapsource->m_pMemoryMap);

               pbitmapsource->m_pMemoryMap = NULL;

            }

            if (pbitmapsource->m_hMapFile != NULL)
            {

               CloseHandle(pbitmapsource->m_hMapFile);

               pbitmapsource->m_hMapFile = NULL;

            }

            ReleaseMutex(pbitmapsource->m_hmutex);

         }

         CloseHandle(pbitmapsource->m_hmutex);

      }

      char szNameMutex2[2048];

      sprintf(szNameMutex2, szFrameCaptureMutex, pbitmapsource->m_pszName);

      pbitmapsource->m_hmutex = CreateMutexA(NULL, FALSE, szNameMutex2);

      if (WaitForSingleObject(pbitmapsource->m_hmutex, INFINITE) == WAIT_OBJECT_0)
      {

         char szName2[2048];

         const char * pszFolder = NULL;

#ifdef WIN32

         pszFolder = getenv("APPDATA");

#else

         pszFolder*******;

#endif

         sprintf(szName2, szFrameCaptureFile, pszFolder, pbitmapsource->m_pszName);

         pbitmapsource->m_hfile = CreateFileA(szName2, FILE_READ_DATA, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

         if (pbitmapsource->m_hfile != INVALID_HANDLE_VALUE)
         {

            pbitmapsource->m_hMapFile = CreateFileMappingA(
               pbitmapsource->m_hfile,    // use paging file
               NULL,                    // default security
               PAGE_READONLY,          // read/write access
               0,                       // maximum object size (high-order DWORD)
               8192 * 4096 * 4,                // maximum object size (low-order DWORD)
               NULL);                 // name of mapping object

         }

         if (pbitmapsource->m_hMapFile == NULL)
         {

            ReleaseMutex(pbitmapsource->m_hmutex);

            CloseHandle(pbitmapsource->m_hmutex);

         }
         else
         {

            pbitmapsource->m_pMemoryMap = (LPTSTR)MapViewOfFile(pbitmapsource->m_hMapFile,   // handle to map object
                                                                FILE_MAP_READ, // read/write permission
                                                                0,
                                                                0,
                                                                8192 * 4096 * 4);

            if (pbitmapsource->m_pMemoryMap == NULL)
            {

               CloseHandle(pbitmapsource->m_hMapFile);

               ReleaseMutex(pbitmapsource->m_hmutex);

               CloseHandle(pbitmapsource->m_hmutex);

            }

         }

         ReleaseMutex(pbitmapsource->m_hmutex);

      }

   }

   if(pbitmapsource->m_pMemoryMap)
   {

      int64_t * pi = (int64_t *) pbitmapsource->m_pMemoryMap;

      pbitmapsource->m_iWidth = *pi++;
      pbitmapsource->m_iHeight = *pi++;
      pbitmapsource->m_iScan = *pi++;
      pbitmapsource->m_pBitmapData = (uint32_t *) pi;

      obs_enter_graphics();

      ca2_plugin_frame_capture(pbitmapsource);

      obs_leave_graphics();

   }

}


static const char *bitmap_source_getname(void *unused)
{

   UNUSED_PARAMETER(unused);

   return TEXT_BITMAP_SOURCE;

}


static void *bitmap_source_create(obs_data_t *settings, obs_source_t *source)
{

   struct bitmap_source *pbitmapsource = bzalloc(sizeof(struct bitmap_source));

   pbitmapsource->source = source;

   pbitmapsource->compatibility = true;

   pthread_mutex_init_value(&pbitmapsource->m_mutex);

   if (pthread_mutex_init(&pbitmapsource->m_mutex, NULL) != 0)
   {

      return NULL;

   }

   update_settings(pbitmapsource, settings);

   return pbitmapsource;

}


static void bitmap_source_destroy(void *data)
{

   struct bitmap_source *pbitmapsource = data;

   if (pbitmapsource)
   {

      obs_enter_graphics();

      bitmap_source_buffer_free(&pbitmapsource->m_buffer);

      obs_leave_graphics();

      if (pbitmapsource->m_hmutex != NULL)
      {

         if (WaitForSingleObject(pbitmapsource->m_hmutex, INFINITE) == WAIT_OBJECT_0)
         {

            if (pbitmapsource->m_pMemoryMap != NULL)
            {

               UnmapViewOfFile(pbitmapsource->m_pMemoryMap);

               pbitmapsource->m_pMemoryMap = NULL;

            }

            if (pbitmapsource->m_hMapFile != NULL)
            {

               CloseHandle(pbitmapsource->m_hMapFile);

               pbitmapsource->m_hMapFile = NULL;

            }

            ReleaseMutex(pbitmapsource->m_hmutex);

         }

         CloseHandle(pbitmapsource->m_hmutex);

      }

      bfree(pbitmapsource);

   }

}


static void bitmap_source_defaults(obs_data_t *defaults)
{

}


static void bitmap_source_render(void *data, gs_effect_t *effect)
{

   struct bitmap_source *pbitmapsource = data;

   bitmap_source_buffer_render(&pbitmapsource->m_buffer, obs_get_base_effect(OBS_EFFECT_PREMULTIPLIED_ALPHA));

   UNUSED_PARAMETER(effect);

}


#include "audio_render.c"

struct obs_source_info bitmap_source_info =
{
   .id = "bitmap_source",
   .type = OBS_SOURCE_TYPE_INPUT,
   .output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_CUSTOM_DRAW | OBS_SOURCE_COMPOSITE,
   .get_name = bitmap_source_getname,
   .create = bitmap_source_create,
   .destroy = bitmap_source_destroy,
   .update = bitmap_source_update,
   .video_render = bitmap_source_render,
   .video_tick = bitmap_source_tick,
   .audio_render = ss_audio_render,


   .get_width = bitmap_source_width,
   .get_height = bitmap_source_height,
   .get_defaults = bitmap_source_defaults,
   .get_properties = bitmap_source_properties
};


void ca2_plugin_frame_capture(struct bitmap_source * pbitmapsource)
{

   struct bitmap_source_buffer * pbuffer = &pbitmapsource->m_buffer;

   HBITMAP bmp = pbuffer->bmp;

   if(pbitmapsource->m_iWidth != pbuffer->width || pbitmapsource->m_iHeight != pbuffer->height)
   {

      bitmap_source_buffer_free(&pbitmapsource->m_buffer);

      bitmap_source_buffer_init(pbitmapsource, &pbitmapsource->m_buffer, 0, 0,
                                (uint32_t) pbitmapsource->m_iWidth,
                                (uint32_t) pbitmapsource->m_iHeight,
                                pbitmapsource->compatibility);

   }

   if (pbitmapsource->m_hmutex != NULL)
   {

      if (WaitForSingleObject(pbitmapsource->m_hmutex, 1) == WAIT_OBJECT_0)
      {

         if (pbitmapsource->m_pMemoryMap != NULL)
         {

            BITMAPINFO bitmapinfo;

            ZeroMemory(&bitmapinfo, sizeof(bitmapinfo));

            bitmapinfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
            bitmapinfo.bmiHeader.biWidth = (LONG)pbitmapsource->m_iWidth;
            bitmapinfo.bmiHeader.biHeight = (LONG)-pbitmapsource->m_iHeight;
            bitmapinfo.bmiHeader.biPlanes = 1;
            bitmapinfo.bmiHeader.biBitCount = 32;
            bitmapinfo.bmiHeader.biCompression = BI_RGB;
            bitmapinfo.bmiHeader.biSizeImage = (LONG)(pbitmapsource->m_iHeight * pbitmapsource->m_iScan);

	         if (++pbuffer->cur_tex >= pbuffer->num_textures)
	         {

		         pbuffer->cur_tex = 0;

	         }

	         HDC hdc = bitmap_source_buffer_get_dc(pbuffer);

	         if (hdc)
            {

               SetDIBits(hdc, bmp, 0, (UINT) pbitmapsource->m_iHeight,
                         pbitmapsource->m_pBitmapData, &bitmapinfo, DIB_RGB_COLORS);

               bitmap_source_buffer_release_dc(pbuffer);

            }

		      pbuffer->textures_written[pbuffer->cur_tex] = true;

	      }

	   }

      ReleaseMutex(pbitmapsource->m_hmutex);

   }

}


static inline void encode_dstr(struct dstr *str)
{

   dstr_replace(str, "#", "#22");

   dstr_replace(str, ":", "#3A");

}


static inline char *decode_str(const char *src)
{

   struct dstr str = { 0 };

   dstr_copy(&str, src);

   dstr_replace(&str, "#3A", ":");

   dstr_replace(&str, "#22", "#");

   return str.array;

}


static void add_frame_capture(obs_property_t *p, const char * name)
{

   struct dstr desc = { 0 };

   dstr_printf(&desc, "%s", name);

   obs_property_list_add_string(p, desc.array, desc.array);

   dstr_free(&desc);

}


void fill_bitmap_source_list(obs_property_t *p)
{

   obs_property_list_clear(p);

   char szFolder[2048];

   const char * pszFolder = NULL;

#ifdef WIN32

   pszFolder = getenv("APPDATA");

#else

   pszHome = getenv("HOME");

#endif

   sprintf(szFolder, szFrameCaptureFolder, pszFolder);

   os_dir_t *dir = os_opendir(szFolder);

   if (!dir)
   {

      return;

   }

   struct dstr selected_path;

   dstr_init(&selected_path);

   struct os_dirent * pdirent = os_readdir(dir);

   while (pdirent)
   {

	   if (!pdirent->directory)
	   {

		   add_frame_capture(p, file_path_name(pdirent->d_name));

	   }

      pdirent = os_readdir(dir);

   }

   dstr_free(&selected_path);

}


OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("bitmap_source", "en-US")

extern struct obs_source_info bitmap_source_info;


/* temporary, will eventually be erased once we figure out how to create both
* 32bit and 64bit versions of the helpers/hook */
#ifdef _WIN64
#define IS32BIT false
#else
#define IS32BIT true
#endif

#define USE_HOOK_ADDRESS_CACHE false

bool obs_module_load(void)
{
   struct win_version_info ver;
   bool win8_or_above = false;
   char *config_dir;

   config_dir = obs_module_config_path(NULL);
   if (config_dir)
   {
      os_mkdirs(config_dir);
      bfree(config_dir);
   }

   get_win_ver(&ver);

   win8_or_above = ver.major > 6 || (ver.major == 6 && ver.minor >= 2);

   obs_enter_graphics();


   obs_leave_graphics();

   obs_register_source(&bitmap_source_info);


   return true;
}



//#define _CRT_SECURE_NO_WARNINGS

#ifdef _MSC_VER
#pragma warning(disable : 4152) /* casting func ptr to void */
#endif

#include <windows.h>

#define LOWER_HALFBYTE(x) ((x) & 0xF)
#define UPPER_HALFBYTE(x) (((x) >> 4) & 0xF)

static void deobfuscate_str(char *str, uint64_t val)
{
   uint8_t *dec_val = (uint8_t*)&val;
   int i = 0;

   while (*str != 0)
   {
      int pos = i / 2;
      bool bottom = (i % 2) == 0;
      uint8_t *ch = (uint8_t*)str;
      uint8_t xor = bottom ?
                    LOWER_HALFBYTE(dec_val[pos]) :
                    UPPER_HALFBYTE(dec_val[pos]);

      *ch ^= xor;

      if (++i == sizeof(uint64_t) * 2)
         i = 0;

      str++;
   }
}

void *get_obfuscated_func(HMODULE module, const char *str, uint64_t val)
{

   char new_name[128];

   strcpy(new_name, str);

   deobfuscate_str(new_name, val);

   return GetProcAddress(module, new_name);

}






