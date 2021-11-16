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

#pragma once

#include <stdint.h>
#include <inttypes.h>
char *bitmap_source_decode_str(const char *src);
struct bitmap_source;
void bitmap_reader_read_size(struct bitmap_source * pbitmapsource);

#if defined(_UNICODE)
#define _T(x) L ##x
#else
#define _T(x) x
#endif

#ifdef __cplusplus
extern "C" {
#endif
#ifdef _WIN32
	char szName[] = "Local\\bitmap-source-%s";
	char szNameMutex[] = "Local\\bitmap-source-%s";
#pragma comment(lib, "psapi.lib")
#else
	char szName[] = "$HOME/.ca2/bitmap-source-%s;
		char szNameMutex[] = "$HOME/.ca2/bitmap-source-%s;
#endif

		char *bitmap_source_get_path(const char *);

	bool bitmap_source_audio_render(obs_source_t *transition, uint64_t *ts_out,
		struct obs_source_audio_mix *audio_output,
		uint32_t mixers, size_t channels,
		size_t sample_rate);


	/* this is a workaround to A/Vs going crazy whenever certain functions (such as
	* OpenProcess) are used */
	extern void *get_obfuscated_func(HMODULE module, const char *str, uint64_t val);

#ifdef __cplusplus
}
#endif

#define NUM_TEXTURES 2

struct file_mapping
{

	HANDLE			m_hmutex;
	HANDLE			m_hFile;
	HANDLE			m_hMapFile;
	int64_t	*		m_bitmap;
	int64_t			m_iFileLen;

};

int file_mapping_init(struct file_mapping * pfilemapping, const char * pszName);
void file_mapping_term(struct file_mapping * pfilemapping);


struct bitmap_reader
{

	struct file_mapping	m_filemapping;

	int          cur_tex;
	gs_texture_t *textures[NUM_TEXTURES];
	bool         textures_written[NUM_TEXTURES];
	int64_t x, y;
	int64_t     width;
	int64_t height;
	int          num_textures;

	bool         compatibility;
	HDC          hdc;
	HBITMAP      bmp, old_bmp;
	BYTE         *bits;

	bool         valid;
	COLORREF *   pcolorref;

};

struct bitmap_source;

extern void bitmap_reader_init(struct bitmap_source * pbitmapsource);
extern void bitmap_reader_free(struct bitmap_source * pbitmapsource);
extern void bitmap_reader_read(struct bitmap_source * pbitmapsource);
extern void bitmap_reader_draw(struct bitmap_source * pbitmapsource, gs_effect_t *effect);

#pragma once

#include <util/dstr.h>




extern void fill_bitmap_list(obs_property_t *p);




struct bitmap_source
{

	obs_source_t *		source;
	obs_property_t *	plist;

	char *			name;
	bool			compatibility;

	struct bitmap_reader    reader;

	float			resize_timer;

	int64_t			last_width;
	int64_t			last_height;

	pthread_mutex_t		mutex;


};

obs_source_t *bitmap_source_get_transition(struct bitmap_source *ss)
{
	obs_source_t *tr;

	pthread_mutex_lock(&ss->mutex);
	tr = ss->source;
	obs_source_addref(tr);
	pthread_mutex_unlock(&ss->mutex);

	return tr;
}


#define TEXT_BITMAP_SOURCE  obs_module_text("BitmapSource")
#define TEXT_WINDOW         obs_module_text("BitmapSource.Window")
#define TEXT_MATCH_PRIORITY obs_module_text("BitmapSource.Priority")
#define TEXT_MATCH_TITLE    obs_module_text("BitmapSource.Priority.Title")
#define TEXT_MATCH_CLASS    obs_module_text("BitmapSource.Priority.Class")
#define TEXT_MATCH_EXE      obs_module_text("BitmapSource.Priority.Exe")
#define TEXT_CAPTURE_CURSOR obs_module_text("CaptureCursor")
#define TEXT_COMPATIBILITY  obs_module_text("Compatibility")


static void update_settings(struct bitmap_source *pbitmapsource, obs_data_t *s)
{

	if (!gs_gdi_texture_available()) {

		pbitmapsource->compatibility = true;
	}

	const char * name = obs_data_get_string(s, "bitmap-name");

	bfree(pbitmapsource->name);

	pbitmapsource->name = bitmap_source_decode_str(name);

	bitmap_reader_free(pbitmapsource);

	bitmap_reader_init(pbitmapsource);


}


