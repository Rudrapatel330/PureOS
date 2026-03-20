# 1 "mupdf/thirdparty/libjpeg/jaricom.c"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "mupdf/thirdparty/libjpeg/jaricom.c"
# 17 "mupdf/thirdparty/libjpeg/jaricom.c"
# 1 "mupdf/thirdparty/libjpeg/jinclude.h" 1
# 21 "mupdf/thirdparty/libjpeg/jinclude.h"
# 1 "include/jconfig.h" 1
# 22 "mupdf/thirdparty/libjpeg/jinclude.h" 2
# 36 "mupdf/thirdparty/libjpeg/jinclude.h"
# 1 "d:\\1os copy\\animations_working\\1os\\tools\\lib\\gcc\\x86_64-elf\\7.1.0\\include\\stddef.h" 1 3 4
# 149 "d:\\1os copy\\animations_working\\1os\\tools\\lib\\gcc\\x86_64-elf\\7.1.0\\include\\stddef.h" 3 4

# 149 "d:\\1os copy\\animations_working\\1os\\tools\\lib\\gcc\\x86_64-elf\\7.1.0\\include\\stddef.h" 3 4
typedef long int ptrdiff_t;
# 216 "d:\\1os copy\\animations_working\\1os\\tools\\lib\\gcc\\x86_64-elf\\7.1.0\\include\\stddef.h" 3 4
typedef long unsigned int size_t;
# 328 "d:\\1os copy\\animations_working\\1os\\tools\\lib\\gcc\\x86_64-elf\\7.1.0\\include\\stddef.h" 3 4
typedef int wchar_t;
# 426 "d:\\1os copy\\animations_working\\1os\\tools\\lib\\gcc\\x86_64-elf\\7.1.0\\include\\stddef.h" 3 4
typedef struct {
  long long __max_align_ll __attribute__((__aligned__(__alignof__(long long))));
  long double __max_align_ld __attribute__((__aligned__(__alignof__(long double))));
# 437 "d:\\1os copy\\animations_working\\1os\\tools\\lib\\gcc\\x86_64-elf\\7.1.0\\include\\stddef.h" 3 4
} max_align_t;
# 37 "mupdf/thirdparty/libjpeg/jinclude.h" 2



# 1 "include/stdlib.h" 1



# 1 "d:\\1os copy\\animations_working\\1os\\tools\\lib\\gcc\\x86_64-elf\\7.1.0\\include\\stddef.h" 1 3 4
# 5 "include/stdlib.h" 2
# 1 "d:\\1os copy\\animations_working\\1os\\tools\\lib\\gcc\\x86_64-elf\\7.1.0\\include\\stdint.h" 1 3 4
# 11 "d:\\1os copy\\animations_working\\1os\\tools\\lib\\gcc\\x86_64-elf\\7.1.0\\include\\stdint.h" 3 4
# 1 "d:\\1os copy\\animations_working\\1os\\tools\\lib\\gcc\\x86_64-elf\\7.1.0\\include\\stdint-gcc.h" 1 3 4
# 34 "d:\\1os copy\\animations_working\\1os\\tools\\lib\\gcc\\x86_64-elf\\7.1.0\\include\\stdint-gcc.h" 3 4
typedef signed char int8_t;


typedef short int int16_t;


typedef int int32_t;


typedef long int int64_t;


typedef unsigned char uint8_t;


typedef short unsigned int uint16_t;


typedef unsigned int uint32_t;


typedef long unsigned int uint64_t;




typedef signed char int_least8_t;
typedef short int int_least16_t;
typedef int int_least32_t;
typedef long int int_least64_t;
typedef unsigned char uint_least8_t;
typedef short unsigned int uint_least16_t;
typedef unsigned int uint_least32_t;
typedef long unsigned int uint_least64_t;



typedef int int_fast8_t;
typedef int int_fast16_t;
typedef int int_fast32_t;
typedef long int int_fast64_t;
typedef unsigned int uint_fast8_t;
typedef unsigned int uint_fast16_t;
typedef unsigned int uint_fast32_t;
typedef long unsigned int uint_fast64_t;




typedef long int intptr_t;


typedef long unsigned int uintptr_t;




typedef long int intmax_t;
typedef long unsigned int uintmax_t;
# 12 "d:\\1os copy\\animations_working\\1os\\tools\\lib\\gcc\\x86_64-elf\\7.1.0\\include\\stdint.h" 2 3 4
# 6 "include/stdlib.h" 2





# 10 "include/stdlib.h"
extern void exit(int status) __attribute__((noreturn));

extern void *kmalloc(size_t size);
extern void kfree(void *ptr);

extern void *krealloc(void *ptr, size_t size);

extern void *malloc(size_t size);
extern void free(void *ptr);
static inline void *realloc(void *ptr, size_t size) {
  return krealloc(ptr, size);
}
static inline void *calloc(size_t nmemb, size_t size) {
  void *ptr = malloc(nmemb * size);
  if (ptr) {
    char *p = (char *)ptr;
    for (size_t i = 0; i < nmemb * size; i++)
      p[i] = 0;
  }
  return ptr;
}
# 41 "mupdf/thirdparty/libjpeg/jinclude.h" 2






# 1 "include/stdio.h" 1



# 1 "d:\\1os copy\\animations_working\\1os\\tools\\lib\\gcc\\x86_64-elf\\7.1.0\\include\\stdarg.h" 1 3 4
# 40 "d:\\1os copy\\animations_working\\1os\\tools\\lib\\gcc\\x86_64-elf\\7.1.0\\include\\stdarg.h" 3 4

# 40 "d:\\1os copy\\animations_working\\1os\\tools\\lib\\gcc\\x86_64-elf\\7.1.0\\include\\stdarg.h" 3 4
typedef __builtin_va_list __gnuc_va_list;
# 99 "d:\\1os copy\\animations_working\\1os\\tools\\lib\\gcc\\x86_64-elf\\7.1.0\\include\\stdarg.h" 3 4
typedef __gnuc_va_list va_list;
# 5 "include/stdio.h" 2
# 1 "d:\\1os copy\\animations_working\\1os\\tools\\lib\\gcc\\x86_64-elf\\7.1.0\\include\\stddef.h" 1 3 4
# 6 "include/stdio.h" 2

# 1 "include/sys/types.h" 1



# 1 "d:\\1os copy\\animations_working\\1os\\tools\\lib\\gcc\\x86_64-elf\\7.1.0\\include\\stddef.h" 1 3 4
# 5 "include/sys/types.h" 2


# 6 "include/sys/types.h"
typedef long off_t;
typedef int pid_t;
typedef long ssize_t;

typedef unsigned char u_int8_t;
typedef unsigned short u_int16_t;
typedef unsigned int u_int32_t;
typedef unsigned long long u_int64_t;
# 8 "include/stdio.h" 2







typedef struct {
  int fd;
  int err;
} FILE;

extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;

int printf(const char *format, ...);
int sprintf(char *str, const char *format, ...);
int fprintf(FILE *stream, const char *format, ...);
int vfprintf(FILE *stream, const char *format, va_list ap);

FILE *fopen(const char *pathname, const char *mode);
int fclose(FILE *stream);
size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);
int fseek(FILE *stream, long offset, int whence);
long ftell(FILE *stream);
int fgetc(FILE *stream);
int ferror(FILE *stream);

typedef struct {
  char name[128];
  uint32_t size;
  uint8_t flags;
} dirent_t;

