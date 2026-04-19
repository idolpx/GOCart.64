/*
 * Copyright (c) 2019-2024 Kim Jørgensen
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

#include <ff.h>
#include <cstddef>

static FATFS fs;

bool filesystem_mount(void) {
   FRESULT res = f_mount(&fs, "", 1);

   return res == FR_OK;
}

bool filesystem_unmount(void) {
   FRESULT res = f_unmount("");

   return res == FR_OK;
}

size_t filesystem_getfree(void) {
   FATFS *fs_ptr = &fs;
   DWORD fre_clust = 0;

   f_getfree("", &fre_clust, &fs_ptr);

   return fre_clust * fs.csize;
}

bool filesystem_getlabel(char *label) {
   FRESULT res = f_getlabel("", label, NULL);

   return res == FR_OK;
}

bool file_open(FIL *file, const char *file_name, uint8_t mode) {
   FRESULT res = f_open(file, file_name, mode);

   return res == FR_OK;
}

uint32_t file_read(FIL *file, void *buffer, size_t bytes)
{
   UINT bytes_read;
   FRESULT res = f_read(file, buffer, bytes, &bytes_read);
   if (res != FR_OK)
      bytes_read = 0;

   return bytes_read;
}

bool file_seek(FIL *file, FSIZE_t offset) {
   FRESULT res = f_lseek(file, offset);

   return res == FR_OK;
}

uint32_t file_write(FIL *file, void *buffer, size_t bytes) {
   UINT bytes_written;
   FRESULT res = f_write(file, buffer, bytes, &bytes_written);
   if (res != FR_OK)
      bytes_written = 0;

   return bytes_written;
}

bool file_truncate(FIL *file) {
   FRESULT res = f_truncate(file);

   return res == FR_OK;
}

bool file_sync(FIL *file) {
   FRESULT res = f_sync(file);

   return res == FR_OK;
}

bool file_close(FIL *file) {
   FRESULT res = f_close(file);

   return res == FR_OK;
}

bool file_stat(const char *file_name, FILINFO *file_info) {
   FRESULT res = f_stat(file_name, file_info);

   return res == FR_OK;
}

bool file_delete(const char *file_name) {
   FRESULT res = f_unlink(file_name);

   return res == FR_OK;
}

bool dir_change(const char *path) {
   FRESULT res = f_chdir(path);

   return res == FR_OK;
}

bool dir_current(char *path, size_t path_size) {
   FRESULT res = f_getcwd(path, path_size);

   return res == FR_OK;
}

bool dir_open(DIR *dir, const char *pattern) {
   if (!pattern || !pattern[0])
      pattern = "*";
   dir->pat = pattern;

   FRESULT res = f_opendir(dir, "");

   return res == FR_OK;
}

bool dir_read(DIR *dir, FILINFO *file_info) {
   FRESULT res;
   while (true) {
      res = f_findnext(dir, file_info);
      if (res != FR_OK)
         break;

      if (!file_info->fname[0])
         break;

      // Ignore hidden files
      if (file_info->fname[0] != '.' &&
         !(file_info->fattrib & (AM_HID|AM_SYS)))
         break;
   }

   return res == FR_OK;
}

bool dir_close(DIR *dir) {
   FRESULT res = f_closedir(dir);

   return res == FR_OK;
}
