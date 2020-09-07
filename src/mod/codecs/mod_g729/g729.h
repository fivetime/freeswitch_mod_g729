#ifndef _G729_h
#define _G729_h

#include "g729a_v11/typedef.h"
#include "g729a_v11/basic_op.h"
#include "g729a_v11/ld8a.h"
#include "g729a_v11/tab_ld8a.h"
#include "g729a_v11/util.h"
#include "g729a_v11/pre_proc.h"

#define SLIN_FRAME_LEN  160
#define G729_FRAME_LEN  10
#define G729_SAMPLES    80 /* 10ms at 8000 hz, 160 bytes signed linear */
#define BUFFER_SAMPLES  8000
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

#endif