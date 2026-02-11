from __future__ import annotations

from . import _faulthandler, _version

__all__: list[str] = ["install"]

def _init() -> None: ...
def install(path: os.PathLike | str | bytes) -> None: ...

__version__: str
