#include "abx_file.h"


#define FILECP_BUFFER_SIZE 1024


abx_errid_t abx_copyfile(const char * src, const char * dst, bool fail_if_exist)
{
    int from_fd, to_fd;
    int oflag;
    int bytes_read, bytes_write, pos;
    char buffer[FILECP_BUFFER_SIZE];

    from_fd = abx_fileopen(src, O_RDONLY | ABX_O_BINARY);
    if(from_fd == -1)
        return ABXERR_FILENOTEXIST;

    oflag = ABX_O_CREAT | ABX_O_WRONLY | ABX_O_BINARY;
    if (fail_if_exist)
        oflag |= ABX_O_EXCL;

    to_fd = abx_fileopen(dst, oflag, ABX_S_IREAD | ABX_S_IWRITE);
    if(to_fd == -1) {
        abx_fileclose(from_fd);
        return ABXERR_FAILOPENFILE;
    }

    do {
        bytes_read = abx_fileread(from_fd, buffer, FILECP_BUFFER_SIZE);
        if(bytes_read == -1) {      /* todo maybe need to check EINTR? */
            abx_fileclose(from_fd);
            abx_fileclose(to_fd);
            return ABXERR_FAILREADFILE;
        }
        else if(bytes_read > 0)
        {
            pos = 0;
            do {
                bytes_write = abx_filewrite(to_fd, buffer + pos, bytes_read - pos);
                if (bytes_write <= 0) {  /* todo maybe need to check EINTR? */
                    abx_fileclose(from_fd);
                    abx_fileclose(to_fd);
                    return ABXERR_FAILWRITEFILE;
                }
                else if (bytes_write > 0) {
                    pos += bytes_write;
                }
            } while (pos < bytes_read);
        }
    } while(bytes_read != 0);

    abx_fileclose(from_fd);
    abx_fileclose(to_fd);
    return ABXERR_OK;
}
