/* Copyright (C) 2012 Siana Gearz
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA */

#include "llviewerprecompiledheaders.h"

#include "llversionviewer.h"

#include "sgversion.h"
#include "llviewercontrol.h"


std::string gVersionChannel()
{
	static LLCachedControl<std::string> SpecifiedChannel("SpecifiedChannel");
	return SpecifiedChannel;
}

S32 gVersionMajor()
{
	static LLCachedControl<U32> SpecifiedVersionMaj("SpecifiedVersionMaj",LL_VERSION_MAJOR);
	return (S32)SpecifiedVersionMaj;
}

S32 gVersionMinor()
{
	static LLCachedControl<U32> SpecifiedVersionMin("SpecifiedVersionMin",LL_VERSION_MINOR);
	return (S32)SpecifiedVersionMin;
}

S32 gVersionPatch()
{
	static LLCachedControl<U32> SpecifiedVersionPatch("SpecifiedVersionPatch",LL_VERSION_PATCH);
	return (S32)SpecifiedVersionPatch;
}

S32 gVersionBuild()
{
	static LLCachedControl<U32> SpecifiedVersionBuild("SpecifiedVersionBuild",LL_VERSION_BUILD);
	return (S32)SpecifiedVersionBuild;
}

#if LL_DARWIN
const char* gVersionBundleID = LL_VERSION_BUNDLE_ID;
#endif