static void bitmap_source_update(void *data, obs_data_t *settings)
{

	struct bitmap_source *pbitmapsource = data;

	update_settings(pbitmapsource, settings);

}


static uint32_t bitmap_source_width(void *data)
{
	struct bitmap_source *pbitmapsource = data;
	return (uint32_t)pbitmapsource->reader.width;
}

static uint32_t bitmap_source_height(void *data)
{
	struct bitmap_source *pbitmapsource = data;
	return (uint32_t)pbitmapsource->reader.height;
}

static obs_properties_t *bitmap_source_properties(void *unused)
{
	UNUSED_PARAMETER(unused);

	obs_properties_t *ppts = obs_properties_create();
	obs_property_t *p;

	p = obs_properties_add_list(ppts, "bitmap-name", TEXT_WINDOW, OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);

	fill_bitmap_list(p);

	return ppts;

}


#define RESIZE_CHECK_TIME 0.2f

static void bitmap_source_tick(void *data, float seconds)
{

	struct bitmap_source *pbitmapsource = data;

	if (!obs_source_showing(pbitmapsource->source))
		return;

	obs_enter_graphics();

	bool bResizeReader = false;

	if (!pbitmapsource->reader.m_filemapping.m_bitmap) {
		bResizeReader = true;
	}
	if (pbitmapsource->reader.m_filemapping.m_bitmap)
	{

		pbitmapsource->resize_timer += seconds;

		if (pbitmapsource->resize_timer >= RESIZE_CHECK_TIME)
		{

			if (pbitmapsource->reader.m_filemapping.m_bitmap[0] !=
				pbitmapsource->last_width ||
				pbitmapsource->reader.m_filemapping.m_bitmap[1] !=
				pbitmapsource->last_height) {
				bResizeReader = true;
			}


			pbitmapsource->resize_timer = 0.0f;
		}
	}

   if (bResizeReader)
   {

      pbitmapsource->resize_timer = 0.0f;

	bitmap_reader_free(pbitmapsource);

	bitmap_reader_init(pbitmapsource);

}

	bitmap_reader_read(pbitmapsource);
	obs_leave_graphics();
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
	pbitmapsource->reader.m_filemapping.m_hFile = INVALID_HANDLE_VALUE;
	pthread_mutex_init_value(&pbitmapsource->mutex);
	if (pthread_mutex_init(&pbitmapsource->mutex, NULL) != 0)
		return NULL;

	update_settings(pbitmapsource, settings);
	return pbitmapsource;
}

static void bitmap_source_destroy(void *data)
{
	struct bitmap_source *pbitmapsource = data;

	if (pbitmapsource)
	{

		obs_enter_graphics();

		bitmap_reader_free(pbitmapsource);

		obs_leave_graphics();

		bfree(pbitmapsource->name);

		bfree(pbitmapsource);

	}
}




static void bitmap_source_defaults(obs_data_t *defaults)
{

	obs_data_set_default_bool(defaults, "cursor", true);

	obs_data_set_default_bool(defaults, "compatibility", false);

}



static void bitmap_source_render(void *data, gs_effect_t *effect)
{

	struct bitmap_source *pbitmapsource = data;

	bitmap_reader_draw(pbitmapsource, obs_get_base_effect(OBS_EFFECT_PREMULTIPLIED_ALPHA));

	UNUSED_PARAMETER(effect);

}



struct obs_source_info bitmap_source_info =
{
   .id = "bitmap-source",
   .type = OBS_SOURCE_TYPE_INPUT,
   .output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_CUSTOM_DRAW | OBS_SOURCE_COMPOSITE,
   .get_name = bitmap_source_getname,
   .create = bitmap_source_create,
   .destroy = bitmap_source_destroy,
   .update = bitmap_source_update,
   .video_render = bitmap_source_render,
   .video_tick = bitmap_source_tick,
   .audio_render = bitmap_source_audio_render,


   .get_width = bitmap_source_width,
   .get_height = bitmap_source_height,
   .get_defaults = bitmap_source_defaults,
   .get_properties = bitmap_source_properties
};




