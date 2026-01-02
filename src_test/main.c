#include <stdio.h>

#include <test_lib.h>
#include <test_lib_i.h>

int main()
{
    TestStruct test_struct = {
        .test_int = 400
    };

    printf("%s() - Value: %d\n", __func__, test_struct.test_int);

    test_lib_print();

    return 0;
}