"""Local tests for release metadata and archive contents (no network required)."""

import importlib.util
from pathlib import Path
import tempfile
import unittest
import zipfile

spec = importlib.util.spec_from_file_location("release", Path(__file__).with_name("release.py"))
release = importlib.util.module_from_spec(spec)
spec.loader.exec_module(release)


class ReleaseTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.write_metadata("1.2.3")

    def write_metadata(self, version):
        notes = "### Added\n\n- Release test.\n"
        (self.root / "CHANGELOG.md").write_text(f"# {version}\n\n{notes}")
        (self.root / "HISTORY.md").write_text(
            f"# Release history\n\n## {version}\n\n{notes}\n## 1.0.0\n\nOld notes.\n"
        )
        (self.root / "CMakeLists.txt").write_text("project(CPPToolkit VERSION 1.2.3)")
        (self.root / "ports/cpptoolkit").mkdir(parents=True, exist_ok=True)
        for path in ("vcpkg.json", "ports/cpptoolkit/vcpkg.json"):
            (self.root / path).write_text('{"version": "1.2.3"}')
        (self.root / "LICENSE").write_text("Test license")

    def test_valid_stable_and_prerelease_tags(self):
        for version in ("1.2.3", "1.2.3-a001", "1.2.3-b123", "1.2.3-b1"):
            with self.subTest(version=version):
                self.write_metadata(version)
                self.assertEqual(release.validate(self.root, f"release/{version}"), version)

    def test_invalid_tags(self):
        for tag in ("v1.2.3", "release/1.2", "release/01.2.3", "release/1.2.3-rc1",
                    "release/1.2.3-b", "release/1.2.3-b1/extra", "release/1.2.3\n"):
            with self.subTest(tag=tag), self.assertRaises(ValueError):
                release.validate(self.root, tag)

    def test_metadata_must_match_version_and_history(self):
        for path, text in (
            ("CHANGELOG.md", "# 9.9.9\n\nWrong version."),
            ("CHANGELOG.md", "# 1.2.3\n"),
            ("CHANGELOG.md", "# 1.2.3\n\n## 1.0.0\nOld release."),
            ("HISTORY.md", "# History\n\n## 1.2.3\nDifferent notes."),
            ("CMakeLists.txt", "project(CPPToolkit VERSION 9.9.9)"),
            ("vcpkg.json", '{"version": "9.9.9"}'),
        ):
            with self.subTest(path=path):
                self.write_metadata("1.2.3")
                (self.root / path).write_text(text)
                with self.assertRaises(ValueError):
                    release.validate(self.root, "release/1.2.3")

    def test_archives_use_dist_structure_and_only_release_files(self):
        for (system, arch), triplet in release.TARGETS.items():
            with self.subTest(system=system, arch=arch):
                installed = self.root / "installed"
                prefix = installed / triplet
                header = prefix / "include/cpptoolkit/ui/Export.h"
                header.parent.mkdir(parents=True, exist_ok=True)
                header.write_text("exports")
                extension = {"Linux": ".so", "Windows": ".dll", "Darwin": ".dylib"}[system]
                library_dir = prefix / ("bin" if system == "Windows" else "lib")
                library_dir.mkdir(parents=True, exist_ok=True)
                for module in ("mvvm", "net", "ui"):
                    (library_dir / f"cpptoolkit_{module}{extension}").write_text("shared")
                debug = prefix / "debug/lib/debug-only"
                debug.parent.mkdir(parents=True, exist_ok=True)
                debug.write_text("debug")
                copyright_file = prefix / "share/imgui/copyright"
                copyright_file.parent.mkdir(parents=True, exist_ok=True)
                copyright_file.write_text("Dependency license")
                (copyright_file.parent / "imgui-targets-debug.cmake").write_text("debug targets")
                stage = self.root / "dist/Release" / system / arch
                stage.mkdir(parents=True)
                (stage / "stale-build-output").write_text("must not ship")
                output = self.root / "archives"
                release.package(self.root, installed, system, arch, "1.2.3", output)
                archive = output / f"cpptoolkit-1.2.3-{system.lower()}-{arch}.zip"
                with zipfile.ZipFile(archive) as zipped:
                    names = zipped.namelist()
                    self.assertTrue(all(name.startswith(f"dist/Release/{system}/{arch}/") for name in names))
                    self.assertFalse(any("debug" in name or "stale" in name for name in names))
                    self.assertTrue(any(name.endswith("share/imgui/copyright") for name in names))
                    self.assertTrue(any(name.endswith("ui/Export.h") for name in names))
                self.assertTrue(archive.with_suffix(".zip.sha256").is_file())

    def test_missing_shared_libraries_fail(self):
        with self.assertRaises(ValueError):
            release.package(self.root, self.root / "missing", "Linux", "x64",
                            "1.2.3", self.root / "archives")


if __name__ == "__main__":
    unittest.main()
