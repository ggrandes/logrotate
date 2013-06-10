/*
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *
 * 2010.09.06, v1.0, Guillermo Grandes
 *
 * Compile: gcc -Wall -O2 logrotate.c -o logrotate
 */
#define _LARGEFILE_SOURCE
#define _FILE_OFFSET_BITS 64
#define _GNU_SOURCE
#define _XOPEN_SOURCE 600
//
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
//
#define BUF_SIZE_IN 2048
#define BUF_SIZE_TS 64
#define VERSION     "1.0"
//
static char *name = NULL;
static unsigned long MAX_SIZE = 0;
//
inline int
printLineTS(char *buf, int maxLen, char *extra)
{
       time_t t = time(NULL);
       struct tm *p_tm = localtime(&t);
       return snprintf(buf, maxLen, "%04d-%02d-%02d %02d:%02d:%02d%s", p_tm->tm_year+1900, p_tm->tm_mon+1, p_tm->tm_mday, p_tm->tm_hour, p_tm->tm_min, p_tm->tm_sec, extra);
}
//
inline int
loadBuffer(void *buf, int pos, int size)
{
    return read(0, buf + pos, size - pos);
}
//
inline unsigned long long
getFileSize(int fd)
{
    struct stat sb;
    int res = fstat(fd, &sb);
    if (res != 0) {
        fprintf(stderr, "Error fstat res(%d): %d (%s)\n", res, errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
    return sb.st_size;
}
//
inline int
openOutput(void)
{
    int fd = -1;
    // Open Output File
    fd = open(name, O_CREAT|O_APPEND|O_WRONLY|O_LARGEFILE, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
    if (fd < 0) {
        fprintf(stderr, "Error open: %d (%s)\n", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
    return fd;
}
//
inline void 
rotateLog(int *fd)
{
    if (*fd == -1) {
        *fd = openOutput();
    } else {
        unsigned long long fileSize = getFileSize(*fd);
        if (fileSize > MAX_SIZE) {
            char newname[2048];
            close(*fd);
            *fd = -1;
            snprintf(newname, sizeof(newname), "%s.old", name);
            rename(name, newname);
            *fd = openOutput();
        }
    }
}
//
void
process(void) 
{
    ssize_t readed = 0;
    int fd = -1;
    int i;
    int head = 0;
    int eol = 0;
    int newlined = 0;
    char bufin[BUF_SIZE_IN];
    char bufts[BUF_SIZE_TS];
    //
    memset(bufin, 0, sizeof(bufin));
    while ((readed = loadBuffer(bufin, head, sizeof(bufin)-1)) > 0) {
        bufin[readed] = 0;
        for (i = 0; i < readed; i++) {
            if (bufin[i] == 10 || bufin[i] == 13 || bufin[i] == 0) { // EOL
                bufin[i] = 0; // EOB
                write(fd, bufin + head, i - head);
                if (eol == 0) {
                    write(fd, "\n", 1);
                }
                head = i+1;
                eol++; newlined=0;
            } else {
                if (!newlined) {
                    int len = printLineTS(bufts, sizeof(bufts), " > ");
                    rotateLog(&fd);
                    write(fd, bufts, len);
                    newlined++;
                }
                eol = 0;
            }
        }
        if (!eol) {
            write(fd, bufin + head, readed - head);
        }
        head = 0;
    }
    if (newlined) {
        write(fd, "\n", 1);
    }
}
//
int
main(int argc, char *argv[])
{
    //
    // Check Params
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <pathname> <size-limit-bytes>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    //
    name = argv[1];
    MAX_SIZE = atol(argv[2]);
    //
    process();
    //
    exit(EXIT_SUCCESS);
}
//
