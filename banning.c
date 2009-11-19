/*
 * banning.c - ewindow banning management
 *
 * Copyright © 2007-2009 Julien Danjou <julien@danjou.info>
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

#include "banning.h"
#include "objects/client.h"
#include "objects/tag.h"

/** True if the banning on this screen needs to be updated */
static bool need_lazy_banning;

static int
banning_need_update(lua_State *L)
{
    /* We update the complete banning only once per main loop to avoid
     * excessive updates...  */
    need_lazy_banning = true;

    /* But if an ewindow will be banned in our next update we unfocus it now. */
    foreach(ewindow, _G_ewindows)
        if(ewindow_isvisible(*ewindow))
            window_ban_unfocus((window_t *) *ewindow);

    return 0;
}

void
banning_init(void)
{
    luaA_class_connect_signal(globalconf.L, (lua_class_t *) &client_class, "property::minimized", banning_need_update);
    luaA_class_connect_signal(globalconf.L, (lua_class_t *) &client_class, "property::hidden", banning_need_update);
    luaA_class_connect_signal(globalconf.L, (lua_class_t *) &client_class, "property::sticky", banning_need_update);
    luaA_class_connect_signal(globalconf.L, (lua_class_t *) &client_class, "tagged", banning_need_update);
    luaA_class_connect_signal(globalconf.L, (lua_class_t *) &client_class, "untagged", banning_need_update);
    luaA_class_connect_signal(globalconf.L, &tag_class, "property::selected", banning_need_update);
    luaA_class_connect_signal(globalconf.L, &tag_class, "property::attached", banning_need_update);
}

void
banning_refresh(void)
{
    if(!need_lazy_banning)
        return;

    foreach(ewindow, _G_ewindows)
        if(ewindow_isvisible(*ewindow))
            window_unban((window_t *) *ewindow);

    /* Some people disliked the short flicker of background, so we first unban everything.
     * Afterwards we ban everything we don't want. This should avoid that. */
    foreach(ewindow, _G_ewindows)
        if(!ewindow_isvisible(*ewindow))
            window_ban((window_t *) *ewindow);

    need_lazy_banning = false;
}

// vim: filetype=c:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=80
