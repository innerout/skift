#pragma once

#include <libgraphic/shape.h>

void renderer_initialize(void);

Rectangle renderer_bound(void);

void renderer_region_dirty(Rectangle region);

void renderer_repaint_dirty(void);