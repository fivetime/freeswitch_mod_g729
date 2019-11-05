/*
 * FreeSWITCH Modular Media Switching Software Library / Soft-Switch Application
 * Copyright (C) 2005/2006, Anthony Minessale II <anthmct@yahoo.com>
 *
 * Version: MPL 1.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is FreeSWITCH Modular Media Switching Software Library / Soft-Switch Application
 *
 * The Initial Developer of the Original Code is
 * Anthony Minessale II <anthmct@yahoo.com>
 * Portions created by the Initial Developer are Copyright (C)
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Anthony Minessale II <anthmct@yahoo.com>
 * Michael Jerris <mike@jerris.com>
 *
 * The g729 codec itself is not distributed with this module.
 *
 * mod_g729.c -- G.729 Codec Module
 *
 */

#include "switch.h"

SWITCH_MODULE_LOAD_FUNCTION(mod_g729_load);
SWITCH_MODULE_DEFINITION(mod_g729, mod_g729_load, NULL, NULL);

#define SLIN_FRAME_LEN  160
#define G729_FRAME_LEN  10
#define G729_SAMPLES    80 /* 10ms at 8000 hz, 160 bytes signed linear */
#define BUFFER_SAMPLES  8000

#include "g729a_v11/typedef.h"
#include "g729a_v11/basic_op.h"
#include "g729a_v11/ld8a.h"
#include "g729a_v11/tab_ld8a.h"
#include "g729a_v11/util.h"
#include "g729a_v11/pre_proc.h"

/* Sample frame data Hz zachem ono nado */
#include "slin_g729_ex.h"
#include "g729_slin_ex.h"

struct g72x_coder_pvt {
    CodState *coder;
    DecState *decoder;
    void *scratch_mem;
    int16_t buf[BUFFER_SAMPLES]; /* 1 second */
};
#define dec_state g72x_coder_pvt
#define cod_state g72x_coder_pvt
#define PVT struct g72x_coder_pvt

#define g729_coder_pvt translator_pvt
void g729_init_lib(){}

void g729_init_coder(PVT *hEncoder, int dummy){
    struct g72x_coder_pvt *state = hEncoder;
//    state = malloc(sizeof(struct g72x_coder_pvt));
    state->coder = Init_Coder_ld8a();
    Init_Pre_Process(state->coder);
    return;
}

void g729_release_coder(PVT *hEncoder){
    struct g72x_coder_pvt *state = hEncoder;
    free (state->coder);
    free (state->scratch_mem);
}

void g729_init_decoder(PVT *hDecoder){
    struct g72x_coder_pvt *state = hDecoder;
//    state = malloc(sizeof(struct g72x_coder_pvt));
    state->decoder = Init_Decod_ld8a();
    Init_Post_Filter(state->decoder);
    Init_Post_Process(state->decoder);
    return;
}

void g729_release_decoder(PVT *hDecoder){
    struct g72x_coder_pvt *state = hDecoder;
    free (state->decoder);
    free (state->scratch_mem);
}

void g729_coder(PVT *hEncoder, short *ddp, char *edp, int *cbret){
        Word16 parm[PRM_SIZE];

        Copy ((Word16 *) ddp, hEncoder->coder->new_speech, 80);
        Pre_Process(hEncoder->coder, hEncoder->coder->new_speech, 80);
        Coder_ld8a(hEncoder->coder, parm);
        Store_Params(parm, edp);
}
#if 0
static int g729_frame_type(int datalen)
{
    switch (datalen) {
        case 0: return -1;  /* erased */
     /* case 0: return 0; maybe it should be 0 - untransmitted silence? */
        case 2: return 1;  /* SID */
        case 8: return 2;  /* 729d */
        case 10: return 3; /* 729, 729a */
        case 15: return 4; /* 729e */
    }
    return 0;
}
#endif
void g729_decoder(PVT *hDecoder, short *ddp, char *edp, int plen){
        Word16 i;
        Word16 *synth;
        Word16 parm[PRM_SIZE + 1];

        Restore_Params(edp, &parm[1]);
        synth = hDecoder->decoder->synth_buf + M;

        parm[0] = 1;
        for (i = 0; i < PRM_SIZE; i++) {
                if (parm[i + 1] != 0) {
                        parm[0] = 0;
                        break;
                }
        }

        parm[4] = Check_Parity_Pitch(parm[3], parm[4]);

        Decod_ld8a(hDecoder->decoder, parm, synth, hDecoder->decoder->Az_dec,
                                            hDecoder->decoder->T2, &hDecoder->decoder->bad_lsf);
        Post_Filter(hDecoder->decoder, synth, hDecoder->decoder->Az_dec, hDecoder->decoder->T2);
        Post_Process(hDecoder->decoder, synth, L_FRAME);
        memmove( ddp, synth, 2 * L_FRAME);
}

struct g729_context {
	struct dec_state decoder_object;
	struct cod_state encoder_object;
};

