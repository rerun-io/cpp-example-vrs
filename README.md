# C++ Example: VRS Viewer

This is an example that shows how to use [Rerun](https://github.com/rerun-io/rerun)'s C++ API to log and view [VRS](https://github.com/facebookresearch/vrs) files.

> VRS is a file format optimized to record & playback streams of sensor data, such as images, audio samples, and any other discrete sensors (IMU, temperature, etc), stored in per-device streams of time-stamped records.

You can download a sample `.vrs` file from <https://www.projectaria.com/datasets/apd/#download-dataset>.

<picture>
  <img src="https://static.rerun.io/cpp-example-vrs/c13ed42c13ecb65b0ef689533c0525ab97471e21/full.png" alt="">
  <source media="(max-width: 480px)" srcset="https://static.rerun.io/cpp-example-vrs/c13ed42c13ecb65b0ef689533c0525ab97471e21/480w.png">
  <source media="(max-width: 768px)" srcset="https://static.rerun.io/cpp-example-vrs/c13ed42c13ecb65b0ef689533c0525ab97471e21/768w.png">
  <source media="(max-width: 1024px)" srcset="https://static.rerun.io/cpp-example-vrs/c13ed42c13ecb65b0ef689533c0525ab97471e21/1024w.png">
  <source media="(max-width: 1200px)" srcset="https://static.rerun.io/cpp-example-vrs/c13ed42c13ecb65b0ef689533c0525ab97471e21/1200w.png">
</picture>

## Compile and run using `pixi`
The easiest way to get started is to install [pixi](https://prefix.dev/docs/pixi/overview).

The pixi environment described in `pixi.toml` contains all of the dependencies, including the rerun viewer,
allowing you to run the example with a single command:
* `pixi run example dataset.vrs`

## Compile and run without `pixi`
If you choose not to use pixi, you will need to install a few things yourself before you get started.

### Installing the Rerun Viewer
The Rerun C++ SDK works by connecting to an awaiting Rerun Viewer over TCP.

If you need to install the viewer, follow the [installation guide](https://www.rerun.io/docs/getting-started/installing-viewer). Two of the more common ways to install the Rerun Viewer are:
* Via cargo: `cargo install rerun-cli`
* Via pip: `pip install rerun-sdk`

After you have installed it, you should be able to type `rerun` in your terminal to start the viewer.

### Build on Linux & Mac

Build:

```bash
cmake -B build
cmake --build build -j
```

Run:
```
build/rerun_vrs_example
```

## Windows
We haven't tried getting this example working on Windows yet, because VRS has no Windows build instructions: <https://github.com/facebookresearch/vrs?tab=readme-ov-file#windows-support>.


## Known limitations with Rerun
* 0.10.0 of the Rerun C++ SDK accidentally shipped with parts of it compiled in Debug build, making it unnecessarily slow. This will be fixed in [Rerun 0.10.1](https://github.com/rerun-io/rerun/milestone/11).
* Time-scalar plots are currently very slow in Rerun, something we're [actively working on](https://github.com/rerun-io/rerun/issues/374).
* [Points cloud sizes are limited](https://github.com/rerun-io/rerun/issues/3076), and [big point clouds are slow](https://github.com/rerun-io/rerun/issues/1136).