int open(const char *pathname, int flags);
int close(int fd);
ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);
dirent_t *readdir(int fd, int index);
void exit(int status);
# 48 "mupdf/thirdparty/libjpeg/jinclude.h" 2
# 67 "mupdf/thirdparty/libjpeg/jinclude.h"
# 1 "src/kernel/string.h" 1




# 1 "d:\\1os copy\\animations_working\\1os\\tools\\lib\\gcc\\x86_64-elf\\7.1.0\\include\\stddef.h" 1 3 4
# 6 "src/kernel/string.h" 2


int abs(int n);
int strcmp(const char *s1, const char *s2);
size_t strlen(const char *str);
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t n);
void *memset(void *s, int c, size_t n);
void *memcpy(void *dest, const void *src, size_t n);
void *memmove(void *dest, const void *src, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);
char *strchr(const char *s, int c);
int strncmp(const char *s1, const char *s2, size_t n);
char *strcat(char *dest, const char *src);
char *strncat(char *dest, const char *src, size_t n);
char *strstr(const char *haystack, const char *needle);
void k_itoa(int n, char *s);
int atoi(const char *s);
void k_itoa_hex(uint32_t n, char *s);
char *strtok(char *str, const char *delim);
void memset16(void *dest, uint16_t val, size_t count);
int vsnprintf(char *buf, size_t size, const char *fmt, va_list args);
int snprintf(char *buf, size_t size, const char *fmt, ...);
# 68 "mupdf/thirdparty/libjpeg/jinclude.h" 2
# 18 "mupdf/thirdparty/libjpeg/jaricom.c" 2
# 1 "mupdf/thirdparty/libjpeg/jpeglib.h" 1
# 27 "mupdf/thirdparty/libjpeg/jpeglib.h"
# 1 "mupdf/thirdparty/libjpeg/jmorecfg.h" 1
# 69 "mupdf/thirdparty/libjpeg/jmorecfg.h"
typedef unsigned char JSAMPLE;
# 151 "mupdf/thirdparty/libjpeg/jmorecfg.h"
typedef short JCOEF;
# 162 "mupdf/thirdparty/libjpeg/jmorecfg.h"
typedef unsigned char JOCTET;
# 187 "mupdf/thirdparty/libjpeg/jmorecfg.h"
typedef unsigned char UINT8;
# 199 "mupdf/thirdparty/libjpeg/jmorecfg.h"
typedef unsigned short UINT16;







typedef short INT16;
# 216 "mupdf/thirdparty/libjpeg/jmorecfg.h"
typedef long INT32;
# 229 "mupdf/thirdparty/libjpeg/jmorecfg.h"
typedef unsigned int JDIMENSION;
# 280 "mupdf/thirdparty/libjpeg/jmorecfg.h"
typedef void noreturn_t;
# 317 "mupdf/thirdparty/libjpeg/jmorecfg.h"
typedef enum { FALSE = 0, TRUE = 1 } boolean;
# 28 "mupdf/thirdparty/libjpeg/jpeglib.h" 2
# 75 "mupdf/thirdparty/libjpeg/jpeglib.h"
typedef JSAMPLE *JSAMPROW;
typedef JSAMPROW *JSAMPARRAY;
typedef JSAMPARRAY *JSAMPIMAGE;

typedef JCOEF JBLOCK[64];
typedef JBLOCK *JBLOCKROW;
typedef JBLOCKROW *JBLOCKARRAY;
typedef JBLOCKARRAY *JBLOCKIMAGE;

typedef JCOEF *JCOEFPTR;







typedef struct {




  UINT16 quantval[64];





  boolean sent_table;
} JQUANT_TBL;




typedef struct {

  UINT8 bits[17];

  UINT8 huffval[256];





  boolean sent_table;
} JHUFF_TBL;




typedef struct {



  int component_id;
  int component_index;
  int h_samp_factor;
  int v_samp_factor;
  int quant_tbl_no;




  int dc_tbl_no;
  int ac_tbl_no;
# 148 "mupdf/thirdparty/libjpeg/jpeglib.h"
  JDIMENSION width_in_blocks;
  JDIMENSION height_in_blocks;





  int DCT_h_scaled_size;
  int DCT_v_scaled_size;







  JDIMENSION downsampled_width;
  JDIMENSION downsampled_height;
# 174 "mupdf/thirdparty/libjpeg/jpeglib.h"
  boolean component_needed;



  int MCU_width;
  int MCU_height;
  int MCU_blocks;
  int MCU_sample_width;
  int last_col_width;
  int last_row_height;





  JQUANT_TBL * quant_table;


  void * dct_table;
} jpeg_component_info;




typedef struct {
  int comps_in_scan;
  int component_index[4];
  int Ss, Se;
  int Ah, Al;
} jpeg_scan_info;



typedef struct jpeg_marker_struct * jpeg_saved_marker_ptr;

struct jpeg_marker_struct {
  jpeg_saved_marker_ptr next;
  UINT8 marker;
  unsigned int original_length;
  unsigned int data_length;
  JOCTET * data;

};



typedef enum {
 JCS_UNKNOWN,
 JCS_GRAYSCALE,
 JCS_RGB,
 JCS_YCbCr,
 JCS_CMYK,
 JCS_YCCK,
 JCS_BG_RGB,
 JCS_BG_YCC
} J_COLOR_SPACE;



typedef enum {
 JCT_NONE = 0,
 JCT_SUBTRACT_GREEN = 1
} J_COLOR_TRANSFORM;



typedef enum {
 JDCT_ISLOW,
 JDCT_IFAST,
 JDCT_FLOAT
} J_DCT_METHOD;
# 255 "mupdf/thirdparty/libjpeg/jpeglib.h"
typedef enum {
 JDITHER_NONE,
 JDITHER_ORDERED,
 JDITHER_FS
} J_DITHER_MODE;
# 276 "mupdf/thirdparty/libjpeg/jpeglib.h"
struct jpeg_common_struct {
  struct jpeg_error_mgr * err; struct jpeg_memory_mgr * mem; struct jpeg_progress_mgr * progress; void * client_data; boolean is_decompressor; int global_state;




};

typedef struct jpeg_common_struct * j_common_ptr;
typedef struct jpeg_compress_struct * j_compress_ptr;
typedef struct jpeg_decompress_struct * j_decompress_ptr;




struct jpeg_compress_struct {
  struct jpeg_error_mgr * err; struct jpeg_memory_mgr * mem; struct jpeg_progress_mgr * progress; void * client_data; boolean is_decompressor; int global_state;


  struct jpeg_destination_mgr * dest;






  JDIMENSION image_width;
  JDIMENSION image_height;
  int input_components;
  J_COLOR_SPACE in_color_space;

  double input_gamma;
# 317 "mupdf/thirdparty/libjpeg/jpeglib.h"
  unsigned int scale_num, scale_denom;

  JDIMENSION jpeg_width;
  JDIMENSION jpeg_height;







  int data_precision;

  int num_components;
  J_COLOR_SPACE jpeg_color_space;

  jpeg_component_info * comp_info;


  JQUANT_TBL * quant_tbl_ptrs[4];
  int q_scale_factor[4];




  JHUFF_TBL * dc_huff_tbl_ptrs[4];
  JHUFF_TBL * ac_huff_tbl_ptrs[4];


  UINT8 arith_dc_L[16];
  UINT8 arith_dc_U[16];
  UINT8 arith_ac_K[16];

  int num_scans;
  const jpeg_scan_info * scan_info;





  boolean raw_data_in;
  boolean arith_code;
  boolean optimize_coding;
  boolean CCIR601_sampling;
  boolean do_fancy_downsampling;
  int smoothing_factor;
  J_DCT_METHOD dct_method;






