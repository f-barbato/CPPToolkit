"""Validate release metadata and archive a release-only installed vcpkg prefix."""

import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import zipfile


VERSION = r"(?:0|[1-9]\d*)\.(?:0|[1-9]\d*)\.(?:0|[1-9]\d*)(?:-[ab]\d+)?"
TARGETS = {
    ("Linux", "x64"): "x64-linux-dynamic",
    ("Linux", "arm64"): "arm64-linux-dynamic",
    ("Windows", "x64"): "x64-windows",
    ("Windows", "arm64"): "arm64-windows",
    ("Darwin", "x64"): "x64-osx-dynamic",
    ("Darwin", "arm64"): "arm64-osx-dynamic",
}


def validate(root, tag):
    match = re.fullmatch(rf"release/({VERSION})", tag)
    if not match:
        raise ValueError("Expected release/major.minor.patch with optional -aNNN or -bNNN")
    version = match[1]
    changelog = (root / "CHANGELOG.md").read_text(encoding="utf-8")
    heading, separator, body = changelog.partition("\n")
    if heading.strip() != f"# {version}" or not separator or not body.strip():
        raise ValueError(f"CHANGELOG.md must start with '# {version}' and contain release notes")
    if re.search(rf"^#{{1,2}} (?:\[)?{VERSION}(?:\])?(?:\s|$)", body, re.MULTILINE):
        raise ValueError("CHANGELOG.md must contain only the current version, not historical entries")
    history = (root / "HISTORY.md").read_text(encoding="utf-8")
    entry = re.search(
        rf"^## {re.escape(version)}\n(.*?)(?=^## |\Z)", history, re.MULTILINE | re.DOTALL
    )
    if not entry or entry[1].strip() != body.strip():
        raise ValueError("HISTORY.md must contain the same release notes under '## <version>'")
    project = (root / "CMakeLists.txt").read_text(encoding="utf-8")
    project_version = re.search(r"\bVERSION\s+(\d+\.\d+\.\d+)", project)
    if not project_version or project_version[1] != version.split("-")[0]:
        raise ValueError("CMake project VERSION must match the tag's major.minor.patch")
    for manifest in ("vcpkg.json", "ports/cpptoolkit/vcpkg.json"):
        package_version = json.loads((root / manifest).read_text(encoding="utf-8"))["version"]
        if package_version != version.split("-")[0]:
            raise ValueError(f"{manifest} version must match the tag's major.minor.patch")
    return version


def package(root, installed, system, arch, version, output):
    triplet = TARGETS[(system, arch)]
    prefix = installed / triplet
    if not (prefix / "include/cpptoolkit/ui/Export.h").is_file():
        raise ValueError("Missing installed UI export header; build the complete shared package first")
    extension = {"Windows": ".dll", "Linux": ".so", "Darwin": ".dylib"}[system]
    libraries = list((prefix / ("bin" if system == "Windows" else "lib")).glob(f"*{extension}*"))
    for module in ("mvvm", "net", "ui"):
        if not any(f"cpptoolkit_{module}" in file.name for file in libraries):
            raise ValueError(f"Missing shared {module} library for {system}/{arch}")
    stage = root / "dist/Release" / system / arch
    # Do not erase build outputs; archive only files from the installed prefix.
    allowed = []
    for directory in ("include", "lib", "bin", "share"):
        source = prefix / directory
        if not source.exists():
            continue
        for file in sorted(source.rglob("*")):
            if not file.is_file():
                continue
            if file.name.lower().endswith("-debug.cmake"):
                continue
            relative = file.relative_to(prefix)
            destination = stage / relative
            destination.parent.mkdir(parents=True, exist_ok=True)
            # Dereference Unix SONAME links: ZIP extractors may not preserve links.
            shutil.copyfile(file, destination)
            allowed.append(relative)
    for name in ("LICENSE", "CHANGELOG.md", "HISTORY.md"):
        stage.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(root / name, stage / name)
        allowed.append(Path(name))
    output.mkdir(parents=True, exist_ok=True)
    archive = output / f"cpptoolkit-{version}-{system.lower()}-{arch}.zip"
    archive_root = Path("dist/Release") / system / arch
    with zipfile.ZipFile(archive, "w", zipfile.ZIP_DEFLATED) as zip_file:
        for relative in allowed:
            zip_file.write(stage / relative, (archive_root / relative).as_posix())
    digest = hashlib.sha256(archive.read_bytes()).hexdigest()
    archive.with_suffix(".zip.sha256").write_text(f"{digest}  {archive.name}\n", encoding="utf-8")
    return stage


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("command", choices=["validate", "package"])
    parser.add_argument("--root", type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument("--tag", required=True)
    parser.add_argument("--installed", type=Path)
    parser.add_argument("--system", choices=["Linux", "Windows", "Darwin"])
    parser.add_argument("--arch", choices=["x64", "arm64"])
    parser.add_argument("--output", type=Path, default=Path("build/release-archives"))
    args = parser.parse_args()
    version = validate(args.root, args.tag)
    if args.command == "package":
        if not all((args.installed, args.system, args.arch)):
            parser.error("package requires --installed, --system and --arch")
        stage = package(args.root, args.installed, args.system, args.arch, version, args.output)
        print(stage)
    elif os.environ.get("GITHUB_OUTPUT"):
        with open(os.environ["GITHUB_OUTPUT"], "a", encoding="utf-8") as output:
            output.write(f"version={version}\nprerelease={'true' if '-' in version else 'false'}\n")
    else:
        print(version)


if __name__ == "__main__":
    main()
