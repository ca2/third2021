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
struct bitmap_source;

bool bitmap_source_audio_render_(void *data, uint64_t *ts_out,
				 struct obs_source_audio_mix *audio_output,
				 uint32_t mixers, size_t channels,
				 size_t sample_rate);

obs_source_t *bitmap_source_get_transition(struct bitmap_source *ss);


bool bitmap_source_audio_render(obs_source_t *transition, uint64_t *ts_out,
   struct obs_source_audio_mix *audio_output,
   uint32_t mixers, size_t channels, size_t sample_rate)
{
   struct obs_source_audio_mix child_audio;
   uint64_t source_ts;

   if (obs_source_audio_pending(transition))
      return false;

   source_ts = obs_source_get_audio_timestamp(transition);
   if (!source_ts)
      return false;

   obs_source_get_audio_mix(transition, &child_audio);
   for (size_t mix = 0; mix < MAX_AUDIO_MIXES; mix++) {
      if ((mixers & (1 << mix)) == 0)
         continue;

      for (size_t ch = 0; ch < channels; ch++) {
         float *out = audio_output->output[mix].data[ch];
         float *in = child_audio.output[mix].data[ch];

         memcpy(out, in, AUDIO_OUTPUT_FRAMES *
            MAX_AUDIO_CHANNELS * sizeof(float));
      }
   }

   *ts_out = source_ts;

   UNUSED_PARAMETER(sample_rate);
   return true;
}

bool
bitmap_source_audio_render_(void *data, uint64_t *ts_out,
   struct obs_source_audio_mix *audio_output,
   uint32_t mixers, size_t channels, size_t sample_rate)
{
   struct bitmap_source * pbitmapsource = data;
	obs_source_t *transition = bitmap_source_get_transition(pbitmapsource);
   bool success;

   if (!transition)
      return false;

   success = bitmap_source_audio_render_(transition, ts_out, audio_output, mixers,
      channels, sample_rate);

   obs_source_release(transition);
   return success;
}
