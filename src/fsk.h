/*
 * fsk.h
 *
 * Copyright (C) 2011-2020 Kamal Mostafa <kamal@whence.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#warning before
#if(FFTMODE == 1)
#warning FFTMODE 1
#include <fftw3.h>
#elif(FFTMODE == 2)
#warning FFTMODE 2
#include <kiss_fftr.h>
#else
#warning FFTMODE ???
#error Unsupported FFTMODE
#endif
#warning after

typedef struct fsk_plan fsk_plan;

struct fsk_plan {
	float		sample_rate;
    	float		f_mark;
    	float		f_space;
	float		filter_bw;

#if(FFTMODE == 1 || FFTMODE == 2)
	int		fftsize;
	unsigned int	nbands;
	float		band_width;
	unsigned int	b_mark;
	unsigned int	b_space;
#endif
#if(FFTMODE == 1)
	fftwf_plan	fftplan;
	float		*fftin;
	fftwf_complex	*fftout;
#elif(FFTMODE == 2)
        kiss_fftr_cfg    fftcfg;
        kiss_fft_cpx    *fin;
        kiss_fft_cpx    *fout;
#endif
};


fsk_plan *
fsk_plan_new(
	float		sample_rate,
    	float		f_mark,
    	float		f_space,
	float		filter_bw
	);

void
fsk_plan_destroy( fsk_plan *fskp );

/* returns confidence value [0.0 to 1.0] */
float
fsk_find_frame( fsk_plan *fskp, float *samples, unsigned int frame_nsamples,
	unsigned int try_first_sample,
	unsigned int try_max_nsamples,
	unsigned int try_step_nsamples,
	float try_confidence_search_limit,
	const char *expect_bits_string,
	unsigned long long *bits_outp,
	float *ampl_outp,
	unsigned int *frame_start_outp
	);

int
fsk_detect_carrier(fsk_plan *fskp, float *samples, unsigned int nsamples,
	float min_mag_threshold );

void
fsk_set_tones_by_bandshift( fsk_plan *fskp, unsigned int b_mark, int b_shift );


// FIXME move this?:
// #define FSK_DEBUG
#ifdef FSK_DEBUG
# define debug_log(format, args...)  fprintf(stderr, format, ## args)
#else
# define debug_log(format, args...)
#endif