static inline void init_textures(struct bitmap_source * pbitmapsource)
{
	for (int i = 0; i < pbitmapsource->reader.num_textures; i++)
	{
		if (pbitmapsource->reader.compatibility)
			pbitmapsource->reader.textures[i] = gs_texture_create((uint32_t)pbitmapsource->reader.width,
			(uint32_t)pbitmapsource->reader.height,
				GS_BGRA, 1, NULL, GS_DYNAMIC);
		else
			pbitmapsource->reader.textures[i] = gs_texture_create_gdi(
			(uint32_t)pbitmapsource->reader.width, (uint32_t)pbitmapsource->reader.height);

		if (!pbitmapsource->reader.textures[i])
		{
			blog(LOG_WARNING, "[bitmap_reader_init] Failed to "
				"create textures");
			return;
		}
	}

	pbitmapsource->reader.valid = true;
}


int file_mapping_init(struct file_mapping * pfilemapping, const char * pszName)
{

	pfilemapping->m_iFileLen = -1;

	char szNameMutex2[2048];

	sprintf(szNameMutex2, szNameMutex, pszName);

	pfilemapping->m_hmutex = CreateMutexA(NULL, FALSE, szNameMutex2);

	if (pfilemapping->m_hmutex == NULL)
	{

		return 0;

	}

	if (WaitForSingleObject(pfilemapping->m_hmutex, INFINITE) != WAIT_OBJECT_0)
	{

		HANDLE hmutex = pfilemapping->m_hmutex;

		pfilemapping->m_hmutex = NULL;

		CloseHandle(hmutex);

		return 0;

	}

	char *szPath = bitmap_source_get_path(pszName);

	if (szPath == NULL)
	{

		file_mapping_term(pfilemapping);

		return 0;

	}

	pfilemapping->m_hFile = CreateFileA(
		szPath,
		FILE_READ_DATA,
		FILE_SHARE_WRITE | FILE_SHARE_READ, NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, NULL);

	bfree(szPath);

	if (pfilemapping->m_hFile == INVALID_HANDLE_VALUE)
	{

		file_mapping_term(pfilemapping);

		return 0;

	}


	pfilemapping->m_iFileLen = GetFileSize(pfilemapping->m_hFile, NULL);

	if (pfilemapping->m_iFileLen < 20)
	{

		file_mapping_term(pfilemapping);

		return 0;

	}


	pfilemapping->m_hMapFile = CreateFileMappingA(
		pfilemapping->m_hFile, // use paging file
		NULL,                 // default security
		PAGE_READONLY,       // read/write access
		0, // maximum object size (high-order DWORD)
		8192 * 4096 *
		4, // maximum object size (low-order DWORD)
		NULL);   // name of mapping object

	if (pfilemapping->m_hMapFile == NULL)
	{

		file_mapping_term(pfilemapping);

		return 0;

	}


	pfilemapping->m_bitmap = (int64_t*)MapViewOfFile(
		pfilemapping->m_hMapFile, // handle to map object
		FILE_MAP_READ, // read/write permission
		0, 0, 8192 * 4096 * 4);

	if (pfilemapping->m_bitmap == NULL)
	{

		file_mapping_term(pfilemapping);

		return 0;

	}

	ReleaseMutex(pfilemapping->m_hmutex);

	return 1;

}


void file_mapping_term(struct file_mapping * pfilemapping)
{

	if (pfilemapping->m_hmutex == NULL)
	{

		pfilemapping->m_hmutex = NULL;

		pfilemapping->m_hMapFile = NULL;

		pfilemapping->m_hFile = INVALID_HANDLE_VALUE;

		pfilemapping->m_iFileLen = -1;


		return;

	}

	if (WaitForSingleObject(pfilemapping->m_hmutex, INFINITE) != WAIT_OBJECT_0)
	{

		pfilemapping->m_hmutex = NULL;

		pfilemapping->m_hMapFile = NULL;

		pfilemapping->m_hFile = INVALID_HANDLE_VALUE;

		pfilemapping->m_iFileLen = -1;



		return;


	}

	if (pfilemapping->m_bitmap != NULL)
	{

		UnmapViewOfFile(pfilemapping->m_bitmap);

	}

	if (pfilemapping->m_hMapFile != NULL)
	{

		CloseHandle(pfilemapping->m_hMapFile);

	}

	if (pfilemapping->m_hFile != INVALID_HANDLE_VALUE)
	{

		CloseHandle(pfilemapping->m_hFile);

	}

	HANDLE hmutex = pfilemapping->m_hmutex;

	pfilemapping->m_hmutex = NULL;

	pfilemapping->m_hMapFile = NULL;

	pfilemapping->m_hFile = INVALID_HANDLE_VALUE;

	pfilemapping->m_iFileLen = -1;


	ReleaseMutex(hmutex);

	CloseHandle(hmutex);


}


