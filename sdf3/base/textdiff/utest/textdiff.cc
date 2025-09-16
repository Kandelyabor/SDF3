#include <cstdlib>
#include <stdio.h>
#include "../../string/cstring.h"
#include "../textdiff.h"

int main(int argc, char **argv)
{
    CString input_a;
    CString input_b;
    if (argc != 3) return EXIT_FAILURE;

    FILE *fp = fopen(argv[1], "r");
    if (fp==nullptr) {
        std::cerr << "Failed to open file: " << argv[1] << std::endl;
        return EXIT_FAILURE;
    }
    char buffer[1024];
    while (fgets(buffer, 1024, fp) != NULL)
    {
        input_a += CString(buffer);
    }
    fclose(fp);
    fp = fopen(argv[2], "r");
    if (fp==nullptr) {
        std::cerr << "Failed to open file: " << argv[2] << std::endl;
        return EXIT_FAILURE;
    }
    while (fgets(buffer, 1024, fp) != NULL)
    {
        input_b += buffer;
    }
    fclose(fp);

    CString diff = TextDiff(input_a, input_b);

    std::cout << "============" << std::endl;
    std::cout << diff;
    std::cout << "============" << std::endl;

    return EXIT_SUCCESS;
}
