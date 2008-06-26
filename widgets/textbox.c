/*
 * textbox.c - text box widget
 *
 * Copyright © 2007-2008 Julien Danjou <julien@danjou.info>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include "widget.h"
#include "screen.h"
#include "common/util.h"
#include "common/configopts.h"
#include "common/tokenize.h"

extern awesome_t globalconf;

/** The textbox private data structure */
typedef struct
{
    /** Textbox text */
    char *text;
    /** Textbox width */
    int width;
} textbox_data_t;

/** Draw a textbox widget.
 * \param ctx The draw context.
 * \param screen The screen.
 * \param w The widget node we are linked from.
 * \param offset Offset to draw at.
 * \param used The size used on the element.
 * \param p A pointer to the object we're draw onto.
 * \return The width used.
 */
static int
textbox_draw(draw_context_t *ctx, int screen __attribute__ ((unused)),
             widget_node_t *w,
             int offset, int used,
             void *p __attribute__ ((unused)))
{
    textbox_data_t *d = w->widget->data;
    draw_parser_data_t pdata, *pdata_arg = NULL;

    if(d->width)
        w->area.width = d->width;
    else if(w->widget->align == AlignFlex)
        w->area.width = ctx->width - used;
    else
    {
        draw_parser_data_init(&pdata);
        w->area.width = MIN(draw_text_extents(ctx->connection,
                                              ctx->phys_screen,
                                              globalconf.font, d->text, &pdata).width,
                            ctx->width - used);
        if(pdata.bg_image)
            w->area.width = MAX(w->area.width, pdata.bg_resize ? w->area.height : pdata.bg_image->width);

        pdata_arg = &pdata;
    }

    w->area.height = ctx->height;

    w->area.x = widget_calculate_offset(ctx->width,
                                        w->area.width,
                                        offset,
                                        w->widget->align);
    w->area.y = 0;

    draw_text(ctx, globalconf.font, w->area, d->text, pdata_arg);
    if (pdata_arg)
        draw_parser_data_wipe(pdata_arg);

    return w->area.width;
}

/** Delete a textbox widget.
 * \param w The widget to destroy.
 */
static void
textbox_destructor(widget_t *w)
{
    textbox_data_t *d = w->data;
    p_delete(&d->text);
    p_delete(&d);
}

/** Set the text of a textbox.
 * \param L The Lua VM state.
 * \return The number of elements pushed on stack.
 * \lstack
 * \lvalue A widget.
 * \lparam The text to set.
 */
static int
luaA_textbox_text_set(lua_State *L)
{
    widget_t **widget = luaA_checkudata(L, 1, "widget");
    const char *text = luaL_checkstring(L, 2);
    textbox_data_t *d = (*widget)->data;

    p_delete(&d->text);
    a_iso2utf8(text, &d->text);

    widget_invalidate_bywidget(*widget);

    return 0;
}

/** Set the width of a textbox.
 * \param L The Lua VM state.
 * \return The number of elements pushed on stack.
 * \lstack
 * \lvalue A widget.
 * \lparam The width to set.
 */
static int
luaA_textbox_width_set(lua_State *L)
{
    widget_t **widget = luaA_checkudata(L, 1, "widget");
    int width = luaL_checknumber(L, 2);
    textbox_data_t *d = (*widget)->data;

    d->width = width;

    widget_invalidate_bywidget(*widget);

    return 0;
}

/** The __index method for a textbox object.
 * \param L The Lua VM state.
 * \return The number of elements pushed on stack.
 * \param The numbre of elements return on the stack.
 */
static int
luaA_textbox_index(lua_State *L)
{
    size_t len;
    const char *attr = luaL_checklstring(L, 2, &len);

    switch(a_tokenize(attr, len))
    {
      case A_TK_TEXT_SET:
        lua_pushcfunction(L, luaA_textbox_text_set);
        return 1;
      case A_TK_WIDTH_SET:
        lua_pushcfunction(L, luaA_textbox_width_set);
        return 1;
      default:
        return 0;
    }
}

/** Create a new textbox widget.
 * \param align Widget alignment.
 * \return A brand new widget.
 */
widget_t *
textbox_new(alignment_t align)
{
    widget_t *w;
    textbox_data_t *d;

    w = p_new(widget_t, 1);
    widget_common_new(w);
    w->align = align;
    w->draw = textbox_draw;
    w->index = luaA_textbox_index;
    w->destructor = textbox_destructor;
    w->data = d = p_new(textbox_data_t, 1);

    return w;
}
// vim: filetype=c:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=80
