#include <stdlib.h>
#include <sys/unistd.h>
#include "utils/utils.h"

int main()
{
    bool a[100];
    get_installed_lbp_versions(a, NULL, NULL, NULL, NULL, NULL);
	return 0;
}	

