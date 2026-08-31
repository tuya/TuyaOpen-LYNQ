/**
 * @file lv_port_fs.c
 *
 */

/*Copy this file as "lv_port_fs.c" and set this value to "1" to enable content*/
#if FEATURE_SUBSYS_STORAGE_ENABLE

/*********************
 *      INCLUDES
 *********************/

#include "lvgl.h"
#include "lv_port_fs.h"

#include "storage.h"
#include "ostask.h"
#include "osasys.h"
#include "bsp.h"
#include "bsp_custom.h"
// #define EC_TRACE(subId, argLen, format,  ...)  ECOMM_TRACE(UNILOG_LVGL, subId, P_VALUE, argLen, format,  ##__VA_ARGS__) 

// LV_ATTRIBUTE_LARGE_RAM_ARRAY lv_color_t s_disp_bin[LCD_WIDTH*LCD_HEIGHT];
/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

static void fs_init(void);

static void * fs_open(lv_fs_drv_t * drv, const char * path, lv_fs_mode_t mode);
static lv_fs_res_t fs_close(lv_fs_drv_t * drv, void * file_p);

static lv_fs_res_t fs_read(lv_fs_drv_t * drv, void * file_p, void * buf, uint32_t btr, uint32_t * br);
static lv_fs_res_t fs_write(lv_fs_drv_t * drv, void * file_p, const void * buf, uint32_t btw, uint32_t * bw);

static lv_fs_res_t fs_seek(lv_fs_drv_t * drv, void * file_p, uint32_t pos, lv_fs_whence_t whence);
static lv_fs_res_t fs_tell(lv_fs_drv_t * drv, void * file_p, uint32_t * pos_p);

