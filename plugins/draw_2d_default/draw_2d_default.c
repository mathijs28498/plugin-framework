#include "draw_2d_default.h"

#include <plugin_sdk/logger/v1/logger_interface.h>
LOGGER_INTERFACE_REGISTER(draw_2d_default, LOG_LEVEL_DEBUG)

#include "draw_2d_default_register.h"

void draw_2d_test(struct Draw2dContext *context, int test_int)
{
    LoggerInterface *logger = context->deps.logger;
    LOG_WRN(logger, "Inside test fn of draw_2d_default - %d", test_int);
}