  unsigned int restart_interval;
  int restart_in_rows;



  boolean write_JFIF_header;
  UINT8 JFIF_major_version;
  UINT8 JFIF_minor_version;




  UINT8 density_unit;
  UINT16 X_density;
  UINT16 Y_density;
  boolean write_Adobe_marker;

  J_COLOR_TRANSFORM color_transform;







  JDIMENSION next_scanline;
# 404 "mupdf/thirdparty/libjpeg/jpeglib.h"
  boolean progressive_mode;
  int max_h_samp_factor;
  int max_v_samp_factor;

  int min_DCT_h_scaled_size;
  int min_DCT_v_scaled_size;

  JDIMENSION total_iMCU_rows;
# 422 "mupdf/thirdparty/libjpeg/jpeglib.h"
  int comps_in_scan;
  jpeg_component_info * cur_comp_info[4];


  JDIMENSION MCUs_per_row;
  JDIMENSION MCU_rows_in_scan;

  int blocks_in_MCU;
  int MCU_membership[10];



  int Ss, Se, Ah, Al;

  int block_size;
  const int * natural_order;
  int lim_Se;




  struct jpeg_comp_master * master;
  struct jpeg_c_main_controller * main;
  struct jpeg_c_prep_controller * prep;
  struct jpeg_c_coef_controller * coef;
  struct jpeg_marker_writer * marker;
  struct jpeg_color_converter * cconvert;
  struct jpeg_downsampler * downsample;
  struct jpeg_forward_dct * fdct;
  struct jpeg_entropy_encoder * entropy;
  jpeg_scan_info * script_space;
  int script_space_size;
};




struct jpeg_decompress_struct {
  struct jpeg_error_mgr * err; struct jpeg_memory_mgr * mem; struct jpeg_progress_mgr * progress; void * client_data; boolean is_decompressor; int global_state;


  struct jpeg_source_mgr * src;




  JDIMENSION image_width;
  JDIMENSION image_height;
  int num_components;
  J_COLOR_SPACE jpeg_color_space;






  J_COLOR_SPACE out_color_space;

  unsigned int scale_num, scale_denom;

  double output_gamma;

  boolean buffered_image;
  boolean raw_data_out;

  J_DCT_METHOD dct_method;
  boolean do_fancy_upsampling;
  boolean do_block_smoothing;

  boolean quantize_colors;

  J_DITHER_MODE dither_mode;
  boolean two_pass_quantize;
  int desired_number_of_colors;

  boolean enable_1pass_quant;
  boolean enable_external_quant;
  boolean enable_2pass_quant;







  JDIMENSION output_width;
  JDIMENSION output_height;
  int out_color_components;
  int output_components;



  int rec_outbuf_height;
# 526 "mupdf/thirdparty/libjpeg/jpeglib.h"
  int actual_number_of_colors;
  JSAMPARRAY colormap;
# 537 "mupdf/thirdparty/libjpeg/jpeglib.h"
  JDIMENSION output_scanline;




  int input_scan_number;
  JDIMENSION input_iMCU_row;





  int output_scan_number;
  JDIMENSION output_iMCU_row;
# 559 "mupdf/thirdparty/libjpeg/jpeglib.h"
  int (*coef_bits)[64];
# 570 "mupdf/thirdparty/libjpeg/jpeglib.h"
  JQUANT_TBL * quant_tbl_ptrs[4];


  JHUFF_TBL * dc_huff_tbl_ptrs[4];
  JHUFF_TBL * ac_huff_tbl_ptrs[4];






  int data_precision;

  jpeg_component_info * comp_info;


  boolean is_baseline;
  boolean progressive_mode;
  boolean arith_code;

  UINT8 arith_dc_L[16];
  UINT8 arith_dc_U[16];
  UINT8 arith_ac_K[16];

  unsigned int restart_interval;




  boolean saw_JFIF_marker;

  UINT8 JFIF_major_version;
  UINT8 JFIF_minor_version;
  UINT8 density_unit;
  UINT16 X_density;
  UINT16 Y_density;
  boolean saw_Adobe_marker;
  UINT8 Adobe_transform;

  J_COLOR_TRANSFORM color_transform;


  boolean CCIR601_sampling;





  jpeg_saved_marker_ptr marker_list;
# 627 "mupdf/thirdparty/libjpeg/jpeglib.h"
  int max_h_samp_factor;
  int max_v_samp_factor;

  int min_DCT_h_scaled_size;
  int min_DCT_v_scaled_size;

  JDIMENSION total_iMCU_rows;
# 642 "mupdf/thirdparty/libjpeg/jpeglib.h"
  JSAMPLE * sample_range_limit;






  int comps_in_scan;
  jpeg_component_info * cur_comp_info[4];


  JDIMENSION MCUs_per_row;
  JDIMENSION MCU_rows_in_scan;

  int blocks_in_MCU;
  int MCU_membership[10];



  int Ss, Se, Ah, Al;



  int block_size;
  const int * natural_order;
  int lim_Se;





  int unread_marker;




  struct jpeg_decomp_master * master;
  struct jpeg_d_main_controller * main;
  struct jpeg_d_coef_controller * coef;
  struct jpeg_d_post_controller * post;
  struct jpeg_input_controller * inputctl;
  struct jpeg_marker_reader * marker;
  struct jpeg_entropy_decoder * entropy;
  struct jpeg_inverse_dct * idct;
  struct jpeg_upsampler * upsample;
  struct jpeg_color_deconverter * cconvert;
  struct jpeg_color_quantizer * cquantize;
};
# 702 "mupdf/thirdparty/libjpeg/jpeglib.h"
struct jpeg_error_mgr {

  noreturn_t (*error_exit) (j_common_ptr cinfo);

  void (*emit_message) (j_common_ptr cinfo, int msg_level);

  void (*output_message) (j_common_ptr cinfo);

  void (*format_message) (j_common_ptr cinfo, char * buffer);


  void (*reset_error_mgr) (j_common_ptr cinfo);




  int msg_code;

  union {
    int i[8];
    char s[80];
  } msg_parm;



  int trace_level;







  long num_warnings;
# 747 "mupdf/thirdparty/libjpeg/jpeglib.h"
  const char * const * jpeg_message_table;
  int last_jpeg_message;



  const char * const * addon_message_table;
  int first_addon_message;
  int last_addon_message;
};




struct jpeg_progress_mgr {
  void (*progress_monitor) (j_common_ptr cinfo);

  long pass_counter;
  long pass_limit;
  int completed_passes;
  int total_passes;
};




struct jpeg_destination_mgr {
  JOCTET * next_output_byte;
  size_t free_in_buffer;

  void (*init_destination) (j_compress_ptr cinfo);
  boolean (*empty_output_buffer) (j_compress_ptr cinfo);
  void (*term_destination) (j_compress_ptr cinfo);
};




struct jpeg_source_mgr {
  const JOCTET * next_input_byte;
  size_t bytes_in_buffer;

  void (*init_source) (j_decompress_ptr cinfo);
  boolean (*fill_input_buffer) (j_decompress_ptr cinfo);
  void (*skip_input_data) (j_decompress_ptr cinfo, long num_bytes);
  boolean (*resync_to_restart) (j_decompress_ptr cinfo, int desired);
  void (*term_source) (j_decompress_ptr cinfo);
};
# 811 "mupdf/thirdparty/libjpeg/jpeglib.h"
typedef struct jvirt_sarray_control * jvirt_sarray_ptr;
typedef struct jvirt_barray_control * jvirt_barray_ptr;


struct jpeg_memory_mgr {

