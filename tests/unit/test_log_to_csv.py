import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "scripts"))

from log_to_csv import convert


class LogToCsvTests(unittest.TestCase):
    def test_valid_measurement_without_state_is_not_invalid(self):
        rows = convert([
            "cycle=1 timestamp_ms=2000 temp=25.3C humidity=56% light=497 lux "
            "valid=(1,1,1)",
        ])

        self.assertEqual(rows[0]["system_state"], "NO_STATE")

    def test_invalid_sensor_flags_produce_invalid_state(self):
        rows = convert([
            "cycle=12 timestamp_ms=24000 temp=0.5C humidity=57% light=684 lux "
            "valid=(0,1,1)",
        ])

        self.assertEqual(rows[0]["system_state"], "INVALID")


if __name__ == "__main__":
    unittest.main()