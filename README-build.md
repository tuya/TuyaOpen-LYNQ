# L511G build entry points

Two module variants, each with its own vendor target. The **build script is
generated from the OEM `GccBuild_L511G_Y7P*.sh`**, so every feature switch is
the one the vendor validated on the module; regenerate with
`derive_build_script.py` if the OEM script changes.

| Board (`tos.py config choice`) | Vendor target | Build script |
|---|---|---|
| `L511G_Y7PM` (default) | `L511G_Y7PM` | `PLAT/build_tuyaopen_L511G_Y7PM.sh` |
| `L511G_Y7PVM` | `L511G_Y7PVM` | `PLAT/build_tuyaopen_L511G_Y7PVM.sh` |

The targets are **not** interchangeable. `PLAT/mbtk/target/L511G_Y7PVM/Makefile.inc`
sets `IMS_ENABLE=true` and `CHIP_TYPE_MBTK=ec718pvm` and turns
`MIDDLEWARE_FOTA_FS_ENABLE` and `MBTK_OPENCPU_SSL` off; the `L511G_Y7PM` one is
the target upstream tuned for TuyaOpen (it shrinks the FOTA region to give the
internal LFS room for the Tuya KV database). Building Y7PVM firmware for a Y7PM
module leaves the AP running but the modem unregistered.

Each generated script differs from its OEM original only in:

* `PROJECT_NAME=tuyaopen` — the application project under
  `PLAT/project/ec7xx_ref_1h00/ap/apps/tuyaopen`, which links TuyaOpen's
  `libtuyaos.a` / `libtuyaapp.a` in place of the OEM demo sources;
* `GCCLIB_PATH` taken from the environment (`build_example.py` exports the
  toolchain `platform_prepare.py` installed), with a repo-relative fallback;
* the execute bit restored on `PLAT/tools`, which a zip/Windows checkout drops;
* a `clean` argument, for `tos.py clean`;
* `mkdir -p` of the build-log directory, because `tee` opens it before `make`
  runs.

`PLAT/build_g_pm.sh` / `build_g_pvm.sh` are upstream's own entry points and
build the `tuyaos_adapter` **library** project; they are left untouched.
