#ifndef RAW_H_
#define RAW_H
#include "amlraw.h"
  static const struct {
    int fsize;
    char make[12], model[19], withjpeg;
  } table[] = {
    {    62464, "Kodak",    "DC20"            ,0 },
    {   124928, "Kodak",    "DC20"            ,0 },
    {  1652736, "Kodak",    "DCS200"          ,0 },
    {  4159302, "Kodak",    "C330"            ,0 },
    {  4162462, "Kodak",    "C330"            ,0 },
    {   460800, "Kodak",    "C603v"           ,0 },
    {   614400, "Kodak",    "C603v"           ,0 },
    {  6163328, "Kodak",    "C603"            ,0 },
    {  6166488, "Kodak",    "C603"            ,0 },
    {  9116448, "Kodak",    "C603y"           ,0 },
    {   311696, "ST Micro", "STV680 VGA"      ,0 },  /* SPYz */
    {   787456, "Creative", "PC-CAM 600"      ,0 },
    {  1138688, "Minolta",  "RD175"           ,0 },
    {  3840000, "Foculus",  "531C"            ,0 },
    {   786432, "AVT",      "F-080C"          ,0 },
    {  1447680, "AVT",      "F-145C"          ,0 },
    {  1920000, "AVT",      "F-201C"          ,0 },
    {  5067304, "AVT",      "F-510C"          ,0 },
    { 10134608, "AVT",      "F-510C"          ,0 },
    { 16157136, "AVT",      "F-810C"          ,0 },
    {  1409024, "Sony",     "XCD-SX910CR"     ,0 },
    {  2818048, "Sony",     "XCD-SX910CR"     ,0 },
    {  3884928, "Micron",   "2010"            ,0 },
    {  6624000, "Pixelink", "A782"            ,0 },
    { 13248000, "Pixelink", "A782"            ,0 },
    {  6291456, "RoverShot","3320AF"          ,0 },
    {  6553440, "Canon",    "PowerShot A460"  ,0 },
    {  6653280, "Canon",    "PowerShot A530"  ,0 },
    {  6573120, "Canon",    "PowerShot A610"  ,0 },
    {  9219600, "Canon",    "PowerShot A620"  ,0 },
    {  9243240, "Canon",    "PowerShot A470"  ,0 },
    { 10341600, "Canon",    "PowerShot A720"  ,0 },
    { 10383120, "Canon",    "PowerShot A630"  ,0 },
    { 12945240, "Canon",    "PowerShot A640"  ,0 },
    { 15636240, "Canon",    "PowerShot A650"  ,0 },
    {  5298000, "Canon",    "PowerShot SD300" ,0 },
    {  7710960, "Canon",    "PowerShot S3 IS" ,0 },
    { 15467760, "Canon",    "PowerShot SX110 IS",0 },
    {  5939200, "OLYMPUS",  "C770UZ"          ,0 },
    {  1581060, "NIKON",    "E900"            ,1 },  /* or E900s,E910 */
    {  2465792, "NIKON",    "E950"            ,1 },  /* or E800,E700 */
    {  2940928, "NIKON",    "E2100"           ,1 },  /* or E2500 */
    {  4771840, "NIKON",    "E990"            ,1 },  /* or E995, Oly C3030Z */
    {  4775936, "NIKON",    "E3700"           ,1 },  /* or Optio 33WR */
    {  5869568, "NIKON",    "E4300"           ,1 },  /* or DiMAGE Z2 */
    {  5865472, "NIKON",    "E4500"           ,1 },
    {  7438336, "NIKON",    "E5000"           ,1 },  /* or E5700 */
    {  8998912, "NIKON",    "COOLPIX S6"      ,1 },
    {  1976352, "CASIO",    "QV-2000UX"       ,1 },
    {  3217760, "CASIO",    "QV-3*00EX"       ,1 },
    {  6218368, "CASIO",    "QV-5700"         ,1 },
    {  6054400, "CASIO",    "QV-R41"          ,1 },
    {  7530816, "CASIO",    "QV-R51"          ,1 },
    {  7684000, "CASIO",    "QV-4000"         ,1 },
    {  2937856, "CASIO",    "EX-S20"          ,1 },
    {  4948608, "CASIO",    "EX-S100"         ,1 },
    {  7542528, "CASIO",    "EX-Z50"          ,1 },
    {  7753344, "CASIO",    "EX-Z55"          ,1 },
    {  7816704, "CASIO",    "EX-Z60"          ,1 },
    { 10843712, "CASIO",    "EX-Z75"          ,1 },
    { 12310144, "CASIO",    "EX-Z850"         ,1 },
    {  7426656, "CASIO",    "EX-P505"         ,1 },
    {  9313536, "CASIO",    "EX-P600"         ,1 },
    { 10979200, "CASIO",    "EX-P700"         ,1 },
    {  3178560, "PENTAX",   "Optio S"         ,1 },
    {  4841984, "PENTAX",   "Optio S"         ,1 },
    {  6114240, "PENTAX",   "Optio S4"        ,1 },  /* or S4i, CASIO EX-Z4 */
    { 10702848, "PENTAX",   "Optio 750Z"      ,1 },
    { 15980544, "AGFAPHOTO","DC-833m"         ,1 },
    { 16098048, "SAMSUNG",  "S85"             ,1 },
    { 16215552, "SAMSUNG",  "S85"             ,1 },
    { 12582980, "Sinar",    ""                ,0 },
    { 33292868, "Sinar",    ""                ,0 },
    { 44390468, "Sinar",    ""                ,0 } };
  static const char *corp[] =
    { "Canon", "NIKON", "EPSON", "KODAK", "Kodak", "OLYMPUS", "PENTAX",
      "MINOLTA", "Minolta", "Konica", "CASIO", "Sinar", "Phase One",
      "SAMSUNG", "Mamiya", "MOTOROLA" };
      
