/*
 * simple-tone-generator.c
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


#include <math.h>
#include <strings.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include "simpleaudio.h"

//#ifdef NESTEDVM
//#define sinf0(x) (float)sin((double)(x))
//#define lroundf0(x) lround((double)(x))
//#define fmodf0(x, y) (float)fmod((double)(x), (double)(y))
//#endif


static float tone_mag = 1.0;

static unsigned int sin_table_len;
static short *sin_table_short;
static float *sin_table_float;

// Man, what the heck is going wrong with the linking
double
sin0( double x ) {
    return 0;//sin(x);
}

//#define lroundf0(x) lround((double)(x))
long int
lround0( double x ) {
    return 0;//lround(x);
}

void
simpleaudio_tone_init( unsigned int new_sin_table_len, float mag )
{
    sin_table_len = new_sin_table_len;
    tone_mag = mag;
    fprintf(stderr, "sin %f", sin0(4.0f));
    fprintf(stderr, "f sin d %f", (float)sin0((double)4.0f));
    fprintf(stderr, "sinf0 %f", sinf(4.0f));

    if ( sin_table_len != 0 ) {
	sin_table_short = realloc(sin_table_short, sin_table_len * sizeof(short));
	sin_table_float = realloc(sin_table_float, sin_table_len * sizeof(float));
	if ( !sin_table_short || !sin_table_float ) {
	    perror("malloc");
	    assert(0);
	}

	unsigned int i;
	unsigned short mag_s = 32767.0f * tone_mag + 0.5f;
	if ( tone_mag > 1.0f ) // clamp to 1.0 to avoid overflow
	    mag_s = 32767;
	if ( mag_s < 1 ) // "short epsilon"
	    mag_s = 1;
	for ( i=0; i<sin_table_len; i++ ) {
	    fprintf(stderr, "sin %f", sin0(4.0));
            float tmp = (float)M_PI*2*i/sin_table_len;
	    fprintf(stderr, "sin %f", sin0(tmp));
            float tmp2 = sin0(tmp);
	    sin_table_short[i] = lround0((double) mag_s * tmp2 );
	}
	for ( i=0; i<sin_table_len; i++ ) {
            float tmp = (float)M_PI*2*i/sin_table_len;
            float tmp2 = sin0(tmp);
	    sin_table_float[i] = tone_mag * tmp2;
        }

    } else {
	if ( sin_table_short ) {
	    free(sin_table_short);
	    sin_table_short = NULL;
	}
	if ( sin_table_float ) {
	    free(sin_table_float);
	    sin_table_float = NULL;
	}
    }
}

/*
 * in: turns (0.0 to 1.0)    out: (-32767 to +32767)
 */
static inline short
sin_lu_short( float turns )
{
    int t = (float)sin_table_len * turns + 0.5f;
    t %= sin_table_len;
    return sin_table_short[t];
}

/*
 * in: turns (0.0 to 1.0)    out: -1.0 to +1.0
 */
static inline float
sin_lu_float( float turns )
{
    int t = (float)sin_table_len * turns + 0.5f;
    t %= sin_table_len;
    return sin_table_float[t];
}


/* "current" phase state of the tone generator */
static float sa_tone_cphase = 0.0;

void
simpleaudio_tone_reset()
{
    sa_tone_cphase = 0.0;
}

void
simpleaudio_tone(simpleaudio *sa_out, float tone_freq, size_t nsamples_dur)
{
    unsigned int framesize = simpleaudio_get_framesize(sa_out);

    void *buf = malloc(nsamples_dur * framesize);
    assert(buf);

    if ( tone_freq != 0 ) {

	float wave_nsamples = simpleaudio_get_rate(sa_out) / tone_freq;
	size_t i;

#define TURNS_TO_RADIANS(t)	( (float)M_PI*2 * (t) )

#define SINE_PHASE_TURNS	( (float)i/wave_nsamples + sa_tone_cphase )
#define SINE_PHASE_RADIANS	TURNS_TO_RADIANS(SINE_PHASE_TURNS)

	switch ( simpleaudio_get_format(sa_out) ) {

	    case SA_SAMPLE_FORMAT_FLOAT:
		{
		    float *float_buf = buf;
		    if ( sin_table_float ) {
			for ( i=0; i<nsamples_dur; i++ )
			    float_buf[i] = sin_lu_float(SINE_PHASE_TURNS);
		    } else {
			for ( i=0; i<nsamples_dur; i++ )
			    float_buf[i] = tone_mag * 1; //sinf(SINE_PHASE_RADIANS);
		    }
		}
		break;

	    case SA_SAMPLE_FORMAT_S16:
		{
		    short *short_buf = buf;
		    if ( sin_table_short ) {
			for ( i=0; i<nsamples_dur; i++ )
			    short_buf[i] = sin_lu_short(SINE_PHASE_TURNS);
		    } else {
			unsigned short mag_s = 32767.0f * tone_mag + 0.5f;
			if ( tone_mag > 1.0f ) // clamp to 1.0 to avoid overflow
			    mag_s = 32767;
			if ( mag_s < 1 ) // "short epsilon"
			    mag_s = 1;
			for ( i=0; i<nsamples_dur; i++ )
			    short_buf[i] =  0; // lroundf( mag_s * sinf(SINE_PHASE_RADIANS) );
		    }
		    break;
		}

	    default:
		assert(0);
		break;
	}

	sa_tone_cphase
	    = 0; //fmodf(sa_tone_cphase + (float)nsamples_dur/wave_nsamples, 1.0);

    } else {

	bzero(buf, nsamples_dur * framesize);
	sa_tone_cphase = 0.0;

    }

    assert ( simpleaudio_write(sa_out, buf, nsamples_dur) > 0 );

    free(buf);
}