  void * (*alloc_small) (j_common_ptr cinfo, int pool_id, size_t sizeofobject)
                         ;
  void * (*alloc_large) (j_common_ptr cinfo, int pool_id, size_t sizeofobject)
                              ;
  JSAMPARRAY (*alloc_sarray) (j_common_ptr cinfo, int pool_id, JDIMENSION samplesperrow, JDIMENSION numrows)

                             ;
  JBLOCKARRAY (*alloc_barray) (j_common_ptr cinfo, int pool_id, JDIMENSION blocksperrow, JDIMENSION numrows)

                              ;
  jvirt_sarray_ptr (*request_virt_sarray) (j_common_ptr cinfo, int pool_id, boolean pre_zero, JDIMENSION samplesperrow, JDIMENSION numrows, JDIMENSION maxaccess)




                              ;
  jvirt_barray_ptr (*request_virt_barray) (j_common_ptr cinfo, int pool_id, boolean pre_zero, JDIMENSION blocksperrow, JDIMENSION numrows, JDIMENSION maxaccess)




                              ;
  void (*realize_virt_arrays) (j_common_ptr cinfo);
  JSAMPARRAY (*access_virt_sarray) (j_common_ptr cinfo, jvirt_sarray_ptr ptr, JDIMENSION start_row, JDIMENSION num_rows, boolean writable)



                          ;
  JBLOCKARRAY (*access_virt_barray) (j_common_ptr cinfo, jvirt_barray_ptr ptr, JDIMENSION start_row, JDIMENSION num_rows, boolean writable)



                           ;
  void (*free_pool) (j_common_ptr cinfo, int pool_id);
  void (*self_destruct) (j_common_ptr cinfo);






  long max_memory_to_use;


  long max_alloc_chunk;
};





typedef boolean (*jpeg_marker_parser_method) (j_decompress_ptr cinfo);
# 949 "mupdf/thirdparty/libjpeg/jpeglib.h"
extern struct jpeg_error_mgr * jpeg_std_error
 (struct jpeg_error_mgr * err);
# 965 "mupdf/thirdparty/libjpeg/jpeglib.h"
extern void jpeg_CreateCompress (j_compress_ptr cinfo, int version, size_t structsize)
                                          ;
extern void jpeg_CreateDecompress (j_decompress_ptr cinfo, int version, size_t structsize)
                                     ;

extern void jpeg_destroy_compress (j_compress_ptr cinfo);
extern void jpeg_destroy_decompress (j_decompress_ptr cinfo);



extern void jpeg_stdio_dest (j_compress_ptr cinfo, FILE * outfile);
extern void jpeg_stdio_src (j_decompress_ptr cinfo, FILE * infile);


extern void jpeg_mem_dest (j_compress_ptr cinfo, unsigned char ** outbuffer, size_t * outsize)

                            ;
extern void jpeg_mem_src (j_decompress_ptr cinfo, const unsigned char * inbuffer, size_t insize)

                        ;


extern void jpeg_set_defaults (j_compress_ptr cinfo);

extern void jpeg_set_colorspace (j_compress_ptr cinfo, J_COLOR_SPACE colorspace)
                                    ;
extern void jpeg_default_colorspace (j_compress_ptr cinfo);
extern void jpeg_set_quality (j_compress_ptr cinfo, int quality, boolean force_baseline)
                               ;
extern void jpeg_set_linear_quality (j_compress_ptr cinfo, int scale_factor, boolean force_baseline)

                               ;
extern void jpeg_default_qtables (j_compress_ptr cinfo, boolean force_baseline)
                                   ;
extern void jpeg_add_quant_table (j_compress_ptr cinfo, int which_tbl, const unsigned int *basic_table, int scale_factor, boolean force_baseline)


                                   ;
extern int jpeg_quality_scaling (int quality);
extern void jpeg_simple_progression (j_compress_ptr cinfo);
extern void jpeg_suppress_tables (j_compress_ptr cinfo, boolean suppress)
                             ;
extern JQUANT_TBL * jpeg_alloc_quant_table (j_common_ptr cinfo);
extern JHUFF_TBL * jpeg_alloc_huff_table (j_common_ptr cinfo);
extern JHUFF_TBL * jpeg_std_huff_table (j_common_ptr cinfo, boolean isDC, int tblno)
                                   ;


extern void jpeg_start_compress (j_compress_ptr cinfo, boolean write_all_tables)
                                    ;
extern JDIMENSION jpeg_write_scanlines (j_compress_ptr cinfo, JSAMPARRAY scanlines, JDIMENSION num_lines)

                                ;
extern void jpeg_finish_compress (j_compress_ptr cinfo);


extern void jpeg_calc_jpeg_dimensions (j_compress_ptr cinfo);


extern JDIMENSION jpeg_write_raw_data (j_compress_ptr cinfo, JSAMPIMAGE data, JDIMENSION num_lines)

                               ;


extern void jpeg_write_marker
 (j_compress_ptr cinfo, int marker, const JOCTET * dataptr, unsigned int datalen)
                                                    ;

extern void jpeg_write_m_header
 (j_compress_ptr cinfo, int marker, unsigned int datalen);
extern void jpeg_write_m_byte
 (j_compress_ptr cinfo, int val);


extern void jpeg_write_tables (j_compress_ptr cinfo);


extern int jpeg_read_header (j_decompress_ptr cinfo, boolean require_image)
                             ;
# 1055 "mupdf/thirdparty/libjpeg/jpeglib.h"
extern boolean jpeg_start_decompress (j_decompress_ptr cinfo);
extern JDIMENSION jpeg_read_scanlines (j_decompress_ptr cinfo, JSAMPARRAY scanlines, JDIMENSION max_lines)

                               ;
extern boolean jpeg_finish_decompress (j_decompress_ptr cinfo);


extern JDIMENSION jpeg_read_raw_data (j_decompress_ptr cinfo, JSAMPIMAGE data, JDIMENSION max_lines)

                              ;


extern boolean jpeg_has_multiple_scans (j_decompress_ptr cinfo);
extern boolean jpeg_start_output (j_decompress_ptr cinfo, int scan_number)
                            ;
extern boolean jpeg_finish_output (j_decompress_ptr cinfo);
extern boolean jpeg_input_complete (j_decompress_ptr cinfo);
extern void jpeg_new_colormap (j_decompress_ptr cinfo);
extern int jpeg_consume_input (j_decompress_ptr cinfo);
# 1082 "mupdf/thirdparty/libjpeg/jpeglib.h"
extern void jpeg_core_output_dimensions (j_decompress_ptr cinfo);
extern void jpeg_calc_output_dimensions (j_decompress_ptr cinfo);


extern void jpeg_save_markers
 (j_decompress_ptr cinfo, int marker_code, unsigned int length_limit)
                                 ;


extern void jpeg_set_marker_processor
 (j_decompress_ptr cinfo, int marker_code, jpeg_marker_parser_method routine)
                                         ;


extern jvirt_barray_ptr * jpeg_read_coefficients (j_decompress_ptr cinfo);
extern void jpeg_write_coefficients (j_compress_ptr cinfo, jvirt_barray_ptr * coef_arrays)
                                       ;
extern void jpeg_copy_critical_parameters (j_decompress_ptr srcinfo, j_compress_ptr dstinfo)
                              ;







extern void jpeg_abort_compress (j_compress_ptr cinfo);
extern void jpeg_abort_decompress (j_decompress_ptr cinfo);




extern void jpeg_abort (j_common_ptr cinfo);
extern void jpeg_destroy (j_common_ptr cinfo);


