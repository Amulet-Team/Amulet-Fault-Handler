import os
import subprocess
import sys
from pathlib import Path
import platform
from tempfile import TemporaryDirectory
from typing import TypeAlias, TYPE_CHECKING

from setuptools import setup, Extension, Command
from setuptools.command.build_ext import build_ext

import versioneer


def fix_path(path: str | os.PathLike[str]) -> str:
    return os.path.realpath(path).replace(os.sep, "/")


cmdclass: dict[str, type[Command]] = versioneer.get_cmdclass()

if TYPE_CHECKING:
    BuildExt: TypeAlias = build_ext
else:
    BuildExt = cmdclass.get("build_ext", build_ext)


class CMakeBuild(BuildExt):
    def build_extension(self, ext: Extension) -> None:
        import pybind11

        ext_dir = (
            (Path.cwd() / self.get_ext_fullpath("")).parent.resolve()
            / "amulet_faulthandler"
        )
        faulthandler_src_dir = (
            Path.cwd() / "src" / "amulet_faulthandler" if self.editable_mode else ext_dir
        )

        platform_args = []
        if sys.platform == "win32":
            platform_args.extend(["-G", "Visual Studio 17 2022"])
            if sys.maxsize > 2**32:
                platform_args.extend(["-A", "x64"])
            else:
                platform_args.extend(["-A", "Win32"])
            platform_args.extend(["-T", "v143"])
        elif sys.platform == "darwin":
            if platform.machine() == "arm64":
                platform_args.append("-DCMAKE_OSX_ARCHITECTURES=x86_64;arm64")

        if subprocess.run(["cmake", "--version"]).returncode:
            raise RuntimeError("Could not find cmake")
        with TemporaryDirectory() as tempdir:
            if subprocess.run(
                [
                    "cmake",
                    *platform_args,
                    f"-DPYTHON_EXECUTABLE={sys.executable}",
                    f"-Dpybind11_DIR={fix_path(pybind11.get_cmake_dir())}",
                    f"-Damulet_faulthandler_DIR={fix_path(faulthandler_src_dir)}",
                    f"-DAMULET_FAULTHANDLER_EXT_DIR={fix_path(ext_dir)}",
                    f"-DCMAKE_INSTALL_PREFIX=install",
                    "-B",
                    tempdir,
                ]
            ).returncode:
                raise RuntimeError("Error configuring amulet-faulthandler")
            if subprocess.run(
                ["cmake", "--build", tempdir, "--config", "Release"]
            ).returncode:
                raise RuntimeError("Error building amulet-faulthandler")
            if subprocess.run(
                ["cmake", "--install", tempdir, "--config", "Release"]
            ).returncode:
                raise RuntimeError("Error installing amulet-faulthandler")


cmdclass["build_ext"] = CMakeBuild  # type: ignore


setup(
    version=versioneer.get_version(),
    cmdclass=cmdclass,
    ext_modules=[Extension("amulet_faulthandler._faulthandler", [])]
    * (not os.environ.get("AMULET_SKIP_COMPILE", None)),
)
