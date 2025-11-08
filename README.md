# General Magic Watchface

![General Magic Preview](GeneralMagic_preview.png)

General Magic is a Pebble watchface experiment that renders both the background grid and the digits as smoothly-eased diagonal artifacts. The native C code lives inside `GeneralMagic/src/c`, JavaScript helpers (currently only logging) inside `GeneralMagic/src/pkjs`, and Pebble build tooling is configured via `GeneralMagic/wscript`.

## Building

```sh
cd GeneralMagic
pebble build
```

## Deploying

```sh
cd GeneralMagic
pebble install --emulator basalt   # or any supported platform
```

The resulting `.pbw` bundle is generated under `GeneralMagic/build`.
