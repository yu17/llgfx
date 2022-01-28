# DRMGFX

Despite being called DRMGFX, this demo does not use `libdrm`. Instead, it actually interacts directly with the drm interface, or specifically, `/dev/dri/card*`.

Works only when `KMS` is enabled, which is usually enabled on most modern desktop environments. (Otherwise, there simply would be no `dri` directory in `dev`.)
