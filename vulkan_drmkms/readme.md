# VULKAN_DRMKMS

Experimental demo of using vulkan api to render directly on the display via `DRM` interface, (thus without X or any other graphics framework,) using `libdrm`. 

## Fixing bugs for `libdrm` header files

On the current release (2.4.106-1) of `libdrm` package on Arch Linux, there is a bug in two header files which prevents the project from being compiled. To fix them,

1. Locate the header file `/usr/include/xf86drm.h`, which should be part of `libdrm` package. Find the line `#include <drm.h>` and replace `<drm.h>` with `<libdrm/drm.h>`.

2. Locate the header fiel `/usr/include/xf86drmMode.h`, which is also part of `libdrm` package. Find the line `#include <drm.h>` and replace `<drm.h>` with `<libdrm/drm.h>`. Then, find the line `#include <drm_mode.h>` and replace `<drm_mode.h>` with `<libdrm/drm_mode.h>`.

