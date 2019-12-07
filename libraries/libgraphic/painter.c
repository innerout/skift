/* Copyright © 2018-2019 N. Van Bossuyt.                                      */
/* This code is licensed under the MIT License.                               */
/* See: LICENSE.md                                                            */

#include <libsystem/assert.h>
#include <libgraphic/painter.h>
#include <libgraphic/font.h>
#include <libmath/math.h>

void painter_delete(Painter *);

Painter *painter_create(Bitmap *bmp)
{
    Painter *paint = __malloc(Painter);

    paint->bitmap = bmp;
    paint->cliprect = bitmap_bound(bmp);
    paint->cliprect_stack_top = 0;

    return paint;
}

void painter_destroy(Painter *this)
{
    free(this);
}

void painter_push_cliprect(Painter *paint, Rectangle cliprect)
{
    assert(paint->cliprect_stack_top < 32);

    paint->cliprect_stack[paint->cliprect_stack_top] = paint->cliprect;
    paint->cliprect_stack_top++;

    paint->cliprect = rectangle_clip(paint->cliprect, cliprect);
}

void painter_pop_cliprect(Painter *paint)
{
    assert(paint->cliprect_stack_top > 0);

    paint->cliprect_stack_top--;
    paint->cliprect = paint->cliprect_stack[paint->cliprect_stack_top];
}

void painter_plot_pixel(Painter *painter, Point position, color_t color)
{
    Point point_absolue = {painter->cliprect.X + position.X, painter->cliprect.Y + position.Y};

    if (rectangle_containe_point(painter->cliprect, point_absolue))
    {
        bitmap_blend_pixel(painter->bitmap, point_absolue, color);
    }
}

void painter_blit_bitmap_fast(Painter *paint, Bitmap *src, Rectangle src_rect, Rectangle dst_rect)
{
    for (int x = 0; x < dst_rect.width; x++)
    {
        for (int y = 0; y < dst_rect.height; y++)
        {
            color_t pix = bitmap_get_pixel(src, (Point){src_rect.X + x, src_rect.Y + y});
            painter_plot_pixel(paint, point_add(dst_rect.position, (Point){x, y}), pix);
        }
    }
}

void painter_blit_bitmap_scaled(Painter *paint, Bitmap *src, Rectangle src_rect, Rectangle dst_rect)
{
    for (int x = 0; x < dst_rect.width; x++)
    {
        for (int y = 0; y < dst_rect.height; y++)
        {
            float xx = x / (float)dst_rect.width;
            float yy = y / (float)dst_rect.height;

            color_t pix = bitmap_sample(src, src_rect, xx, yy);
            painter_plot_pixel(paint, point_add(dst_rect.position, (Point){x, y}), pix);
        }
    }
}

void painter_blit_bitmap(Painter *paint, Bitmap *src, Rectangle src_rect, Rectangle dst_rect)
{
    if (src_rect.width == dst_rect.width && src_rect.height == dst_rect.height)
    {
        painter_blit_bitmap_fast(paint, src, src_rect, dst_rect);
    }
    else
    {
        painter_blit_bitmap_scaled(paint, src, src_rect, dst_rect);
    }
}

void painter_clear(Painter *paint, color_t color)
{
    painter_clear_rect(paint, bitmap_bound(paint->bitmap), color);
}

void painter_clear_rect(Painter *paint, Rectangle rect, color_t color)
{
    Rectangle rect_absolue = rectangle_clip(paint->cliprect, rect);

    for (int xx = 0; xx < rect_absolue.width; xx++)
    {
        for (int yy = 0; yy < rect_absolue.height; yy++)
        {
            bitmap_set_pixel(
                paint->bitmap,
                (Point){rect_absolue.X + xx, rect_absolue.Y + yy},
                color);
        }
    }
}

void painter_fill_rect(Painter *paint, Rectangle rect, color_t color)
{
    Rectangle rect_absolue = rectangle_clip(paint->cliprect, rect);

    for (int xx = 0; xx < rect_absolue.width; xx++)
    {
        for (int yy = 0; yy < rect_absolue.height; yy++)
        {
            bitmap_blend_pixel(
                paint->bitmap,
                (Point){rect_absolue.X + xx, rect_absolue.Y + yy},
                color);
        }
    }
}

// TODO: void painter_fill_circle(Painter* paint, ...);

// TODO: void painter_fill_triangle(Painter* paint, ...);

