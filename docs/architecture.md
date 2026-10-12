# Architecture document

## Module responsibilities

### Sensor manager

`src/sensors/sensor_manager.h` and `.c` define the common sensor message structure and validation helpers. They enforce physical bounds for temperature, humidity and light while keeping the logic independent from rendering or alarm state.

### Simulator

`src/simulator/sensor_simulator.c` generates deterministic sample values for each scenario. The simulator is intentionally separate from alarm logic so that the same measurement pipeline can be reused for tests and application execution.

### Alarm manager

`src/alarms/alarm_manager.c` evaluates the measurement stream against the configured thresholds. It uses hysteresis to avoid oscillation around thresholds, keeps sensor-specific state independent, and aggregates the global system state from the worst active alarm.

### Monitor manager

`src/monitoring/monitor_manager.c` tracks generated samples, received messages and dropped items. It also tracks timeouts and alarm transitions so that system quality can be assessed over time.

### Diagnostics

`src/diagnostics/diagnostics.c` centralizes fault and defect tracking. It distinguishes environmental alarm conditions from sensor failures and message loss.

### Output manager

`src/output/output_manager.c` produces structured logs with measurement details and global state. It integrates with the rest of the application without forcing the use of a specific user interface.

## Data flow

1. The periodic timer triggers a sensor tick.
2. The sensor thread calls the simulator.
3. The resulting `struct sensor_message` is validated.
4. Invalid data is recorded by diagnostics and discarded; valid data is placed in
	`sensor_msgq`.
5. The monitor thread consumes the message and invokes alarm evaluation.
6. Diagnostics and output components log state and counters.

## Threading model

- `sensor_thread`: periodic generator and producer.
- `monitor_thread`: consumer and alarm evaluator.
- `sensor_timer`: periodic scheduler using `k_timer_start`.
- `sensor_tick_sem`: gate between timer callback and sensor thread.
- `sensor_msgq`: queue between generation and evaluation.

The design keeps the producer and consumer decoupled while maintaining low complexity and deterministic behavior.

Invalid measurements do not reach `alarm_manager` in the application. Diagnostics
record the sensor fault, and the global alarm state remains the last state derived
from valid measurements. The alarm manager's own invalid-channel handling is
covered by host unit tests but is not exercised through this application path.