void bitmap_reader_init(struct bitmap_source * pbitmapsource)
{

	memset(&pbitmapsource->reader, 0, sizeof(struct bitmap_reader));

	if (!file_mapping_init(&pbitmapsource->reader.m_filemapping, pbitmapsource->name))
	{

		memset(&pbitmapsource->reader, 0, sizeof(struct bitmap_reader));

		return;

	}

	if (pbitmapsource->reader.m_filemapping.m_bitmap == NULL) {

		return;
	}

	obs_enter_graphics();

	if (pbitmapsource->reader.m_filemapping.m_iFileLen < 16)
	{

		file_mapping_term(&pbitmapsource->reader.m_filemapping);

		return;

	}
	
	pbitmapsource->reader.width = pbitmapsource->reader.m_filemapping.m_bitmap[0];
	pbitmapsource->reader.height = pbitmapsource->reader.m_filemapping.m_bitmap[1];

	if (pbitmapsource->reader.m_filemapping.m_iFileLen < (pbitmapsource->reader.width * pbitmapsource->reader.height * 4 + 16))
	{

		file_mapping_term(&pbitmapsource->reader.m_filemapping);

		return;

	}


	pbitmapsource->last_width = pbitmapsource->reader.m_filemapping.m_bitmap[0];
	pbitmapsource->last_height = pbitmapsource->reader.m_filemapping.m_bitmap[1];
	pbitmapsource->reader.compatibility = pbitmapsource->compatibility;
	pbitmapsource->reader.num_textures = pbitmapsource->compatibility ? 1 : 2;

	init_textures(pbitmapsource);

	obs_leave_graphics();

	if (!pbitmapsource->reader.valid)
		return;

	if (pbitmapsource->compatibility)
	{
		BITMAPINFO bi = { 0 };
		BITMAPINFOHEADER *bih = &bi.bmiHeader;
		bih->biSize = sizeof(BITMAPINFOHEADER);
		bih->biBitCount = 32;
		bih->biWidth = (LONG)pbitmapsource->reader.width;
		bih->biHeight = (LONG)pbitmapsource->reader.height;
		bih->biPlanes = 1;

		pbitmapsource->reader.hdc = CreateCompatibleDC(NULL);
		pbitmapsource->reader.bmp = CreateDIBSection(pbitmapsource->reader.hdc, &bi,
			DIB_RGB_COLORS, (void**)&pbitmapsource->reader.bits,
			NULL, 0);
		pbitmapsource->reader.pcolorref = (COLORREF *)malloc(pbitmapsource->reader.width * pbitmapsource->reader.height * 4);
		pbitmapsource->reader.old_bmp = SelectObject(pbitmapsource->reader.hdc, pbitmapsource->reader.bmp);
	}


}


void bitmap_reader_free(struct bitmap_source * pbitmapsource)
{
	if (pbitmapsource->reader.hdc)
	{
		free(pbitmapsource->reader.pcolorref);
		SelectObject(pbitmapsource->reader.hdc, pbitmapsource->reader.old_bmp);
		DeleteDC(pbitmapsource->reader.hdc);
		DeleteObject(pbitmapsource->reader.bmp);
	}

	obs_enter_graphics();

	for (int i = 0; i < pbitmapsource->reader.num_textures; i++)
		gs_texture_destroy(pbitmapsource->reader.textures[i]);

	obs_leave_graphics();

	file_mapping_term(&pbitmapsource->reader.m_filemapping);

	memset(&pbitmapsource->reader, 0, sizeof(struct bitmap_reader));



}


static inline HDC bitmap_reader_get_dc(struct bitmap_source * pbitmapsource)
{
	if (!pbitmapsource->reader.valid)
		return NULL;

	if (pbitmapsource->reader.compatibility)
		return pbitmapsource->reader.hdc;
	else
		return gs_texture_get_dc(pbitmapsource->reader.textures[pbitmapsource->reader.cur_tex]);
}

static inline void bitmap_reader_release_dc(struct bitmap_source *pbitmapsource)
{
	if (pbitmapsource->reader.compatibility)
	{
		gs_texture_set_image(pbitmapsource->reader.textures[pbitmapsource->reader.cur_tex], pbitmapsource->reader.bits,
			(uint32_t)pbitmapsource->reader.width * 4, false);
	}
	else
	{
		gs_texture_release_dc(pbitmapsource->reader.textures[pbitmapsource->reader.cur_tex]);
	}
}


