# L511G build entry points

Two module variants, each with its own vendor target:

| Board (`tos.py config choice`) | Vendor target | TuyaOpen entry | OEM script it runs |
|---|---|---|---|
| `L511G_Y7PM` (default) | `L511G_Y7PM` | `PLAT/build_tuyaopen_L511G_Y7PM.sh` | `PLAT/GccBuild_L511G_Y7PM.sh` |
| `L511G_Y7PVM` | `L511G_Y7PVM` | `PLAT/build_tuyaopen_L511G_Y7PVM.sh` | `PLAT/GccBuild_L511G_Y7PVM.sh` |

**The build is the OEM's.** `build_tuyaopen_*.sh` is a ~20-line wrapper that
exports `PROJECT_NAME=tuyaopen`, handles the `clean` argument `tos.py clean`
needs, and then `exec`s the OEM `GccBuild` script unmodified. Every feature
switch, the merge/package path, `fcelf` and `mem_map` are the vendor's, so
there is no second copy to drift.

The OEM scripts were changed in three small ways, all of which leave their
standalone behaviour identical:

* `PROJECT_NAME=${PROJECT_NAME:-app_demo}` and
  `GCCLIB_PATH=${GCCLIB_PATH:-<platform/tools>}`, so the wrapper (and
  `build_example.py`, which exports the toolchain path) can override them;
* `mkdir -p` of the build-log directory, because `tee` opens it before `make`
  runs;
* restoring the execute bit on `PLAT/tools`, which a zip/Windows checkout drops
  from `fcelf` / `LogPrePass` / `ecsecure`.

## The targets are not interchangeable

`PLAT/mbtk/target/L511G_Y7PVM/Makefile.inc` sets `IMS_ENABLE=true` and
`CHIP_TYPE_MBTK=ec718pvm` and turns `MIDDLEWARE_FOTA_FS_ENABLE` and
`MBTK_OPENCPU_SSL` off; the `L511G_Y7PM` one is the target upstream tuned for
TuyaOpen (it shrinks the FOTA region to give the internal LFS room for the Tuya
KV database). Both package the same CP image. Built as Y7PVM, this module's AP
runs normally but the modem never registers and `AT+CSQ` answers `99,99`.

## The application project

`PLAT/project/ec7xx_ref_1h00/ap/apps/tuyaopen/` holds only what is actually its
own:

* `src/main.c` — `main_entry` / `app_init`, ending in `tuya_app_main()`;
* `inc/mode_config.h` — this project's OEM feature selection. Per-project by
  design (`app_demo` has its own) and pulled in by every subsys source through
  `subsys/common/inc/subsys.h`;
* `GCC/Makefile` — `app_demo`'s, with the demo sources replaced by this
  project's, the TKL implementation from `tuyaos/`, and TuyaOpen's
  `libtuyaos.a` / `libtuyaapp.a`.

The board glue is **not** copied here: `bsp_custom.c`, `rawData.c`,
`RTE_Device.h` and `bsp_custom.h` come from the `tuyaos_adapter` project, which
holds the Tuya-adapted versions for this same module. `Makefile.rules` resolves
sources through `vpath %.c $(TOP)` and `BUILDDIR` is per-`PROJECT`, so those
objects build into this project's own tree.

`PLAT/build_g_pm.sh` / `build_g_pvm.sh` are upstream's own entry points and
build the `tuyaos_adapter` **library** project; they are left untouched.
