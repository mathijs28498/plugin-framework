#pragma once

#include <stdint.h>

struct RenderGraphContext;
struct RG_Pass;

int32_t render_graph_default_register_pass(struct RenderGraphContext *context, struct RG_Pass *pass);
int32_t render_graph_default_render(struct RenderGraphContext *context);