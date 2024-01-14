#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdint.h>
#include <stddef.h>
#define ADDR_LED 0xFF203000


int main ()
{
    uint32_t * p;

    int fd = open("/dev/mem", O_RDWR);
    p = (uint32_t*)mmap(NULL, 4, PROT_WRITE|PROT_READ, MAP_SHARED,fd, ADDR_LED);
    *p = (1<<1); //allume les 4 premières led en partant de la droite
     
}