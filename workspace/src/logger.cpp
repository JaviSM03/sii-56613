#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

int main(int argc, char **argv) {
    int fd;
    char buffer[1024];    

    // borra FIFO por si existía previamente
    unlink("FIFO");
    /* crea el FIFO */
    if (mkfifo("FIFO", 0666)<0) {
        perror("No puede crearse el FIFO");
        return(1);
    }
    /* Abre el FIFO */
    if ((fd=open("FIFO", O_RDONLY))<0) {
        perror("No puede abrirse el FIFO");
        return(1);
    }
    while (read(fd, buffer, sizeof(buffer)) > 0) {
	printf("%s\n", buffer);
    }
    close(fd);
    unlink("FIFO");
    return(0);
}
