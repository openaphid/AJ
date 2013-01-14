
/*
Copyright 2012 Aphid Mobile

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
 
   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/
/*
 * Copyright (C) 2008 Collabora Ltd.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "config.h"
#include "GOwnPtr.h"

#include <gio/gio.h>
#include <glib.h>

namespace ATF {

template <> void freeOwnedGPtr<GError>(GError* ptr)
{
    if (ptr)
        g_error_free(ptr);
}

template <> void freeOwnedGPtr<GList>(GList* ptr)
{
    g_list_free(ptr);
}

template <> void freeOwnedGPtr<GCond>(GCond* ptr)
{
    if (ptr)
        g_cond_free(ptr);
}

template <> void freeOwnedGPtr<GMutex>(GMutex* ptr)
{
    if (ptr)
        g_mutex_free(ptr);
}

template <> void freeOwnedGPtr<GPatternSpec>(GPatternSpec* ptr)
{
    if (ptr)
        g_pattern_spec_free(ptr);
}

template <> void freeOwnedGPtr<GDir>(GDir* ptr)
{
    if (ptr)
        g_dir_close(ptr);
}

template <> void freeOwnedGPtr<GFile>(GFile* ptr)
{
    if (ptr)
        g_object_unref(ptr);
}
} // namespace ATF
