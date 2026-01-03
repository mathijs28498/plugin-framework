#include <stdio.h>

#include <test_lib.h>
#include <test_lib_i.h>

int main(int argc, const char *argv[])
{
    TestStruct test_struct = {
        .test_int = 400};

    printf("%s() - Value: %d\n", __func__, test_struct.test_int);
    if (argc >= 3)
    {
        printf("%s() argc: %d - argv[1]: %s - argv[2]: %s\n", __func__, argc, argv[1], argv[2]);
    }

    test_lib_print();

    return 0;
}