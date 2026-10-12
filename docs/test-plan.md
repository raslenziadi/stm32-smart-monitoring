# Test plan

## Unit tests

The host test suite checks core alarm logic and deterministic scenario behavior.

### Expected outcomes

- Temperature warning is raised at 30.0 C.
- Temperature critical is raised at 35.0 C.
- Temperature returns to warning when above 29.0 C after a critical alarm.
- Temperature returns to normal below 29.0 C.
- Humidity follows the same hysteresis behavior.
- Invalid sensor values do not trigger environmental alarm logic.
- The same simulator scenario and seed produce identical output.
- NORMAL stays below warning thresholds for 600 cycles with seeds `0x1234` and
  `1234`.
- FAILURE preserves the injected invalid temperature from cycle 12 onward.
- SIMULTANEOUS_ALARMS stays within sensor-valid humidity bounds.
- Non-target channels remain below their warning thresholds in the individual
  alarm scenarios for both documented seeds.

### Actual results

The host-side test run executed successfully with the following command:

```bash
gcc -std=c11 -Wall -Wextra -Werror -I src \
  tests/unit/test_alarm_manager.c \
  src/sensors/sensor_manager.c \
  src/alarms/alarm_manager.c \
  src/simulator/sensor_simulator.c \
  -o /tmp/test_alarm_manager
/tmp/test_alarm_manager

gcc -std=c11 -Wall -Wextra -Werror -I src \
  tests/unit/test_scenarios.c \
  src/sensors/sensor_manager.c \
  src/alarms/alarm_manager.c \
  src/simulator/sensor_simulator.c \
  -o /tmp/test_scenarios
/tmp/test_scenarios

python3 tests/unit/test_log_to_csv.py
```

Observed result: both C executables PASS with `-Wall -Wextra -Werror`; both CSV
converter regression tests PASS.

## Runtime validation

The Zephyr build was executed using the standard project command:

```bash
source ~/zephyrproject/.venv/bin/activate
cd ~/zephyrproject
west build -p always -b native_sim \
  /path/to/stm32-smart-monitoring \
  -d /path/to/stm32-smart-monitoring/build
```

Observed result: PASS using Zephyr 4.5.0-rc1, Zephyr SDK 1.0.1, and the host
toolchain. The workspace's existing `build/` directory was configured for
FAILURE; a separate `build/validation-normal/` configuration was used for the
NORMAL runtime check.

A 30-second smoke test on the separate NORMAL build was run using:

```bash
timeout 30s ./build/validation-normal/zephyr/zephyr.exe
```

Observed result: 14 measurements, all 14 system states NORMAL, and no invalid
sensor flags. The timeout intentionally stops the periodic application.

## Not executed

- GitHub Actions Zephyr build job on a hosted runner.
- Twister/ZTest suite as a full Zephyr-native test project.
- Hardware validation on a physical STM32 board.
