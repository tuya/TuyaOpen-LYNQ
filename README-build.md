# L511G build entry points

## Vendor SDK sync checklist

`PLAT/` is the chip vendor's SDK and gets replaced wholesale when a new drop
arrives. **The TuyaOpen port modifies exactly two things in it**, both listed
here so a sync is mechanical rather than archaeology:

| File | Change | Why it cannot be avoided |
|---|---|---|
| `PLAT/GccBuild_L511G_Y7PM.sh`, `..._Y7PVM.sh` | `PROJECT_NAME` and `GCCLIB_PATH` become `${VAR:-<OEM default>}` (2 hunks each) | Both are hard-set with plain `export`, which overrides what the TuyaOpen build exports. Everything else in these scripts runs unmodified. |
| `PLAT/middleware/developed/debug/inc/debug_trace.h` | `UNILOG_TuyaOpen = 0` added to `UniLogCustModIdType_e` | The log database takes module names from this enum, and the file marks the spot: `!!! for customer SDK development, please add here !!!!` |

Nothing else under `PLAT/` is touched. New files are added
(`build_tuyaopen_*.sh`, `project/ec7xx_ref_1h00/ap/apps/tuyaopen/`), which a
sync will not conflict with.

If a sync does overwrite the `${VAR:-...}` forms, `build_tuyaopen_*.sh` checks
for them and **refuses to build**. Without that check the build would quietly
produce an `app_demo` image and `build_example.py` — which judges success by the
packaged `.binpkg` existing — would publish it as the application's firmware.

Deliberately *not* modified, though it was tempting:

* `PLAT/project/ec7xx_ref_1h00/ap/apps/tuyaos_adapter/` — the OEM's TuyaOS 3.x
  project. Nothing here builds it (its Makefile points at a `sdk/` level that
  this flattened layout does not have), so it is left exactly as shipped. The
  tuyaopen project does *read* four files from it — see below — but changes
  none.
* `PLAT/build_g_pm.sh` / `build_g_pvm.sh` — upstream's entry points for that
  project.

## Boards and scripts

| Board (`tos.py config choice`) | Vendor target | TuyaOpen entry | OEM script it runs |
|---|---|---|---|
| `L511G_Y7PM` (default) | `L511G_Y7PM` | `PLAT/build_tuyaopen_L511G_Y7PM.sh` | `PLAT/GccBuild_L511G_Y7PM.sh` |
| `L511G_Y7PVM` | `L511G_Y7PVM` | `PLAT/build_tuyaopen_L511G_Y7PVM.sh` | `PLAT/GccBuild_L511G_Y7PVM.sh` |

`build_tuyaopen_*.sh` is a ~40-line wrapper: it checks the two overrides are
present, exports `PROJECT_NAME=tuyaopen`, handles the `clean` argument
`tos.py clean` needs, and `exec`s the OEM script. Every feature switch, the
merge/package path, `fcelf` and `mem_map` stay the vendor's, so there is no
second copy to drift.

### The targets are not interchangeable

`PLAT/mbtk/target/L511G_Y7PVM/Makefile.inc` sets `IMS_ENABLE=true` and
`CHIP_TYPE_MBTK=ec718pvm` and turns `MIDDLEWARE_FOTA_FS_ENABLE` and
`MBTK_OPENCPU_SSL` off; the `L511G_Y7PM` one is the target upstream tuned for
TuyaOpen (it shrinks the FOTA region to give the internal LFS room for the Tuya
KV database). Both package the same CP image. Built as Y7PVM, this module's AP
runs normally but the modem never registers and `AT+CSQ` answers `99,99`.

## The application project

`PLAT/project/ec7xx_ref_1h00/ap/apps/tuyaopen/` holds only what is its own:

* `src/main.c` — `main_entry` / `app_init`, ending in `tuya_app_main()`;
* `inc/mode_config.h` — this project's OEM feature selection. Per-project by
  design (`app_demo` has its own) and pulled in by every subsys source through
  `subsys/common/inc/subsys.h`;
* `GCC/Makefile` — `app_demo`'s, with the demo sources replaced by this
  project's, the TKL implementation from `tuyaos/`, and TuyaOpen's
  `libtuyaos.a` / `libtuyaapp.a`.

The board glue is **read from** the `tuyaos_adapter` project rather than copied:
`src/bsp_custom.c`, `src/rawData.c`, `inc/RTE_Device.h` and `inc/bsp_custom.h`
hold the Tuya-adapted versions for this same module. `Makefile.rules` resolves
sources through `vpath %.c $(TOP)` and `BUILDDIR` is per-`PROJECT`, so those
objects build into this project's own tree, and a vendor update to them flows
through without a merge.

`tuyaos/` — the TKL adapter — is the port itself, not vendor SDK, and is
maintained normally.
