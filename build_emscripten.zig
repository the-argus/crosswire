const std = @import("std");
const zcc = @import("compile_commands");

const emcc_executable = "emcc";

const BuildEmscriptenOptions = struct {
    app_name: []const u8,
    cpp_sources: []const []const u8,
    optimize: std.builtin.OptimizeMode,
    target: std.zig.CrossTarget,
    libraries_to_link: []*std.Build.Step.Compile,
    owned_flags: []const []const u8,
};

pub fn buildEmscripten(b: *std.Build, options: BuildEmscriptenOptions) *std.Build.Step.Compile {
    if (b.sysroot == null) {
        @panic("\n\nUSAGE: Pass the '--sysroot \"$EMSDK/upstream/emscripten\"' flag.\n\n");
    }

    const emscriptenSrc = "build/emscripten/";
    const webOutdir = b.pathJoin(&.{ b.install_prefix, "web" });
    std.fs.cwd().makePath(webOutdir) catch {};
    const webCache = b.pathJoin(&.{ b.install_prefix, "web_cache" });
    std.fs.cwd().makePath(webCache) catch {};
    const webOutFile = b.pathJoin(&.{ webOutdir, "game.html" });

    var flags = std.ArrayList([]const u8).init(b.allocator);
    defer flags.deinit();

    flags.appendSlice(options.owned_flags) catch @panic("OOM");

    // a wasm artifact. doesnt need emscripten because it doesnt define any of
    // the actual windowing or graphics related symbols
    var lib = b.addStaticLibrary(.{
        .name = options.app_name,
        .optimize = options.optimize,
        .target = options.target,
    });

    // using flags instead of addIncludePath because we want these includes to be
    // private to the C source files that need them
    const emscripten_include_flag = b.fmt("-I{s}/include", .{b.sysroot.?});
    const emscripten_cpp_include_flag = b.fmt("-I{s}/include/c++/v1", .{b.sysroot.?});

    // order here matters
    flags.append(emscripten_cpp_include_flag) catch @panic("OOM");
    flags.append(emscripten_include_flag) catch @panic("OOM");

    // add the libraries installation path /include dir to be included,
    // but don't actually link the libraries (let emcc invoke its linker)
    // this code is mostly copied from linkLibrary, just removing the linking
    // part
    for (options.libraries_to_link) |remote_lib| {
        for (remote_lib.installed_headers.items) |include_dir_step| {
            const path = b.pathJoin(&.{ include_dir_step.owner.install_prefix, "include" });
            defer b.allocator.free(path);
            flags.append(b.fmt("-I{s}", .{path})) catch @panic("OOM");
            // these include flags wont work unless we wait for installation to happen
            remote_lib.step.dependOn(include_dir_step);
        }
    }

    lib.addCSourceFiles(options.cpp_sources, flags.toOwnedSlice() catch @panic("OOM"));
    lib.defineCMacro("__EMSCRIPTEN__", null);
    lib.defineCMacro("PLATFORM_WEB", null);

    // const lib_output_include_flag = b.fmt("-I{s}/include", .{b.install_prefix});
    const shell_file = b.pathJoin(&.{ emscriptenSrc, "minshell.html" });
    const emcc_path = b.pathJoin(&.{ b.sysroot.?, "bin", emcc_executable });

    const command = &[_][]const u8{
        emcc_path,
        "-o",
        webOutFile,
        emscriptenSrc ++ "entry.c",
        "--cache",
        // this needs to be duped for some reason? I thought I owned the memory and
        // wasn't freeing it but whatever. im too tired to figure this out right now
        b.allocator.dupe(u8, webCache) catch @panic("OOM"),
        "-I.",
        "-L.",
        // "-I" ++ emscriptenSrc, // we arent including anything from emscriptenSrc
        // lib_output_include_flag,
        "--shell-file",
        shell_file,
        "-DPLATFORM_WEB",
        "-sUSE_GLFW=3",
        "-sWASM=1",
        "-sALLOW_MEMORY_GROWTH=1",
        "-sWASM_MEM_MAX=512MB", //going higher than that seems not to work on iOS browsers ¯\_(ツ)_/¯
        "-sTOTAL_MEMORY=512MB",
        "-sABORTING_MALLOC=0",
        "-sASYNCIFY",
        "-sFORCE_FILESYSTEM=1",
        "-sASSERTIONS=1",
        "--memory-init-file",
        "0",
        "--preload-file",
        "assets",
        "--source-map-base",
        // "-sLLD_REPORT_UNDEFINED",
        "-sERROR_ON_UNDEFINED_SYMBOLS=0",
        // optimizations
        "-O3",
        // "-Os",
        // "-sUSE_PTHREADS=1",
        // "--profiling",
        // "-sTOTAL_STACK=128MB",
        // "-sMALLOC='emmalloc'",
        // "--no-entry",
        "-sEXPORTED_FUNCTIONS=['_malloc','_free','_main', '_emsc_main','_emsc_set_window_size']",
        "-sEXPORTED_RUNTIME_METHODS=ccall,cwrap",
    };

    const emcc = b.addSystemCommand(command);

    for (options.libraries_to_link) |library| {
        emcc.addArtifactArg(library);
        emcc.step.dependOn(&library.step);
    }

    // add all the accumulated stuff to the command
    emcc.addArtifactArg(lib);
    emcc.step.dependOn(&lib.step);

    // emcc system command generates a file, so make sure its generated before
    // installation
    b.getInstallStep().dependOn(&emcc.step);

    std.log.info(
        \\
        \\Output files will be in {s}
        \\
        \\---
        \\cd {s}
        \\python -m http.server
        \\---
        \\
        \\building...
    ,
        .{ webOutdir, webOutdir },
    );

    return lib;
}
