#include "draw_2d_default.h"

#include <logger_interface.h>
LOGGER_INTERFACE_REGISTER(draw_2d_default, LOG_LEVEL_DEBUG)

#include "draw_2d_default_register.h"

void draw_2d_test(struct Draw2dContext *context, int test_int)
{
    LoggerInterface *logger = context->logger;
    LOG_WRN("Inside test fn of draw_2d_default - %d", test_int);
}
