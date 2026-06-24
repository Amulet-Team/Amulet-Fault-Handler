from __future__ import annotations
import os

__all__: list[str] = ["install"]

def install(path: os.PathLike | str | bytes, full_dump: bool) -> None: ...

__version__: str