extern boolean jpeg_resync_to_restart (j_decompress_ptr cinfo, int desired)
                      ;
# 1173 "mupdf/thirdparty/libjpeg/jpeglib.h"
# 1 "mupdf/thirdparty/libjpeg/jpegint.h" 1
# 17 "mupdf/thirdparty/libjpeg/jpegint.h"
typedef enum {
 JBUF_PASS_THRU,

 JBUF_SAVE_SOURCE,
 JBUF_CRANK_DEST,
 JBUF_SAVE_AND_PASS
} J_BUF_MODE;
# 46 "mupdf/thirdparty/libjpeg/jpegint.h"
struct jpeg_comp_master {
  void (*prepare_for_pass) (j_compress_ptr cinfo);
  void (*pass_startup) (j_compress_ptr cinfo);
  void (*finish_pass) (j_compress_ptr cinfo);


  boolean call_pass_startup;
  boolean is_last_pass;
};


struct jpeg_c_main_controller {
  void (*start_pass) (j_compress_ptr cinfo, J_BUF_MODE pass_mode);
  void (*process_data) (j_compress_ptr cinfo, JSAMPARRAY input_buf, JDIMENSION *in_row_ctr, JDIMENSION in_rows_avail)

                                    ;
};


struct jpeg_c_prep_controller {
  void (*start_pass) (j_compress_ptr cinfo, J_BUF_MODE pass_mode);
  void (*pre_process_data) (j_compress_ptr cinfo, JSAMPARRAY input_buf, JDIMENSION *in_row_ctr, JDIMENSION in_rows_avail, JSAMPIMAGE output_buf, JDIMENSION *out_row_group_ctr, JDIMENSION out_row_groups_avail)





                                        ;
};


struct jpeg_c_coef_controller {
  void (*start_pass) (j_compress_ptr cinfo, J_BUF_MODE pass_mode);
  boolean (*compress_data) (j_compress_ptr cinfo, JSAMPIMAGE input_buf)
                             ;
};


struct jpeg_color_converter {
  void (*start_pass) (j_compress_ptr cinfo);
  void (*color_convert) (j_compress_ptr cinfo, JSAMPARRAY input_buf, JSAMPIMAGE output_buf, JDIMENSION output_row, int num_rows)

                                         ;
};


struct jpeg_downsampler {
  void (*start_pass) (j_compress_ptr cinfo);
  void (*downsample) (j_compress_ptr cinfo, JSAMPIMAGE input_buf, JDIMENSION in_row_index, JSAMPIMAGE output_buf, JDIMENSION out_row_group_index)


                                        ;

  boolean need_context_rows;
};


typedef void (*forward_DCT_ptr) (j_compress_ptr cinfo, jpeg_component_info * compptr, JSAMPARRAY sample_data, JBLOCKROW coef_blocks, JDIMENSION start_col, JDIMENSION num_blocks)


                                                ;

struct jpeg_forward_dct {
  void (*start_pass) (j_compress_ptr cinfo);

  forward_DCT_ptr forward_DCT[10];
};


struct jpeg_entropy_encoder {
  void (*start_pass) (j_compress_ptr cinfo, boolean gather_statistics);
  boolean (*encode_mcu) (j_compress_ptr cinfo, JBLOCKARRAY MCU_data);
  void (*finish_pass) (j_compress_ptr cinfo);
};


struct jpeg_marker_writer {
  void (*write_file_header) (j_compress_ptr cinfo);
  void (*write_frame_header) (j_compress_ptr cinfo);
  void (*write_scan_header) (j_compress_ptr cinfo);
  void (*write_file_trailer) (j_compress_ptr cinfo);
  void (*write_tables_only) (j_compress_ptr cinfo);


  void (*write_marker_header) (j_compress_ptr cinfo, int marker, unsigned int datalen)
                                ;
  void (*write_marker_byte) (j_compress_ptr cinfo, int val);
};





struct jpeg_decomp_master {
  void (*prepare_for_output_pass) (j_decompress_ptr cinfo);
  void (*finish_output_pass) (j_decompress_ptr cinfo);


  boolean is_dummy_pass;
};


struct jpeg_input_controller {
  int (*consume_input) (j_decompress_ptr cinfo);
  void (*reset_input_controller) (j_decompress_ptr cinfo);
  void (*start_input_pass) (j_decompress_ptr cinfo);
  void (*finish_input_pass) (j_decompress_ptr cinfo);


  boolean has_multiple_scans;
  boolean eoi_reached;
};


struct jpeg_d_main_controller {
  void (*start_pass) (j_decompress_ptr cinfo, J_BUF_MODE pass_mode);
  void (*process_data) (j_decompress_ptr cinfo, JSAMPARRAY output_buf, JDIMENSION *out_row_ctr, JDIMENSION out_rows_avail)

                                     ;
};


struct jpeg_d_coef_controller {
  void (*start_input_pass) (j_decompress_ptr cinfo);
  int (*consume_data) (j_decompress_ptr cinfo);
  void (*start_output_pass) (j_decompress_ptr cinfo);
  int (*decompress_data) (j_decompress_ptr cinfo, JSAMPIMAGE output_buf)
                            ;

  jvirt_barray_ptr *coef_arrays;
};


struct jpeg_d_post_controller {
  void (*start_pass) (j_decompress_ptr cinfo, J_BUF_MODE pass_mode);
  void (*post_process_data) (j_decompress_ptr cinfo, JSAMPIMAGE input_buf, JDIMENSION *in_row_group_ctr, JDIMENSION in_row_groups_avail, JSAMPARRAY output_buf, JDIMENSION *out_row_ctr, JDIMENSION out_rows_avail)





                                   ;
};


struct jpeg_marker_reader {
  void (*reset_marker_reader) (j_decompress_ptr cinfo);




  int (*read_markers) (j_decompress_ptr cinfo);

  jpeg_marker_parser_method read_restart_marker;




  boolean saw_SOI;
  boolean saw_SOF;
  int next_restart_num;
  unsigned int discarded_bytes;
};


struct jpeg_entropy_decoder {
  void (*start_pass) (j_decompress_ptr cinfo);
  boolean (*decode_mcu) (j_decompress_ptr cinfo, JBLOCKARRAY MCU_data);
  void (*finish_pass) (j_decompress_ptr cinfo);
};


typedef void (*inverse_DCT_method_ptr) (j_decompress_ptr cinfo, jpeg_component_info * compptr, JCOEFPTR coef_block, JSAMPARRAY output_buf, JDIMENSION output_col)


                                                 ;

struct jpeg_inverse_dct {
  void (*start_pass) (j_decompress_ptr cinfo);

  inverse_DCT_method_ptr inverse_DCT[10];
};


struct jpeg_upsampler {
  void (*start_pass) (j_decompress_ptr cinfo);
  void (*upsample) (j_decompress_ptr cinfo, JSAMPIMAGE input_buf, JDIMENSION *in_row_group_ctr, JDIMENSION in_row_groups_avail, JSAMPARRAY output_buf, JDIMENSION *out_row_ctr, JDIMENSION out_rows_avail)





                                 ;

  boolean need_context_rows;
};


struct jpeg_color_deconverter {
  void (*start_pass) (j_decompress_ptr cinfo);
  void (*color_convert) (j_decompress_ptr cinfo, JSAMPIMAGE input_buf, JDIMENSION input_row, JSAMPARRAY output_buf, int num_rows)

                                         ;
};


struct jpeg_color_quantizer {
  void (*start_pass) (j_decompress_ptr cinfo, boolean is_pre_scan);
  void (*color_quantize) (j_decompress_ptr cinfo, JSAMPARRAY input_buf, JSAMPARRAY output_buf, int num_rows)