void painter_draw_line_x_aligned(Painter *paint, int x, int start, int end, color_t color)
{
    for (int i = start; i < end; i++)
    {
        painter_plot_pixel(paint, (Point){x, i}, color);
    }
}

void painter_draw_line_y_aligned(Painter *paint, int y, int start, int end, color_t color)
{
    for (int i = start; i < end; i++)
    {
        painter_plot_pixel(paint, (Point){i, y}, color);
    }
}

void painter_draw_line_not_aligned(Painter *paint, Point a, Point b, color_t color)
{
    int dx = abs(b.X - a.X), sx = a.X < b.X ? 1 : -1;
    int dy = abs(b.Y - a.Y), sy = a.Y < b.Y ? 1 : -1;
    int err = (dx > dy ? dx : -dy) / 2, e2;

    for (;;)
    {
        painter_plot_pixel(paint, a, color);

        if (a.X == b.X && a.Y == b.Y)
            break;

        e2 = err;
        if (e2 > -dx)
        {
            err -= dy;
            a.X += sx;
        }
        if (e2 < dy)
        {
            err += dx;
            a.Y += sy;
        }
    }
}

void painter_draw_line(Painter *paint, Point a, Point b, color_t color)
{
    if (a.X == b.X)
    {
        painter_draw_line_x_aligned(paint, a.X, min(a.Y, b.Y), max(a.Y, b.Y), color);
    }
    else if (a.Y == b.Y)
    {
        painter_draw_line_y_aligned(paint, a.Y, min(a.X, b.X), max(a.X, b.X), color);
    }
    else
    {
        painter_draw_line_not_aligned(paint, a, b, color);
    }
}

void painter_draw_rect(Painter *paint, Rectangle rect, color_t color)
{
    painter_draw_line(paint, rect.position, point_sub(point_add(rect.position, point_x(rect.size)), (Point){1, 0}), color);
    painter_draw_line(paint, rect.position, point_sub(point_add(rect.position, point_y(rect.size)), (Point){0, 1}), color);
    painter_draw_line(paint, point_sub(point_add(rect.position, point_x(rect.size)), (Point){1, 0}), point_sub(point_add(rect.position, rect.size), (Point){1, 0}), color);
    painter_draw_line(paint, point_sub(point_add(rect.position, point_y(rect.size)), (Point){0, 1}), point_sub(point_add(rect.position, rect.size), (Point){0, 1}), color);
}

const int FONT_SIZE = 16;

void painter_blit_bitmap_sdf(Painter *paint, Bitmap *src, Rectangle src_rect, Rectangle dst_rect, float size, color_t color)
{
    const double FONT_GAMMA = 1.7;
    const double FONT_BUFFER = 0.80;

    for (int x = 0; x < dst_rect.width; x++)
    {
        for (int y = 0; y < dst_rect.height; y++)
        {
            double xx = x / (double)dst_rect.width;
            double yy = y / (double)dst_rect.height;

            color_t sample = bitmap_sample(src, src_rect, xx, yy);

            double distance = (sample.R / 150.0);
            double edge0 = FONT_BUFFER - FONT_GAMMA * 1.4142 / size;
            double edge1 = FONT_BUFFER + FONT_GAMMA * 1.4142 / size;

            double a = (distance - edge0) / (edge1 - edge0);
            if (a < 0.0)
                a = 0.0;
            if (a > 1.0)
                a = 1.0;
            a = a * a * (3 - 2 * a);

            color_t final = color;
            final.A = a * 255;

            painter_plot_pixel(paint, point_add(dst_rect.position, (Point){x, y}), final);
        }
    }
}

void painter_draw_glyph(Painter *paint, font_t *font, glyph_t *glyph, Point position, float size, color_t color)
{
    Rectangle dest;
    dest.position = point_sub(position, point_scale(glyph->origin, size / FONT_SIZE));
    dest.size = point_scale(glyph->bound.size, size / FONT_SIZE);

    painter_blit_bitmap_sdf(paint, font->bitmap, glyph->bound, dest, size, color);
}

void painter_draw_text(
    Painter *paint,
    font_t *font,
    const char *text,
    int text_size,
    Point position,
    float font_size,
    color_t color)
{
    Point current = position;

    for (int i = 0; i < text_size; i++)
    {
        glyph_t *glyph = font_glyph(font, text[i]);

        painter_draw_glyph(paint, font, glyph, current, font_size, color);

        current = point_add(current, (Point){glyph->advance * (font_size / FONT_SIZE), 0});
    }
}