typedef struct {
    unsigned    offset;
    unsigned    length;
} raw_stripe_t;

typedef enum {
    RAW_TYPE_BILEVEL,
    RAW_TYPE_GRAYSCALE,
    RAW_TYPE_PALETTE,
    RAW_TYPE_RGB
} raw_type_t;

typedef enum {
    RAW_PARSE_HEADER = 0,
    RAW_PARSE_IFD,
    RAW_PARSE_IFD_ENTRY,
    RAW_PARSE_DATA,
    RAW_PROCESS_GRAYSCALE,
    RAW_PROCESS_PALETTE,
    RAW_PROCESS_RGB
} raw_state_t;

typedef struct {
//    tiffend_t       endian;             /* endian */
	unsigned		cur_data_offset;
    unsigned        ifd_offset;         /* ifd offset */
    unsigned        ifd_num;            /* ifd numbers */
    unsigned        width;              /* picture width */
    unsigned        height;             /* picture height */
    unsigned        bits_per_sample;    /* sample resolution */
    unsigned        samples_per_pixel;  /* components number */
    unsigned        compression;        /* compression method */
    unsigned        pi;                 /* PhotometricInterpretation */
    unsigned        rows_per_strip;     /* how many rows in each strip */
    raw_stripe_t  *strip_info;         /* strip position and length */
    unsigned        strip_counts;       /* strips counts */
    unsigned        color_map_offset;
    unsigned char   palette[256][3];    /* color palette */
    unsigned        palette_num;        /* palete entries number */
    raw_type_t     type;
    int line_mode;                     /* bit 0: 1 is column first. bit 9 pixel order, 1 is desc mode.*/
    int buf_width;                     /* width of buffer for decoding. */
} RawInfo_t;

typedef void (*decoder_func_t)(void);

typedef struct {

    int             irqhandler;
    int             decoder_state;
    unsigned        pic_height;
    unsigned        pic_width;
    unsigned        format;
    unsigned        baserate;

    raw_state_t     raw_state;                     /*  data consumer state */
    unsigned        error_code;                     /* decoder error code */
    unsigned        cur_stripe;
    unsigned        stripe_offset;
    unsigned        stripe_rows;

    decoder_func_t  decoder_funcs[6];               /* decoder funcs */
    unsigned char  *line_buffer;                    /* line buffer */

	int 			driver_taskid;                  /* driver task ID */
	unsigned        driver_prio;                    /* driver task priority */
	int             scaler_task_safe2kill;          /* scaler task can be safely killed */

	unsigned		state_cb_arg;                   /* decoder status callback parameter */

    unsigned short  cur_x;
    unsigned short  cur_y;
    unsigned short  decode_percent;

    unsigned		target_top;						/* final picture screen positions */
	unsigned		target_left;
	unsigned		target_width;
	unsigned		target_height;

	
	unsigned short  cur_in_h_start;
	unsigned short  cur_in_v_start;
	unsigned short  cur_in_v_show_end;  /* end of data decoded for show. */
	unsigned short  cur_in_v_end;
	unsigned short  cur_in_h_size;
	unsigned short  cur_in_v_size; 
	unsigned short  cur_out_v_start;
	unsigned short  cur_out_v_size;  
	unsigned short  total_v_size;
    unsigned short  scan_start;
    unsigned short  scan_end;
    int black;
    double dark[2] ; 
    unsigned maximum;
    unsigned short  *pixel;
	int             transition_mode;

	unsigned		frame_top;						/* display frame on screen */
	unsigned		frame_left;
	unsigned		frame_width;
	unsigned		frame_height;
	unsigned		area;                           /* display buffer selector */
	pic_dir_t		rotation;                       /* display direction control */

    unsigned        use_bilinear;
    simp_scaler_t   simp_scaler;                    /* output scaler */
    unsigned        flags;
    unsigned        post_process_cfg;
    unsigned        config_width;       /* display device parameter*/
    unsigned        config_height;
    unsigned 		config_screen_width;
    unsigned 		config_screen_height;
} RawDecMgr_t;
#define next_word(n)     ((rawinfo.endian == END_BIG) ?  \
                        (((unsigned)(*(n)) << 8) | ((unsigned)(*((n)+1)))) : \
                        (((unsigned)(*(n))) | ((unsigned)(*((n)+1)) << 8)))
#define next_dword(n)    ((rawinfo.endian == END_BIG) ?  \
                        (((unsigned)(*(n)) << 24) | ((unsigned)(*((n)+1)) << 16) | ((unsigned)(*((n)+2)) << 8) | ((unsigned)(*((n)+3)))) : \
                        ((unsigned)(*(n)) | ((unsigned)(*((n)+1)) << 8) | ((unsigned)(*((n)+2) << 16)) | ((unsigned)(*((n)+3) << 24))))
#endif