#include <stdlib.h>
#include <sys/unistd.h>
#include "utils/utils.h"

int main()
{
    sleep(60);

    bool a[100];
    get_installed_lbp_versions(&a[0], NULL, NULL, NULL, NULL, NULL);
	return 0;
}	

