from __future__ import annotations

from . import _version

__version__ = _version.get_versions()["version"]


def _init() -> None:
    import sys

    from ._faulthandler import init

    init(sys.modules[__name__])


_init()
del _init