                   ;
  void (*finish_pass) (j_decompress_ptr cinfo);
  void (*new_color_map) (j_decompress_ptr cinfo);
};
# 379 "mupdf/thirdparty/libjpeg/jpegint.h"
extern void jinit_compress_master (j_compress_ptr cinfo);
extern void jinit_c_master_control (j_compress_ptr cinfo, boolean transcode_only)
                              ;
extern void jinit_c_main_controller (j_compress_ptr cinfo, boolean need_full_buffer)
                                 ;
extern void jinit_c_prep_controller (j_compress_ptr cinfo, boolean need_full_buffer)
                                 ;
extern void jinit_c_coef_controller (j_compress_ptr cinfo, boolean need_full_buffer)
                                 ;
extern void jinit_color_converter (j_compress_ptr cinfo);
extern void jinit_downsampler (j_compress_ptr cinfo);
extern void jinit_forward_dct (j_compress_ptr cinfo);
extern void jinit_huff_encoder (j_compress_ptr cinfo);
extern void jinit_arith_encoder (j_compress_ptr cinfo);
extern void jinit_marker_writer (j_compress_ptr cinfo);

extern void jinit_master_decompress (j_decompress_ptr cinfo);
extern void jinit_d_main_controller (j_decompress_ptr cinfo, boolean need_full_buffer)
                                 ;
extern void jinit_d_coef_controller (j_decompress_ptr cinfo, boolean need_full_buffer)
                                 ;
extern void jinit_d_post_controller (j_decompress_ptr cinfo, boolean need_full_buffer)
                                 ;
extern void jinit_input_controller (j_decompress_ptr cinfo);
extern void jinit_marker_reader (j_decompress_ptr cinfo);
extern void jinit_huff_decoder (j_decompress_ptr cinfo);
extern void jinit_arith_decoder (j_decompress_ptr cinfo);
extern void jinit_inverse_dct (j_decompress_ptr cinfo);
extern void jinit_upsampler (j_decompress_ptr cinfo);
extern void jinit_color_deconverter (j_decompress_ptr cinfo);
extern void jinit_1pass_quantizer (j_decompress_ptr cinfo);
extern void jinit_2pass_quantizer (j_decompress_ptr cinfo);
extern void jinit_merged_upsampler (j_decompress_ptr cinfo);

extern void jinit_memory_mgr (j_common_ptr cinfo);


extern long jdiv_round_up (long a, long b);
extern long jround_up (long a, long b);
extern void jcopy_sample_rows (JSAMPARRAY input_array, JSAMPARRAY output_array, int num_rows, JDIMENSION num_cols)

                                           ;
extern void jcopy_block_row (JBLOCKROW input_row, JBLOCKROW output_row, JDIMENSION num_blocks)
                             ;




extern const int jpeg_natural_order[];
extern const int jpeg_natural_order7[];
extern const int jpeg_natural_order6[];
extern const int jpeg_natural_order5[];
extern const int jpeg_natural_order4[];
extern const int jpeg_natural_order3[];
extern const int jpeg_natural_order2[];


