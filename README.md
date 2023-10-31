# C++ Example: VRS Viewer

This is an example that shows how to use [Rerun](https://github.com/rerun-io/rerun)'s C++ API to log and view VRS files.

You can get a sample `.vrs` file at <https://www.projectaria.com/datasets/apd/#download-dataset>.

## Using `pixi`
The easiest way to get started is to install [pixi](https://prefix.dev/docs/pixi/overview).

The pixi environment described in `pixi.toml` contains all of the dependencies, including the rerun viewer,
allowing you to run the example with a single command:
* `pixi run example`

## Without `pixi`
If you choose not to use pixi, you will need to install a few things yourself before you get started.

### Installing the Rerun Viewer
The Rerun C++ SDK works by connecting to an awaiting Rerun Viewer over TCP.

If you need to install the viewer, follow the [installation guide](https://www.rerun.io/docs/getting-started/installing-viewer). Two of the more common ways to install the Rerun are:
* Via cargo: `cargo install rerun-cli`
* Via pip: `pip install rerun-sdk`

After you have installed it, you should be able to type `rerun` in your terminal to start the viewer.

### Build on Linux & Mac

Build:
```bash
cmake -B build
cmake --build build -j
```
