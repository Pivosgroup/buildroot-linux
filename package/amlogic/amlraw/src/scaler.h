#ifndef SCALER_H_
#define SCALER_H_
typedef enum {
    PIC_DEC_DIR_0 = 0,
    PIC_DEC_DIR_90,
    PIC_DEC_DIR_180,
    PIC_DEC_DIR_270
} pic_dir_t;

typedef struct {
    unsigned char  *luma_top_adr;
    unsigned char  *chro_v_top_adr;
    unsigned char  *chro_u_top_adr;

    unsigned char  *luma_bottom_adr;
    unsigned char  *chro_v_bottom_adr;
    unsigned char  *chro_u_bottom_adr;
} scaler_buf_t;

typedef struct {
    unsigned        pic_top;
    unsigned        pic_left;
    unsigned        pic_width;
    unsigned        pic_height;
    
    unsigned        frame_top;
    unsigned        frame_left;
    unsigned        frame_width;
    unsigned        frame_height;
                    
    unsigned        scaled_top;
    unsigned        scaled_left;
    unsigned        scaled_width;
    unsigned        scaled_height;

    unsigned        lsd_set_flg;
    unsigned        scaled_lsd_top;
    unsigned        scaled_lsd_left;
    unsigned        scaled_lsd_width;
    unsigned        scaled_lsd_height;
    float           lsd_ratio_x;
    float           lsd_ratio_y;
    unsigned    pic_relative_top;
    unsigned    pic_relative_left;
                    
    unsigned 		screen_width;
    unsigned 		screen_height;
    unsigned char	ratio_method;    
    float 			display_ratio;
                    
    unsigned        hd_flag;            /* hd output flag */
    unsigned        output_bt;          /* output up-down sequence */
    pic_dir_t       rotation;           /* picture output rotation */

    unsigned char*    output;        /* output buffer */
    int bpp;
    int             area;
    unsigned        flags;
                                        /* pixel output function */
    unsigned        config_width;       /* display device parameter*/
    unsigned        config_height;
    unsigned 		config_screen_width;
    unsigned 		config_screen_height;                                       
                                        
    int             crop_policy;                /*for cropping , the percent of top cut off */
    unsigned        max_crop;                   /*the maximum percent of crop, high 16bit/low 16bit*/
} scaler_input_t;
#define INTI_FRAME_WIDTH 1280
#define INIT_FRAME_HEIGHT 720

#define INTI_SCREEN_WIDTH 4
#define INIT_SCREEN_HEIGHT 3
typedef struct simp_scaler_s {
    scaler_input_t  input;
    void (*pixel_write)(struct simp_scaler_s *scaler, unsigned char r,unsigned char g, 
                            unsigned char b,unsigned char a, int image_x, int image_y);
    unsigned short *map_x;
    unsigned short *map_y;
} simp_scaler_t;

int     simp_scaler_init(simp_scaler_t *scaler);
void    simp_scaler_output_pixel(   simp_scaler_t *scaler, unsigned char r, unsigned char g,
                                    unsigned char b, unsigned char a ,int image_x, int image_y);
void    simp_scaler_flush(simp_scaler_t *scaler);
void    simp_scaler_destroy(simp_scaler_t *scaler);
extern void     get_canvas(int* x,int* y);
extern void     set_canvas(int w , int h);
extern void pre_out_scaler();

extern void post_out_scaler();

#define OUT_BEGIN pre_out_scaler();
#define OUT_END	post_out_scaler();
#define PIC_SCALER
#endif
