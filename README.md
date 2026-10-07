# AdHoc Engine

A game engine built as a student project, developed mainly between 2021 and 2022.
It was built as a learning exercise to explore different areas of engine development.

## Getting started

### Clone the repository

```sh
git clone --recursive https://github.com/vostranox/adhoc-engine.git
cd adhoc-engine
```

### Build

Run these commands from the repository root on Windows, macOS, or Linux.

```sh
cmake --workflow --preset <config>
```

- `<config>` — `debug` or `release`

Set the `CMAKE_BUILD_PARALLEL_LEVEL` environment variable to limit parallel build jobs (for example, `8`). Otherwise, the build tool uses its default.

To also run the editor, use `<config>-run`:

```sh
cmake --workflow --preset release-run
```
