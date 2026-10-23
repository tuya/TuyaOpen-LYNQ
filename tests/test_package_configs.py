"""Check the platform's application-package output and portable config paths."""

import importlib.util
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest
from unittest.mock import patch


spec = importlib.util.spec_from_file_location(
    "l511c_build_example", Path(__file__).resolve().parents[1] / "build_example.py"
)
build = importlib.util.module_from_spec(spec)
spec.loader.exec_module(build)


class TestPackageConfigs(unittest.TestCase):
    def test_build_output_tracks_project_version_and_can_be_moved(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            plat = root / "vendor"
            download = plat / "L511C_Y7PM_download"
            download.mkdir(parents=True)
            (plat / "tools").mkdir()
            (plat / "tools/fcelf").write_bytes(b"fixture")
            vendor_script = plat / "build_c_pm.sh"
            vendor_script.write_bytes(b"fixture")
            package = download / "L511C_Y7PM.binpkg"
            original = (
                b"[package_info]\r\n"
                b"arg_pkg_path_val = .\\Custom.binpkg\r\n"
                b"pkg_extract_exe = .\\fcelf.exe\r\n"
            )
            configs = ("config_custom_usb.ini", "config_custom_uart.ini")
            for name in configs:
                (download / name).write_bytes(original)

            def vendor_build(*args, **kwargs):
                package.write_bytes(b"package fixture")
                return subprocess.CompletedProcess(args[0], 0)

            with patch.object(build, "PLAT", str(plat)), \
                 patch.object(build, "DOWNLOAD_DIR", str(download)), \
                 patch.object(build, "PACKAGE", str(package)), \
                 patch.object(build, "VENDOR_BUILD_SCRIPT", str(vendor_script)), \
                 patch.object(build.subprocess, "run", side_effect=vendor_build):
                for app, version in (("display-test", "1.0.1"), ("other-app", "2.0.0")):
                    output = root / app / ".build/bin"
                    params = {
                        "CONFIG_PROJECT_NAME": app,
                        "CONFIG_PROJECT_VERSION": version,
                        "BIN_OUTPUT_DIR": str(output),
                    }
                    with patch.object(build, "read_params", return_value=params):
                        build.run(str(root), "build")
                    published = root / "copied_dist" / f"{app}_{version}"
                    shutil.copytree(output, published)
                    filename = f"{app}_QIO_{version}.binpkg"
                    self.assertTrue((published / filename).is_file())
                    for name in configs:
                        config = published / download.name / name
                        content = config.read_bytes()
                        expected = b"arg_pkg_path_val = ..\\" + filename.encode() + b"\r\n"
                        self.assertIn(expected, content)
                        self.assertIn(b"pkg_extract_exe = .\\fcelf.exe\r\n", content)
                        self.assertEqual((download / name).read_bytes(), original)

    def test_malformed_config_fails(self):
        with tempfile.TemporaryDirectory() as temp:
            config = Path(temp) / "config_custom_usb.ini"
            config.write_bytes(b"[package_info]\r\n")
            with self.assertRaisesRegex(ValueError, "expected one arg_pkg_path_val"):
                build.bind_package_configs(temp, "app_QIO_1.0.0.binpkg")


if __name__ == "__main__":
    unittest.main()
