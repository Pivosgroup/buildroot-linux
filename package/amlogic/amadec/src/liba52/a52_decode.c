#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <math.h>
#include <assert.h>
#include <inttypes.h>

#include "adec.h"
#include "log.h"
#include "a52.h"
#include "mm_accel.h"

#define LOCAL inline

static adec_feeder_t *c_feeder = NULL;
static a52_state_t *a52_state;
static unsigned int a52_flags = 0;
static unsigned char frame_buffer[3840];

static sample_t gain = 1;
static int audio_output_channels = 2;

typedef struct {
	int samplerate;
	int bps;
	int channels;
	int a_in_buffer_len;
	char *a_in_buffer;
} a52_buffer_t;

static a52_buffer_t a52_buffer;

static LOCAL int read_byte(void)
{
	unsigned long tmp;
	c_feeder->get_bits(&tmp, 8);
	return tmp;
}

static LOCAL int read_2byte(void)
{
	unsigned long tmp;
	c_feeder->get_bits(&tmp, 16);
	return tmp;
}

static LOCAL int reset_bits(void)
{
	unsigned long tmp, bit;
	bit = c_feeder->bits_left() & (7);
	if (bit > 0)
		c_feeder->get_bits(&tmp, bit);
}

static LOCAL int a52_read(char *buf, int size)
{
	int i;
	reset_bits();
	for (i = 0; i < size; i++) {
		buf[i] = read_byte();
		//tmp=read_2byte();
		//buf[2*i]=(tmp&0xff00)>>8;
		//buf[2*i+1]=tmp&0xff;

	}
	return i;
}

static int a52_fillbuff(a52_buffer_t * buf)
{
	int length = 0;
	int flags = 0;
	int sample_rate = 0;
	int bit_rate = 0;

	buf->a_in_buffer_len = 0;

	while (1) {
		while (buf->a_in_buffer_len < 7) {
			int c = read_byte();
			if (c < 0)
				return -1;
			buf->a_in_buffer[buf->a_in_buffer_len++] = c;
		}
		length =
		    a52_syncinfo(buf->a_in_buffer, &flags, &sample_rate,
				 &bit_rate);
		//a52_flags |= flags;
		if (length >= 7 && length <= 3840)
			break;
		memmove(buf->a_in_buffer, buf->a_in_buffer + 1, 6);
		--buf->a_in_buffer_len;
	}

	buf->samplerate = sample_rate;
	buf->bps = bit_rate / 8;
	a52_read((buf->a_in_buffer + 7), (length - 7));

	return length;
}

static int liba52_init(adec_feeder_t * feeder)
{
	uint32_t a52_accel = 0;
	int flags = 0;

	c_feeder = feeder;
	if (feeder == NULL)
		return -1;

	memset(&a52_buffer, 0, sizeof(a52_buffer));
	a52_buffer.a_in_buffer = frame_buffer;

	a52_state = a52_init(a52_accel);
	if (a52_state == NULL) {
		printf("A52 init failed !\n");
		return -1;
	}

	if (a52_fillbuff(&a52_buffer) < 0) {
		printf("A52 sync failed !\n");
		return -1;
	}

	a52_buffer.channels = audio_output_channels;
	switch (a52_buffer.channels) {
	case 1:
		a52_flags = A52_MONO;
		break;
		/*case 2: a52_flags=A52_STEREO; break; */
	case 2:
		a52_flags = A52_DOLBY;
		break;
		/*case 3: a52_flags=A52_3F; break; */
	case 3:
		a52_flags = A52_2F1R;
		break;
	case 4:
		a52_flags = A52_2F2R;
		break;		/* 2+2 */
	case 5:
		a52_flags = A52_3F2R;
		break;
	case 6:
		a52_flags = A52_3F2R | A52_LFE;
		break;		/* 5.1 */
	}
	flags |= a52_flags;

	if (a52_resample_init(a52_accel, flags, a52_buffer.channels) < 0) {
		printf("a52_resample_init  failed!\n");
		a52_free(a52_state);
		return -1;
	}

	feeder->channel_num = audio_output_channels;
	feeder->sample_rate = a52_buffer.samplerate;

	return 0;
}

static int a52_decode_frame(unsigned char *buf, int maxlen,
			    struct frame_fmt *fmt)
{
	sample_t level = 1, bias = 384;
	int flags;
	int i, len = -1;
	if (!a52_buffer.a_in_buffer_len)
		if (a52_fillbuff(&a52_buffer) < 0)
			return len;
	a52_buffer.a_in_buffer_len = 0;

	flags = a52_flags | A52_ADJUST_LEVEL;
	level *= gain;

	if (a52_frame(a52_state, a52_buffer.a_in_buffer, &flags, &level, bias)) {
		printf("a52_decode_frame a52: error decoding frame\n");
		return len;
	}

	len = 0;
	for (i = 0; i < 6; i++) {
		if (a52_block(a52_state)) {
			printf("a52: error at resampling\n");
			break;
		}
		len +=
		    2 * a52_resample(a52_samples(a52_state),
				     (int16_t *) & buf[len]);
	}

	assert(len <= maxlen);
	return len;
}

static int a52_decode_release(void)
{
	if (a52_state) {
		a52_free(a52_state);
		a52_state = NULL;
	}
	memset(frame_buffer, 0, 3840);
	return 0;
}

static am_codec_struct a52_codec = {
	.name = "liba52",
	.format = AUDIO_FORMAT_AC3,
	.used = 0,
	.init = liba52_init,
	.release = a52_decode_release,
	.decode_frame = a52_decode_frame,
};

int register_a52_codec(void)
{
	return register_audio_codec(&a52_codec);
}
