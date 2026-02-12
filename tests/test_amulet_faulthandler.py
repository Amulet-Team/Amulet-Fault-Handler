import unittest
import multiprocessing
import os
import sys
from tempfile import TemporaryDirectory


import amulet_faulthandler

from _test_amulet_faulthandler import throw_access_violation


def sub_throw_access_violation(log_path: str):
    amulet_faulthandler.install(log_path, False)
    throw_access_violation()


class FaulthandlerTestCase(unittest.TestCase):
    def test_faulthandler(self):
        with TemporaryDirectory() as temp_directory:
            log_path = os.path.join(temp_directory, "crash.dmp")
            p = multiprocessing.Process(
                target=sub_throw_access_violation, args=(log_path,)
            )
            p.start()
            p.join()
            self.assertTrue(p.exitcode)
            if sys.platform == "win32":
                self.assertEqual(p.exitcode, 0xC0000005)
                self.assertTrue(os.path.isfile(log_path))


if __name__ == "__main__":
    unittest.main()
