import unittest
import multiprocessing
import os
import sys
from tempfile import TemporaryDirectory
import faulthandler
from collections.abc import Callable
from typing import Any

import amulet_faulthandler

from _test_amulet_faulthandler import (
    throw_access_violation,
    throw_stack_overflow,
    throw_double_free,
    throw_abort,
)


def setup_faulthandler(log_path: str, dump_path: str):
    log_file = open(log_path, "w")
    faulthandler.enable(log_file)
    amulet_faulthandler.install(dump_path, False)


def subprocess_main(func: Callable[[], Any], log_path: str, dump_path: str):
    setup_faulthandler(log_path, dump_path)
    func()


class FaulthandlerTestCase(unittest.TestCase):
    def _call_in_subprocess(
        self, func: Callable[[], Any], exit_code: int, crash_optional: bool = False
    ):
        with TemporaryDirectory() as temp_directory:
            p = multiprocessing.Process(
                target=subprocess_main,
                args=(
                    func,
                    os.path.join(temp_directory, "log.txt"),
                    os.path.join(temp_directory, "crash.dmp"),
                ),
            )
            p.start()
            p.join()
            if crash_optional and not p.exitcode:
                # Some crashes (heap corruption) do not manifest on all platforms (windows arm64)
                return
            if sys.platform == "win32":
                self.assertEqual(exit_code, p.exitcode)
                self.assertTrue(
                    os.path.isfile(os.path.join(temp_directory, "crash_0.dmp"))
                )
            else:
                self.assertTrue(p.exitcode)

    def test_access_violation(self) -> None:
        self._call_in_subprocess(throw_access_violation, 0xC0000005)

    def test_stack_overflow(self) -> None:
        self._call_in_subprocess(throw_stack_overflow, 0xC00000FD)

    def test_throw_heap_corruption(self) -> None:
        self._call_in_subprocess(throw_double_free, 0xC0000374, True)

    def test_abort(self) -> None:
        self._call_in_subprocess(throw_abort, 3)


if __name__ == "__main__":
    unittest.main()