static void * fs_dir_open(lv_fs_drv_t * drv, const char * path);
static lv_fs_res_t fs_dir_read(lv_fs_drv_t * drv, void * dir_p, char * fn);
static lv_fs_res_t fs_dir_close(lv_fs_drv_t * drv, void * dir_p);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_port_fs_init(void)
{
    /*----------------------------------------------------
     * Initialize your storage device and File System
     * -------------------------------------------------*/
    fs_init();

    /*---------------------------------------------------
     * Register the file system interface in LVGL
     *--------------------------------------------------*/
    static lv_fs_drv_t fs_drv_c;
    lv_fs_drv_init(&fs_drv_c);

    /*Set up fields...*/
    fs_drv_c.letter = 'C';
    fs_drv_c.cache_size = 8191;
    fs_drv_c.open_cb = fs_open;
    fs_drv_c.close_cb = fs_close;
    fs_drv_c.read_cb = fs_read;
    fs_drv_c.write_cb = fs_write;
    fs_drv_c.seek_cb = fs_seek;
    fs_drv_c.tell_cb = fs_tell;
    fs_drv_c.dir_open_cb = fs_dir_open;
    fs_drv_c.dir_close_cb = fs_dir_close;
    fs_drv_c.dir_read_cb = fs_dir_read;
    #if LV_USE_USER_DATA
    fs_drv_c.user_data = NULL;
    #endif
    lv_fs_drv_register(&fs_drv_c);

    /*Add a simple drive to open images*/
    static lv_fs_drv_t fs_drv;
    lv_fs_drv_init(&fs_drv);

    /*Set up fields...*/
    fs_drv.letter = 'X';
    fs_drv.cache_size = 8191;   //每次只能读8K需要另外的PSRAM作为缓存空间

    fs_drv.open_cb = fs_open;
    fs_drv.close_cb = fs_close;
    fs_drv.read_cb = fs_read;
    fs_drv.write_cb = fs_write;
    fs_drv.seek_cb = fs_seek;
    fs_drv.tell_cb = fs_tell;

    fs_drv.dir_open_cb = fs_dir_open;
    fs_drv.dir_close_cb = fs_dir_close;
    fs_drv.dir_read_cb = fs_dir_read;

    #if LV_USE_USER_DATA
    fs_drv.user_data = NULL;
    #endif

    lv_fs_drv_register(&fs_drv);


    /*Add a simple drive to open images*/
    static lv_fs_drv_t fs_drv_d;
    lv_fs_drv_init(&fs_drv_d);

    /*Set up fields...*/
    fs_drv_d.letter = 'D';
    fs_drv_d.cache_size = 8191;   //每次只能读8K需要另外的PSRAM作为缓存空间

    fs_drv_d.open_cb = fs_open;
    fs_drv_d.close_cb = fs_close;
    fs_drv_d.read_cb = fs_read;
    fs_drv_d.write_cb = fs_write;
    fs_drv_d.seek_cb = fs_seek;
    fs_drv_d.tell_cb = fs_tell;
    fs_drv_d.dir_open_cb = fs_dir_open;
    fs_drv_d.dir_close_cb = fs_dir_close;
    fs_drv_d.dir_read_cb = fs_dir_read;
    #if LV_USE_USER_DATA
    fs_drv_d.user_data = NULL;
    #endif
    lv_fs_drv_register(&fs_drv_d);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/*Initialize your Storage device and File system.*/
static void fs_init(void)
{
    /* Initialize the internal flash or SD-card and LittleFS itself.
     * Better to do it in your code to keep this library untouched for easy updating */
}

/**
 * Open a file
 * @param drv       pointer to a driver where this function belongs
 * @param path      path to the file beginning with the driver letter (e.g. S:/folder/file.txt)
 * @param mode      read: FS_MODE_RD, write: FS_MODE_WR, both: FS_MODE_RD | FS_MODE_WR
 * @return          pointer to a file descriptor or NULL on error
 */
// int lfs_file_open(lfs_t *lfs, lfs_file_t *file,const char *path, int flags) 
static void * fs_open(lv_fs_drv_t * drv, const char * path, lv_fs_mode_t mode)
{
    FILE * file = NULL;
    uint16_t path_len = strlen(path);
    char *path_buf = (char *)lv_mem_alloc(sizeof(char)*(path_len + 4));
    sprintf(path_buf, "%c:%s", drv->letter,path);
    if(mode == LV_FS_MODE_RD) {
        file = file_fopen(path_buf, "r");
    }
    else if(mode == LV_FS_MODE_WR) {
    }
    // LV_LOG_WARN("0x%X,mode %d,path:%s->%s,0x%X",drv, mode,path,path_buf,file);
    lv_mem_free(path_buf);
    return file;
}

/**
 * Close an opened file
 * @param drv       pointer to a driver where this function belongs
 * @param file_p    pointer to a file_t variable. (opened with fs_open)
 * @return          LV_FS_RES_OK: no error or  any error from @lv_fs_res_t enum
 */
static lv_fs_res_t fs_close(lv_fs_drv_t * drv, void * file_p)
{
    FILE * file = file_p;
    int ret = file_fclose(file);
    // LV_LOG_WARN("0x%X,ret %d", file,ret);
    if (ret != LFS_ERR_OK)
        return LV_FS_RES_NOT_IMP;
  	else
        return LV_FS_RES_OK;
}
/**
 * Read data from an opened file
 * @param drv       pointer to a driver where this function belongs
 * @param file_p    pointer to a file_t variable.
 * @param buf       pointer to a memory block where to store the read data
 * @param btr       number of Bytes To Read
 * @param br        the real number of read bytes (Byte Read)
 * @return          LV_FS_RES_OK: no error or  any error from @lv_fs_res_t enum
 */
static lv_fs_res_t fs_read(lv_fs_drv_t * drv, void * file_p, void * buf, uint32_t btr, uint32_t * br)
{
    FILE * file = file_p;
    // struct stat  fstat = {0};
    // file_fstat((int)file, &fstat);
    // * br = fstat.st_size;
    * br = file_fread(buf, 1, btr, file);
    // LV_LOG_WARN("0x%X:read %d,real %d,buf 0x%X",file_p,fstat.st_size,*br,*((uint32_t *)buf));
    if(*br < 1) return LV_FS_RES_UNKNOWN;
    return LV_FS_RES_OK;
}

/**
 * Write into a file
 * @param drv       pointer to a driver where this function belongs
 * @param file_p    pointer to a file_t variable
 * @param buf       pointer to a buffer with the bytes to write
 * @param btw       Bytes To Write
 * @param bw        the number of real written bytes (Bytes Written). NULL if unused.
 * @return          LV_FS_RES_OK: no error or  any error from @lv_fs_res_t enum
 */
static lv_fs_res_t fs_write(lv_fs_drv_t * drv, void * file_p, const void * buf, uint32_t btw, uint32_t * bw)
{
    lv_fs_res_t res = LV_FS_RES_NOT_IMP;
    // LV_LOG_WARN("0x%X,0x%X", drv,file_p);
    /*Add your code here*/
    return res;
}

/**
 * Set the read write pointer. Also expand the file size if necessary.
 * @param drv       pointer to a driver where this function belongs
 * @param file_p    pointer to a file_t variable. (opened with fs_open )
 * @param pos       the new position of read write pointer
 * @param whence    tells from where to interpret the `pos`. See @lv_fs_whence_t
 * @return          LV_FS_RES_OK: no error or  any error from @lv_fs_res_t enum
 */
static lv_fs_res_t fs_seek(lv_fs_drv_t * drv, void * file_p, uint32_t pos, lv_fs_whence_t whence)
{
    FILE * file = file_p;
    if(whence == LV_FS_SEEK_SET) {
        file_fseek(file, pos, SEEK_SET);
    }
    else if(whence == LV_FS_SEEK_CUR) {
        file_fseek(file, pos, SEEK_CUR);
    }
    else if(whence == LV_FS_SEEK_END) {
        file_fseek(file, pos, SEEK_END);
    }
    // LV_LOG_WARN("0x%X,0x%X:%d,pos %d", drv,file,whence,pos);
    // EC_TRACE(fs_seek,3,"%d,0x%X,0x%X",whence,pos,*((uint32_t *)file_p));
    /*pos = result;*/ /*not supported by lv_fs*/
    return LV_FS_RES_OK;
}
/**
 * Give the position of the read write pointer
 * @param drv       pointer to a driver where this function belongs
 * @param file_p    pointer to a file_t variable.
 * @param pos_p     pointer to where to store the result
 * @return          LV_FS_RES_OK: no error or  any error from @lv_fs_res_t enum
 */
static lv_fs_res_t fs_tell(lv_fs_drv_t * drv, void * file_p, uint32_t * pos_p)
{
    FILE * file = file_p;
    lfs_soff_t result = file_ftell(file);
    // LV_LOG_WARN("0x%X,0x%X", drv,file_p);
    // EC_TRACE(fs_tell,3,"%d,0x%X",result,*((uint32_t *)file_p));
    if(result < 0) return LV_FS_RES_UNKNOWN;

    *pos_p = (uint32_t) result;
    return LV_FS_RES_OK;
}

/**
 * Initialize a 'lv_fs_dir_t' variable for directory reading
 * @param drv       pointer to a driver where this function belongs
 * @param path      path to a directory
 * @return          pointer to the directory read descriptor or NULL on error
 */
static void * fs_dir_open(lv_fs_drv_t * drv, const char * path)
{
    return NULL;
}

/**
 * Read the next filename form a directory.
 * The name of the directories will begin with '/'
 * @param drv       pointer to a driver where this function belongs
 * @param rddir_p   pointer to an initialized 'lv_fs_dir_t' variable
 * @param fn        pointer to a buffer to store the filename
 * @return          LV_FS_RES_OK: no error or  any error from @lv_fs_res_t enum
 */
static lv_fs_res_t fs_dir_read(lv_fs_drv_t * drv, void * rddir_p, char * fn)
{
    return LV_FS_RES_OK;
}

/**
 * Close the directory reading
 * @param drv       pointer to a driver where this function belongs
 * @param rddir_p   pointer to an initialized 'lv_fs_dir_t' variable
 * @return          LV_FS_RES_OK: no error or  any error from @lv_fs_res_t enum
 */
static lv_fs_res_t fs_dir_close(lv_fs_drv_t * drv, void * rddir_p)
{
    return LV_FS_RES_OK;
}

#else /*LV_USE_FS_LITTLEFS == 0*/

/*This dummy typedef exists purely to silence -Wpedantic.*/
typedef int keep_pedantic_happy;
#endif