void bitmap_reader_read(struct bitmap_source * pbitmapsource)
{

	HDC hdc;

	if (!pbitmapsource->reader.m_filemapping.m_bitmap) {
		return;

	}


	if (++pbitmapsource->reader.cur_tex == pbitmapsource->reader.num_textures)
	{

		pbitmapsource->reader.cur_tex = 0;

	}

	hdc = bitmap_reader_get_dc(pbitmapsource);

	if (!hdc)
	{

		blog(LOG_WARNING, "[capture_screen] Failed to get texture DC");

		return;

	}

	HBITMAP bmp = pbitmapsource->reader.bmp;

	if (pbitmapsource->reader.m_filemapping.m_hmutex != NULL)
	{

		if (WaitForSingleObject(pbitmapsource->reader.m_filemapping.m_hmutex, 1) == WAIT_OBJECT_0)
		{

			if (pbitmapsource->reader.m_filemapping.m_bitmap != NULL)
			{

				BITMAPINFO m_bitmapinfo;

				ZeroMemory(&m_bitmapinfo, sizeof(m_bitmapinfo));

				int64_t * p = pbitmapsource->reader.m_filemapping.m_bitmap;

				int64_t cx = *p++;
				int64_t cy = *p++;
				int64_t scan = *p++;

				m_bitmapinfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
				m_bitmapinfo.bmiHeader.biWidth = (LONG)cx;
				m_bitmapinfo.bmiHeader.biHeight = (LONG)-cy;
				m_bitmapinfo.bmiHeader.biPlanes = 1;
				m_bitmapinfo.bmiHeader.biBitCount = 32;
				m_bitmapinfo.bmiHeader.biCompression = BI_RGB;
				m_bitmapinfo.bmiHeader.biSizeImage = (LONG)(cy * scan);

				//if (bmp == NULL)
				//{

				//   bmp = (HBITMAP) GetCurrentObject(hdc, OBJ_BITMAP);
				//}

				SetDIBits(hdc, bmp, 0, (UINT)cy, p, &m_bitmapinfo, DIB_RGB_COLORS);

			}

			ReleaseMutex(pbitmapsource->reader.m_filemapping.m_hmutex);

		}

	}

	bitmap_reader_release_dc(pbitmapsource);

	pbitmapsource->reader.textures_written[pbitmapsource->reader.cur_tex] =
		true;

}

static void draw_texture(struct bitmap_source *pbitmapsource, int id, gs_effect_t *effect)
{

	gs_texture_t   *texture = pbitmapsource->reader.textures[id];
	gs_technique_t *tech = gs_effect_get_technique(effect, "Draw");
	gs_eparam_t    *image = gs_effect_get_param_by_name(effect, "image");
	size_t      passes;

	gs_effect_set_texture(image, texture);

	passes = gs_technique_begin(tech);
	for (size_t i = 0; i < passes; i++)
	{
		if (gs_technique_begin_pass(tech, i))
		{
			if (pbitmapsource->reader.compatibility)
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
				gs_draw_sprite(texture, 0, 0, 0);

			gs_technique_end_pass(tech);
		}
	}
	gs_technique_end(tech);
}

void bitmap_reader_draw(struct bitmap_source *pbitmapsource, gs_effect_t *effect)
{
	int last_tex = (pbitmapsource->reader.cur_tex > 0) ?
		pbitmapsource->reader.cur_tex - 1 : pbitmapsource->reader.num_textures - 1;

	if (!pbitmapsource->reader.valid)
		return;

	if (pbitmapsource->reader.textures_written[last_tex])
		draw_texture(pbitmapsource, last_tex, effect);
}



static inline void encode_dstr(struct dstr *str)
{
	dstr_replace(str, "#", "#22");
	dstr_replace(str, ":", "#3A");
}

char *bitmap_source_decode_str(const char *src)
{
	struct dstr str = { 0 };
	dstr_copy(&str, src);
	dstr_replace(&str, "#3A", ":");
	dstr_replace(&str, "#22", "#");
	return str.array;
}

static HMODULE kernel32(void)
{
	static HMODULE kernel32_handle = NULL;
	if (!kernel32_handle)
		kernel32_handle = GetModuleHandleA("kernel32");
	return kernel32_handle;
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






