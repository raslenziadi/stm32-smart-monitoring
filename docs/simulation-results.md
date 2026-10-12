# Simulation results

## Expected behavior

- NORMAL remains below warning thresholds and keeps the system state NORMAL.
- OVERHEAT, HIGH_HUMIDITY and HIGH_LIGHT raise alarms only on their target
	channels; non-target channels remain below their warning thresholds.
- SIMULTANEOUS_ALARMS remains within the physical sensor bounds while producing
	multiple alarms.
- FAILURE preserves its injected invalid temperature flag from cycle 12 onward.

## Verified results

Host-side scenario tests passed for 600 cycles, using seeds `0x1234` and `1234`
where applicable. They verify NORMAL's alarm state, FAILURE validity propagation,
the 100% humidity ceiling in SIMULTANEOUS_ALARMS, and the non-target channel
limits in the individual alarm scenarios.

A 30-second `native_sim` run of a separate NORMAL build produced 14 measurements.
All 14 were sensor-valid and had system state NORMAL. The original build
directory was configured for FAILURE and was left unchanged. The normal run's
log, CSV, and plot are under the ignored `build/validation-normal/` directory.

The timed run ends while the application is active; a valid measurement without
an emitted state log is represented as `NO_STATE`, not `INVALID`, by the CSV
converter. Sensor-invalid measurements are represented as `INVALID` based on
their validity flags.

## Scope

These are host-test and `native_sim` results only. No physical STM32 hardware was
used, and the GitHub Actions Zephyr job has not yet run on a hosted runner.
