# L511C-Y7PM TuyaOpen build

This platform repository contains the L511C-Y7PM TuyaOpen adaptation. From a TuyaOpen SDK checkout with this repository available at `platform/L511C_Y7PM`, select the board config and build the switch demo:

```sh
cd apps/tuya_cloud/switch_demo
tos.py config choice -d -c L511C_Y7PM
tos.py build
```

Set a valid product PID and device authorization in the demo's existing configuration before connecting a device. The demo reports a logical switch DP; it does not drive a physical switch.

The build wrapper invokes the vendor `sdk/PLAT/GccBuild_L511C_Y7PM.sh` flow. That script builds the bootloader and `tuyaos_adapter` targets, then packages the result. Vendor feature flags, including `AUDIO_ENABLE=true`, follow the official L511C-Y7PM target settings required by its prebuilt ABI and memory layout. This does not enable the TuyaOpen board's CAT1 player option. No L511G target or prebuilt library is used.

Cellular uses the vendor MDS registration callback. The vendor app calls `main()` from `app_init()`, and the platform adapter's `main()` starts `tuya_app_main()`. This establishes compile-time integration; it does not verify hardware behavior. Hardware acceptance should cover cellular registration, cloud binding, App DP commands and device DP reports, CLI `switch on/off`, and KV persistence after restart. Physical behavior still requires validation on L511C-Y7PM hardware, and no physical GPIO switch, LED, or unverified peripheral is enabled.

The `.bin` file emitted in the project's `dist/` directory is a TuyaOpen naming alias containing the vendor `.binpkg` payload. For actual flashing, use the vendor-generated `sdk/PLAT/L511C_Y7PM_download/L511C_Y7PM.binpkg`.

The TuyaOpen SDK pins platform revisions in `platform/platform_config.yaml`. When integrating this adaptation, update the `L511C_Y7PM` entry to a platform commit that contains these changes.
