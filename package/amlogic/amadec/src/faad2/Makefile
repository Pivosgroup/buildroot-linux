
LIBNAME_COMMON=libfaad.a

OBJS=aac_decode.o bits.o cfft.o decoder.o drc.o \
                     drm_dec.o error.o filtbank.o \
                     ic_predict.o is.o lt_predict.o mdct.o mp4.o ms.o output.o pns.o \
                     ps_dec.o ps_syntax.o \
                     pulse.o specrec.o syntax.o tns.o hcr.o huffman.o \
                     rvlc.o ssr.o ssr_fb.o ssr_ipqf.o common.o \
                     sbr_dct.o sbr_e_nf.o sbr_fbt.o sbr_hfadj.o sbr_hfgen.o \
                     sbr_huff.o sbr_qmf.o sbr_syntax.o sbr_tf_grid.o sbr_dec.o

#HEADERS1=$(shell ls ../include/*.h)
#HEADERS=$(shell ls *.h) $(HEADERS1)
DEFS = -DHAVE_CONFIG_H

all:$(LIBNAME_COMMON)

$(LIBNAME_COMMON):$(OBJS)
	$(HOST_NAME)-ar rcs $@ $(OBJS)

$(OBJS):%.o:%.c
	$(CC) -c $(DEFS) $(CFLAGS) $< -o $@


clean:
	-rm -rf $(OBJS)