extern const INT32 jpeg_aritab[];
# 1174 "mupdf/thirdparty/libjpeg/jpeglib.h" 2
# 1 "mupdf/thirdparty/libjpeg/jerror.h" 1
# 34 "mupdf/thirdparty/libjpeg/jerror.h"
typedef enum {





JMSG_NOMESSAGE ,


JERR_BAD_ALIGN_TYPE ,
JERR_BAD_ALLOC_CHUNK ,
JERR_BAD_BUFFER_MODE ,
JERR_BAD_COMPONENT_ID ,
JERR_BAD_CROP_SPEC ,
JERR_BAD_DCT_COEF ,
JERR_BAD_DCTSIZE ,
JERR_BAD_DROP_SAMPLING ,

JERR_BAD_HUFF_TABLE ,
JERR_BAD_IN_COLORSPACE ,
JERR_BAD_J_COLORSPACE ,
JERR_BAD_LENGTH ,
JERR_BAD_LIB_VERSION ,

JERR_BAD_MCU_SIZE ,
JERR_BAD_POOL_ID ,
JERR_BAD_PRECISION ,
JERR_BAD_PROGRESSION ,

JERR_BAD_PROG_SCRIPT ,

JERR_BAD_SAMPLING ,
JERR_BAD_SCAN_SCRIPT ,
JERR_BAD_STATE ,
JERR_BAD_STRUCT_SIZE ,

JERR_BAD_VIRTUAL_ACCESS ,
JERR_BUFFER_SIZE ,
JERR_CANT_SUSPEND ,
JERR_CCIR601_NOTIMPL ,
JERR_COMPONENT_COUNT ,
JERR_CONVERSION_NOTIMPL ,
JERR_DAC_INDEX ,
JERR_DAC_VALUE ,
JERR_DHT_INDEX ,
JERR_DQT_INDEX ,
JERR_EMPTY_IMAGE ,
JERR_EMS_READ ,
JERR_EMS_WRITE ,
JERR_EOI_EXPECTED ,
JERR_FILE_READ ,
JERR_FILE_WRITE ,
JERR_FRACT_SAMPLE_NOTIMPL ,
JERR_HUFF_CLEN_OUTOFBOUNDS ,
JERR_HUFF_MISSING_CODE ,
JERR_IMAGE_TOO_BIG ,
JERR_INPUT_EMPTY ,
JERR_INPUT_EOF ,
JERR_MISMATCHED_QUANT_TABLE ,

JERR_MISSING_DATA ,
JERR_MODE_CHANGE ,
JERR_NOTIMPL ,
JERR_NOT_COMPILED ,
JERR_NO_ARITH_TABLE ,
JERR_NO_BACKING_STORE ,
JERR_NO_HUFF_TABLE ,
JERR_NO_IMAGE ,
JERR_NO_QUANT_TABLE ,
JERR_NO_SOI ,
JERR_OUT_OF_MEMORY ,
JERR_QUANT_COMPONENTS ,

JERR_QUANT_FEW_COLORS ,
JERR_QUANT_MANY_COLORS ,
JERR_SOF_BEFORE ,
JERR_SOF_DUPLICATE ,
JERR_SOF_NO_SOS ,
JERR_SOF_UNSUPPORTED ,
JERR_SOI_DUPLICATE ,
JERR_TFILE_CREATE ,
JERR_TFILE_READ ,
JERR_TFILE_SEEK ,
JERR_TFILE_WRITE ,

JERR_TOO_LITTLE_DATA ,
JERR_UNKNOWN_MARKER ,
JERR_VIRTUAL_BUG ,
JERR_WIDTH_OVERFLOW ,
JERR_XMS_READ ,
JERR_XMS_WRITE ,
JMSG_COPYRIGHT ,
JMSG_VERSION ,
JTRC_16BIT_TABLES ,

JTRC_ADOBE ,

JTRC_APP0 ,
JTRC_APP14 ,
JTRC_DAC ,
JTRC_DHT ,
JTRC_DQT ,
JTRC_DRI ,
JTRC_EMS_CLOSE ,
JTRC_EMS_OPEN ,
JTRC_EOI ,
JTRC_HUFFBITS ,
JTRC_JFIF ,
JTRC_JFIF_BADTHUMBNAILSIZE ,

JTRC_JFIF_EXTENSION ,

JTRC_JFIF_THUMBNAIL ,
JTRC_MISC_MARKER ,
JTRC_PARMLESS_MARKER ,
JTRC_QUANTVALS ,
JTRC_QUANT_3_NCOLORS ,
JTRC_QUANT_NCOLORS ,
JTRC_QUANT_SELECTED ,
JTRC_RECOVERY_ACTION ,
JTRC_RST ,
JTRC_SMOOTH_NOTIMPL ,

JTRC_SOF ,
JTRC_SOF_COMPONENT ,
JTRC_SOI ,
JTRC_SOS ,
JTRC_SOS_COMPONENT ,
JTRC_SOS_PARAMS ,
JTRC_TFILE_CLOSE ,
JTRC_TFILE_OPEN ,
JTRC_THUMB_JPEG ,

JTRC_THUMB_PALETTE ,

JTRC_THUMB_RGB ,

JTRC_UNKNOWN_IDS ,

JTRC_XMS_CLOSE ,
JTRC_XMS_OPEN ,
JWRN_ADOBE_XFORM ,
JWRN_ARITH_BAD_CODE ,
JWRN_BOGUS_PROGRESSION ,

JWRN_EXTRANEOUS_DATA ,

JWRN_HIT_MARKER ,
JWRN_HUFF_BAD_CODE ,
JWRN_JFIF_MAJOR ,
JWRN_JPEG_EOF ,
JWRN_MUST_RESYNC ,

JWRN_NOT_SEQUENTIAL ,
JWRN_TOO_MUCH_DATA ,



  JMSG_LASTMSGCODE
} J_MESSAGE_CODE;
# 1175 "mupdf/thirdparty/libjpeg/jpeglib.h" 2
# 19 "mupdf/thirdparty/libjpeg/jaricom.c" 2
# 31 "mupdf/thirdparty/libjpeg/jaricom.c"
const INT32 jpeg_aritab[113+1] = {



  (((INT32)0x5a1d << 16) | ((INT32)1 << 8) | ((INT32)1 << 7) | 1),
  (((INT32)0x2586 << 16) | ((INT32)2 << 8) | ((INT32)0 << 7) | 14),
  (((INT32)0x1114 << 16) | ((INT32)3 << 8) | ((INT32)0 << 7) | 16),
  (((INT32)0x080b << 16) | ((INT32)4 << 8) | ((INT32)0 << 7) | 18),
  (((INT32)0x03d8 << 16) | ((INT32)5 << 8) | ((INT32)0 << 7) | 20),
  (((INT32)0x01da << 16) | ((INT32)6 << 8) | ((INT32)0 << 7) | 23),
  (((INT32)0x00e5 << 16) | ((INT32)7 << 8) | ((INT32)0 << 7) | 25),
  (((INT32)0x006f << 16) | ((INT32)8 << 8) | ((INT32)0 << 7) | 28),
  (((INT32)0x0036 << 16) | ((INT32)9 << 8) | ((INT32)0 << 7) | 30),
  (((INT32)0x001a << 16) | ((INT32)10 << 8) | ((INT32)0 << 7) | 33),
  (((INT32)0x000d << 16) | ((INT32)11 << 8) | ((INT32)0 << 7) | 35),
  (((INT32)0x0006 << 16) | ((INT32)12 << 8) | ((INT32)0 << 7) | 9),
  (((INT32)0x0003 << 16) | ((INT32)13 << 8) | ((INT32)0 << 7) | 10),
  (((INT32)0x0001 << 16) | ((INT32)13 << 8) | ((INT32)0 << 7) | 12),
  (((INT32)0x5a7f << 16) | ((INT32)15 << 8) | ((INT32)1 << 7) | 15),
  (((INT32)0x3f25 << 16) | ((INT32)16 << 8) | ((INT32)0 << 7) | 36),
  (((INT32)0x2cf2 << 16) | ((INT32)17 << 8) | ((INT32)0 << 7) | 38),
  (((INT32)0x207c << 16) | ((INT32)18 << 8) | ((INT32)0 << 7) | 39),
  (((INT32)0x17b9 << 16) | ((INT32)19 << 8) | ((INT32)0 << 7) | 40),
  (((INT32)0x1182 << 16) | ((INT32)20 << 8) | ((INT32)0 << 7) | 42),
  (((INT32)0x0cef << 16) | ((INT32)21 << 8) | ((INT32)0 << 7) | 43),
  (((INT32)0x09a1 << 16) | ((INT32)22 << 8) | ((INT32)0 << 7) | 45),
  (((INT32)0x072f << 16) | ((INT32)23 << 8) | ((INT32)0 << 7) | 46),
  (((INT32)0x055c << 16) | ((INT32)24 << 8) | ((INT32)0 << 7) | 48),
  (((INT32)0x0406 << 16) | ((INT32)25 << 8) | ((INT32)0 << 7) | 49),
  (((INT32)0x0303 << 16) | ((INT32)26 << 8) | ((INT32)0 << 7) | 51),
  (((INT32)0x0240 << 16) | ((INT32)27 << 8) | ((INT32)0 << 7) | 52),
  (((INT32)0x01b1 << 16) | ((INT32)28 << 8) | ((INT32)0 << 7) | 54),
  (((INT32)0x0144 << 16) | ((INT32)29 << 8) | ((INT32)0 << 7) | 56),
  (((INT32)0x00f5 << 16) | ((INT32)30 << 8) | ((INT32)0 << 7) | 57),
  (((INT32)0x00b7 << 16) | ((INT32)31 << 8) | ((INT32)0 << 7) | 59),
  (((INT32)0x008a << 16) | ((INT32)32 << 8) | ((INT32)0 << 7) | 60),
  (((INT32)0x0068 << 16) | ((INT32)33 << 8) | ((INT32)0 << 7) | 62),
  (((INT32)0x004e << 16) | ((INT32)34 << 8) | ((INT32)0 << 7) | 63),
  (((INT32)0x003b << 16) | ((INT32)35 << 8) | ((INT32)0 << 7) | 32),
  (((INT32)0x002c << 16) | ((INT32)9 << 8) | ((INT32)0 << 7) | 33),
  (((INT32)0x5ae1 << 16) | ((INT32)37 << 8) | ((INT32)1 << 7) | 37),
  (((INT32)0x484c << 16) | ((INT32)38 << 8) | ((INT32)0 << 7) | 64),
  (((INT32)0x3a0d << 16) | ((INT32)39 << 8) | ((INT32)0 << 7) | 65),
  (((INT32)0x2ef1 << 16) | ((INT32)40 << 8) | ((INT32)0 << 7) | 67),
  (((INT32)0x261f << 16) | ((INT32)41 << 8) | ((INT32)0 << 7) | 68),
  (((INT32)0x1f33 << 16) | ((INT32)42 << 8) | ((INT32)0 << 7) | 69),
  (((INT32)0x19a8 << 16) | ((INT32)43 << 8) | ((INT32)0 << 7) | 70),
  (((INT32)0x1518 << 16) | ((INT32)44 << 8) | ((INT32)0 << 7) | 72),
  (((INT32)0x1177 << 16) | ((INT32)45 << 8) | ((INT32)0 << 7) | 73),
  (((INT32)0x0e74 << 16) | ((INT32)46 << 8) | ((INT32)0 << 7) | 74),
  (((INT32)0x0bfb << 16) | ((INT32)47 << 8) | ((INT32)0 << 7) | 75),
  (((INT32)0x09f8 << 16) | ((INT32)48 << 8) | ((INT32)0 << 7) | 77),
  (((INT32)0x0861 << 16) | ((INT32)49 << 8) | ((INT32)0 << 7) | 78),
  (((INT32)0x0706 << 16) | ((INT32)50 << 8) | ((INT32)0 << 7) | 79),
  (((INT32)0x05cd << 16) | ((INT32)51 << 8) | ((INT32)0 << 7) | 48),
  (((INT32)0x04de << 16) | ((INT32)52 << 8) | ((INT32)0 << 7) | 50),
  (((INT32)0x040f << 16) | ((INT32)53 << 8) | ((INT32)0 << 7) | 50),
  (((INT32)0x0363 << 16) | ((INT32)54 << 8) | ((INT32)0 << 7) | 51),
  (((INT32)0x02d4 << 16) | ((INT32)55 << 8) | ((INT32)0 << 7) | 52),
  (((INT32)0x025c << 16) | ((INT32)56 << 8) | ((INT32)0 << 7) | 53),
  (((INT32)0x01f8 << 16) | ((INT32)57 << 8) | ((INT32)0 << 7) | 54),
  (((INT32)0x01a4 << 16) | ((INT32)58 << 8) | ((INT32)0 << 7) | 55),
  (((INT32)0x0160 << 16) | ((INT32)59 << 8) | ((INT32)0 << 7) | 56),
  (((INT32)0x0125 << 16) | ((INT32)60 << 8) | ((INT32)0 << 7) | 57),
  (((INT32)0x00f6 << 16) | ((INT32)61 << 8) | ((INT32)0 << 7) | 58),
  (((INT32)0x00cb << 16) | ((INT32)62 << 8) | ((INT32)0 << 7) | 59),
  (((INT32)0x00ab << 16) | ((INT32)63 << 8) | ((INT32)0 << 7) | 61),
  (((INT32)0x008f << 16) | ((INT32)32 << 8) | ((INT32)0 << 7) | 61),
  (((INT32)0x5b12 << 16) | ((INT32)65 << 8) | ((INT32)1 << 7) | 65),
  (((INT32)0x4d04 << 16) | ((INT32)66 << 8) | ((INT32)0 << 7) | 80),
  (((INT32)0x412c << 16) | ((INT32)67 << 8) | ((INT32)0 << 7) | 81),
  (((INT32)0x37d8 << 16) | ((INT32)68 << 8) | ((INT32)0 << 7) | 82),
  (((INT32)0x2fe8 << 16) | ((INT32)69 << 8) | ((INT32)0 << 7) | 83),
  (((INT32)0x293c << 16) | ((INT32)70 << 8) | ((INT32)0 << 7) | 84),
  (((INT32)0x2379 << 16) | ((INT32)71 << 8) | ((INT32)0 << 7) | 86),
  (((INT32)0x1edf << 16) | ((INT32)72 << 8) | ((INT32)0 << 7) | 87),
  (((INT32)0x1aa9 << 16) | ((INT32)73 << 8) | ((INT32)0 << 7) | 87),
  (((INT32)0x174e << 16) | ((INT32)74 << 8) | ((INT32)0 << 7) | 72),
  (((INT32)0x1424 << 16) | ((INT32)75 << 8) | ((INT32)0 << 7) | 72),
  (((INT32)0x119c << 16) | ((INT32)76 << 8) | ((INT32)0 << 7) | 74),
  (((INT32)0x0f6b << 16) | ((INT32)77 << 8) | ((INT32)0 << 7) | 74),
  (((INT32)0x0d51 << 16) | ((INT32)78 << 8) | ((INT32)0 << 7) | 75),
  (((INT32)0x0bb6 << 16) | ((INT32)79 << 8) | ((INT32)0 << 7) | 77),
  (((INT32)0x0a40 << 16) | ((INT32)48 << 8) | ((INT32)0 << 7) | 77),
  (((INT32)0x5832 << 16) | ((INT32)81 << 8) | ((INT32)1 << 7) | 80),
  (((INT32)0x4d1c << 16) | ((INT32)82 << 8) | ((INT32)0 << 7) | 88),
  (((INT32)0x438e << 16) | ((INT32)83 << 8) | ((INT32)0 << 7) | 89),
  (((INT32)0x3bdd << 16) | ((INT32)84 << 8) | ((INT32)0 << 7) | 90),
  (((INT32)0x34ee << 16) | ((INT32)85 << 8) | ((INT32)0 << 7) | 91),
  (((INT32)0x2eae << 16) | ((INT32)86 << 8) | ((INT32)0 << 7) | 92),
  (((INT32)0x299a << 16) | ((INT32)87 << 8) | ((INT32)0 << 7) | 93),
  (((INT32)0x2516 << 16) | ((INT32)71 << 8) | ((INT32)0 << 7) | 86),
  (((INT32)0x5570 << 16) | ((INT32)89 << 8) | ((INT32)1 << 7) | 88),
  (((INT32)0x4ca9 << 16) | ((INT32)90 << 8) | ((INT32)0 << 7) | 95),
  (((INT32)0x44d9 << 16) | ((INT32)91 << 8) | ((INT32)0 << 7) | 96),
  (((INT32)0x3e22 << 16) | ((INT32)92 << 8) | ((INT32)0 << 7) | 97),
  (((INT32)0x3824 << 16) | ((INT32)93 << 8) | ((INT32)0 << 7) | 99),
  (((INT32)0x32b4 << 16) | ((INT32)94 << 8) | ((INT32)0 << 7) | 99),
  (((INT32)0x2e17 << 16) | ((INT32)86 << 8) | ((INT32)0 << 7) | 93),
  (((INT32)0x56a8 << 16) | ((INT32)96 << 8) | ((INT32)1 << 7) | 95),
  (((INT32)0x4f46 << 16) | ((INT32)97 << 8) | ((INT32)0 << 7) | 101),
  (((INT32)0x47e5 << 16) | ((INT32)98 << 8) | ((INT32)0 << 7) | 102),
  (((INT32)0x41cf << 16) | ((INT32)99 << 8) | ((INT32)0 << 7) | 103),
  (((INT32)0x3c3d << 16) | ((INT32)100 << 8) | ((INT32)0 << 7) | 104),
  (((INT32)0x375e << 16) | ((INT32)93 << 8) | ((INT32)0 << 7) | 99),
  (((INT32)0x5231 << 16) | ((INT32)102 << 8) | ((INT32)0 << 7) | 105),
  (((INT32)0x4c0f << 16) | ((INT32)103 << 8) | ((INT32)0 << 7) | 106),
  (((INT32)0x4639 << 16) | ((INT32)104 << 8) | ((INT32)0 << 7) | 107),
  (((INT32)0x415e << 16) | ((INT32)99 << 8) | ((INT32)0 << 7) | 103),
  (((INT32)0x5627 << 16) | ((INT32)106 << 8) | ((INT32)1 << 7) | 105),
  (((INT32)0x50e7 << 16) | ((INT32)107 << 8) | ((INT32)0 << 7) | 108),
  (((INT32)0x4b85 << 16) | ((INT32)103 << 8) | ((INT32)0 << 7) | 109),
  (((INT32)0x5597 << 16) | ((INT32)109 << 8) | ((INT32)0 << 7) | 110),
  (((INT32)0x504f << 16) | ((INT32)107 << 8) | ((INT32)0 << 7) | 111),
  (((INT32)0x5a10 << 16) | ((INT32)111 << 8) | ((INT32)1 << 7) | 110),
  (((INT32)0x5522 << 16) | ((INT32)109 << 8) | ((INT32)0 << 7) | 112),
  (((INT32)0x59eb << 16) | ((INT32)111 << 8) | ((INT32)1 << 7) | 112),




  (((INT32)0x5a1d << 16) | ((INT32)113 << 8) | ((INT32)0 << 7) | 113)
};
