"""Verify portable USB/UART config paths in collected platform outputs."""

from pathlib import Path
import shutil
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import build_example as build


class TestPackageConfigs(unittest.TestCase):
    def test_project_version_and_directory_changes(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            vendor = root / "L511G_Y7PM_download"
            vendor.mkdir()
            (vendor / "L511G_Y7PM.binpkg").write_bytes(b"package fixture")
            original = b"arg_pkg_path_val = .\\L511G_Y7PM.binpkg\nother = unchanged\n"
            names = ("config_custom_usb.ini", "config_custom_uart.ini")
            for name in names:
                (vendor / name).write_bytes(original)
            with patch.object(build, "download_dir", return_value=str(vendor)):
                for app, version in (("first-app", "1.0.0"), ("second-app", "2.0.1")):
                    output = root / "bin"
                    params = {"CONFIG_PROJECT_NAME": app, "CONFIG_PROJECT_VERSION": version,
                              "BIN_OUTPUT_DIR": str(output)}
                    self.assertTrue(build.collect_output({"build_pro_name": "L511G_Y7PM"}, params))
                    copied = root / "moved" / f"{app}_{version}"
                    shutil.copytree(output, copied)
                    filename = f"{app}_QIO_{version}.binpkg"
                    for name in names:
                        config = copied / vendor.name / name
                        self.assertEqual(config.read_bytes(),
                                         b"arg_pkg_path_val = ..\\" + filename.encode() +
                                         b"\nother = unchanged\n")
                        self.assertTrue((config.parent / ".." / filename).is_file())
                        self.assertEqual((vendor / name).read_bytes(), original)

    def test_malformed_config_fails(self):
        with tempfile.TemporaryDirectory() as temp:
            (Path(temp) / "config_custom_usb.ini").write_bytes(b"[package_info]\n")
            with self.assertRaises(ValueError):
                build.bind_package_configs(temp, "app_QIO_1.0.0.binpkg")


if __name__ == "__main__":
    unittest.main()
