#include "volc_errno.h"
#include <errno.h>
#include "sockets.h"
int volc_errno(int fd) {
    return sock_get_errno(fd);
}