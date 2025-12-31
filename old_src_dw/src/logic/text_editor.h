#pragma once

#warning TODO: Make this better, it still needs a render_context which this doesnt know
// TODO: Make this better, it 
typedef struct {
    void (*render_frame)(void *renderer_context);
} TextEditorRenderer;

void text_editor_test();