static switch_status_t switch_g729_init(switch_codec_t *codec, switch_codec_flag_t flags, const switch_codec_settings_t *codec_settings)
{
	struct g729_context *context = NULL;
	int encoding, decoding;

	encoding = (flags & SWITCH_CODEC_FLAG_ENCODE);
	decoding = (flags & SWITCH_CODEC_FLAG_DECODE);

	if (!(encoding || decoding) || (!(context = switch_core_alloc(codec->memory_pool, sizeof(struct g729_context))))) {
		return SWITCH_STATUS_FALSE;
	} else {
		if (codec->fmtp_in) {
			codec->fmtp_out = switch_core_strdup(codec->memory_pool, codec->fmtp_in);
		}

		if (encoding) {
			g729_init_coder(&context->encoder_object, 0);
		}

		if (decoding) {
			g729_init_decoder(&context->decoder_object);
		}

		codec->private_info = context;

		return SWITCH_STATUS_SUCCESS;
	}
}

static switch_status_t switch_g729_destroy(switch_codec_t *codec)
{
        struct g729_context *context;
        context = codec->private_info;
	g729_release_coder( &(context->encoder_object));
	g729_release_decoder( &(context->decoder_object));
	codec->private_info = NULL;
	return SWITCH_STATUS_SUCCESS;
}

static switch_status_t switch_g729_encode(switch_codec_t *codec,
                                              switch_codec_t *other_codec,
                                              void *decoded_data,
                                              uint32_t decoded_data_len, uint32_t decoded_rate,
                                              void *encoded_data, uint32_t *encoded_data_len, uint32_t *encoded_rate,
                                              unsigned int *flag)
{
	struct g729_context *context = codec->private_info;
	int cbret = 0;

//	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "g729 encode!!!\n");
	if (!context) {
		return SWITCH_STATUS_FALSE;
	}

	if (decoded_data_len % 160 == 0) {
		uint32_t new_len = 0;
		int16_t *ddp = decoded_data;
		char *edp = encoded_data;
		int x;
		int loops = (int) decoded_data_len / 160;

		for (x = 0; x < loops && new_len < *encoded_data_len; x++) {
			g729_coder(&context->encoder_object, ddp, edp, &cbret);
			edp += 10;
			ddp += 80;
			new_len += 10;
		}

		if (new_len <= *encoded_data_len) {
			*encoded_data_len = new_len;
		} else {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "buffer overflow!!! %u >= %u\n", new_len, *encoded_data_len);
			return SWITCH_STATUS_FALSE;
		}
	}
	return SWITCH_STATUS_SUCCESS;
}
// For zero data
static int16_t lost_frame[80] = { 0 };

static switch_status_t switch_g729_decode(switch_codec_t *codec,
                                          switch_codec_t *other_codec,
                                          void *encoded_data,
                                          uint32_t encoded_data_len, uint32_t encoded_rate,
                                          void *decoded_data, uint32_t *decoded_data_len, uint32_t *decoded_rate,
                                          unsigned int *flag)
{
    struct g729_context *context = codec->private_info;
    int framesize;
    int x;
    uint32_t new_len = 0;
    char *edp = encoded_data;
    short *ddp = decoded_data;

    if (!context) {
        return SWITCH_STATUS_FALSE;
    }
//////////////////// Start code from * g729

    if (encoded_data_len == 0) {  /* Native PLC interpolation */
	g729_decoder(&context->decoder_object, ddp, (char *)lost_frame, 0);
	ddp+=80; *decoded_data_len=160;
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "g729 zero length frame\n");
        return SWITCH_STATUS_SUCCESS;
    }

    for(x = 0; x < encoded_data_len && new_len < *decoded_data_len; x += framesize) {
        if(encoded_data_len - x < 8)
            framesize = 2;  /* SID */
        else
            framesize = 10; /* regular 729a frame */
	g729_decoder(&context->decoder_object, ddp, edp, framesize);
	ddp += 80;
	edp += framesize;
	new_len += 160;
    }
    if (new_len <= *decoded_data_len) {
        *decoded_data_len = new_len;
    } else {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "buffer overflow!!!\n");
        return SWITCH_STATUS_FALSE;
    }
    return SWITCH_STATUS_SUCCESS;
//////////////////// End code from * g729
}

SWITCH_MODULE_LOAD_FUNCTION(mod_g729_load)
{
	switch_codec_interface_t *codec_interface;
	int mpf = 10000, spf = 80, bpf = 160, ebpf = 10, count;

	/* connect my internal structure to the blank pointer passed to me */
	*module_interface = switch_loadable_module_create_module_interface(pool, modname);

	SWITCH_ADD_CODEC(codec_interface, "G.729");
	for (count = 12; count > 0; count--) {
		switch_core_codec_add_implementation(pool, codec_interface,
                SWITCH_CODEC_TYPE_AUDIO, 18, "G729", NULL, 8000, 8000, 8000,
                mpf * count, spf * count, bpf * count, ebpf * count, 1, count * 10,
                switch_g729_init, switch_g729_encode, switch_g729_decode, switch_g729_destroy);
	}
	/* indicate that the module should continue to be loaded */
	return SWITCH_STATUS_SUCCESS;
}

/* For Emacs:
 * Local Variables:
 * mode:c
 * indent-tabs-mode:t
 * tab-width:4
 * c-basic-offset:4
 * End:
 * For VIM:
 * vim:set softtabstop=4 shiftwidth=4 tabstop=4:
 */
