const std = @import("std");
const builtin = @import("builtin");
const app_name = "crosswire";

const release_flags = &[_][]const u8{
    "-DNDEBUG",
    "-std=c++20",
    "-DFMT_HEADER_ONLY",
    "-DTHELIB_OPT_T_LOGGING",
    // enable checks within optional before reset() and emplace() etc calls
    // also may cause UB if disabled
    "-DTHELIB_OPT_T_CHECKED",
    "-DALLO_LOGGING",
    "-DALLO_ALLOC_RESULT_CHECKED",
    "-DTHELIB_RESULT_T_LOGGING",
    "-DTHELIB_SLICE_T_LOGGING",
    "-DTHELIB_ONE_OF_T_LOGGING",
    "-DALLO_STACK_ALLOCATOR_USE_CTTI",
};
const debug_flags = &[_][]const u8{
    "-g",
    "-fPIC",
    "-DWERMS_DEBUG",
    "-DTHELIB_DEBUG",
    "-std=c++20",
    // enable exceptions in debug mode
    "-DFMT_HEADER_ONLY",
    "-DTHELIB_OPT_T_LOGGING",
    // enable checks within optional before reset() and emplace() etc calls
    // also may cause UB if disabled
    "-DTHELIB_OPT_T_CHECKED",
    "-DALLO_LOGGING",
    // whether
    "-DALLO_ALLOC_RESULT_CHECKED",
    "-DTHELIB_RESULT_T_LOGGING",
    "-DTHELIB_SLICE_T_LOGGING",
    "-DTHELIB_ONE_OF_T_LOGGING",
    "-DALLO_STACK_ALLOCATOR_USE_CTTI",
};

const testing_flags = &[_][]const u8{
    "-DFMT_EXCEPTIONS=1",
    "-DTESTING",
    "-DTESTING_NOEXCEPT=",
    "-DTESTING_THELIB_OPT_T_NO_NOTHROW",
    "-DTESTING_THELIB_RESULT_T_NO_NOTHROW",
    "-DTESTING_ALLO_STACK_ALLOCATOR_T_NO_NOTHROW",
    "-DTESTING_THELIB_ONE_OF_T_NO_NOTHROW",
};

const non_testing_flags = &[_][]const u8{
    "-DTESTING_NOEXCEPT=noexcept",
    "-DFMT_EXCEPTIONS=0",
    "-fno-exceptions",
    "-fno-rtti",
};

const zcc = @import("compile_commands");

const build_emscripten = @import("build_emscripten.zig");

const cpp_sources = &[_][]const u8{
    "src/thelib/body.cpp",
    "src/thelib/shape.cpp",
    "src/thelib/space.cpp",
    "src/thelib/vect.cpp",
    "src/thelib/contact.cpp",
    "src/thelib/arbiter.cpp",
    "src/natural_log/natural_log.cpp",
    "src/allo/c_allocator.cpp",
    "src/allo/random_allocation_registry.cpp",
    "src/allo/stack_allocator.cpp",
    "src/render_pipeline.cpp",
    "src/bullet.cpp",
    "src/main.cpp",
    "src/globals.cpp",
    "src/input.cpp",
    "src/physics.cpp",
    "src/player.cpp",
    "src/terrain.cpp",
    "src/resources.cpp",
    "src/build_site.cpp",
    "src/wire.cpp",
    "src/level_loader.cpp",
    "src/turret.cpp",
};

const include_dirs = &[_][]const u8{
    "src/",
    "src/include/",
};

const test_source_files = &[_][]const u8{
    "vect_t/vect_t.cpp",
    "result_t/result_t.cpp",
    "result_t/result_t_static_asserts.cpp",
    "opt_t/opt_t.cpp",
    "stack_allocator_t/stack_allocator_t.cpp",
    "slice_t/slice_t.cpp",
    "pool_allocator_generational_t/pool_allocator_generational_t.cpp",
};

const Library = struct {
    // name in build.zig
    remote_name: []const u8,
    // the name given to this library in its build.zig. usually in addStaticLibrary
    artifact_name: []const u8,
    imported: ?*std.Build.Dependency,

    fn artifact(self: @This()) *std.Build.CompileStep {
        return self.imported.?.artifact(self.artifact_name);
    }
};

var raylib = Library{ .remote_name = "raylib", .artifact_name = "raylib", .imported = null };
var chipmunk = Library{ .remote_name = "chipmunk2d", .artifact_name = "chipmunk", .imported = null };
var fmt: ?*std.Build.Dependency = null;
var fmt_include_path: ?[]u8 = null;

pub fn build(b: *std.Build) !void {
    // options
    const target = b.standardTargetOptions(.{});
    const mode = b.standardOptimizeOption(.{});

    const testing = b.option(bool, "testing", "Build the program with testing enabled. Corresponds mostly to exceptions being enabled.") orelse true;

    var flags = std.ArrayList([]const u8).init(b.allocator);
    defer flags.deinit();
    try flags.appendSlice(if (mode == .Debug) debug_flags else release_flags);

    try flags.appendSlice(if (testing) testing_flags else non_testing_flags);

    var targets = std.ArrayList(*std.Build.Step.Compile).init(b.allocator);
    defer targets.deinit();
    var tests = std.ArrayList(*std.Build.Step.Compile).init(b.allocator);
    defer tests.deinit();

    // create executable
    var exe: *std.Build.CompileStep =
        b.addExecutable(.{
        .name = app_name,
        .optimize = mode,
        .target = target,
    });
    try targets.append(exe);
    b.installArtifact(exe);

    // import and link all libraries
    raylib.imported = b.dependency(raylib.remote_name, .{
        .target = target,
        .optimize = mode,
    });
    chipmunk.imported = b.dependency(chipmunk.remote_name, .{
        .target = target,
        .optimize = mode,
        .use_doubles = false,
    });

    // dont pass any build options to fmt
    fmt = b.dependency("fmt", .{});
    fmt_include_path = std.fs.path.join(b.allocator, &.{ fmt.?.builder.install_path, "include" }) catch @panic("OOM");

    // HACK: this is just here to fix compile_commands.json not having chipmunk include_dirs
    {
        for (chipmunk.artifact().installed_headers.items) |include_dir_step| {
            const path = b.pathJoin(&.{ include_dir_step.owner.install_prefix, "include" });
            defer b.allocator.free(path);
            flags.append(b.fmt("-I{s}", .{path})) catch @panic("OOM");
        }
        flags.append("-DCP_USE_DOUBLES=0") catch @panic("OOM");
    }

    linkLibrariesFor(exe);

    for (include_dirs) |include_dir| {
        try flags.append(b.fmt("-I{s}", .{include_dir}));
    }
    // add editor headers as private includes
    {
        const editor_dep = b.dependency("editor", .{ .target = target, .optimize = mode });
        try flags.append(b.fmt("-I{s}/include", .{editor_dep.builder.install_path}));
    }

    var tests_lib = b.addSharedLibrary(.{
        .name = "main",
        .target = target,
        .optimize = mode,
    });
    linkLibrariesFor(tests_lib);

    switch (target.getOsTag()) {
        .wasi, .emscripten => {
            // emcc will provide the libc in the case of emscripten
            exe.is_linking_libcpp = false;
            if (mode == .Debug) @panic("Can only compile emscripten in release mode.");
            var library_artifacts = std.ArrayList(*std.Build.Step.Compile).init(b.allocator);
            defer library_artifacts.deinit();
            try library_artifacts.append(raylib.artifact());
            try library_artifacts.append(chipmunk.artifact());
            const emscripten_lib = build_emscripten.buildEmscripten(b, .{
                .app_name = app_name,
                .optimize = mode,
                .target = target,
                .cpp_sources = cpp_sources,
                .libraries_to_link = library_artifacts.items,
                .owned_flags = try flags.toOwnedSlice(),
            });
            try targets.append(emscripten_lib);
            exe.linkLibrary(emscripten_lib);
        },
        else => {
            const flags_owned = flags.toOwnedSlice() catch @panic("OOM");
            const all_sources_owned = cpp_sources;
            exe.addCSourceFiles(all_sources_owned, flags_owned);
            tests_lib.addCSourceFiles(all_sources_owned, flags_owned);
            // set up tests (executables which dont link artefacts built from
            // all_sources_owned but they do need flags, so we do it in this
            // scope so we can have flags_owned)
            {
                for (test_source_files) |source_file| {
                    var test_exe = b.addExecutable(.{
                        .name = std.fs.path.stem(source_file),
                        .optimize = mode,
                        .target = target,
                    });
                    test_exe.addCSourceFile(.{
                        .file = .{ .path = b.pathJoin(&.{ "tests", source_file }) },
                        .flags = flags_owned,
                    });
                    test_exe.addIncludePath(.{ .path = "tests/" });
                    linkLibrariesFor(test_exe);
                    try tests.append(test_exe);
                }
            }
        },
    }

    // add "zig build run"
    {
        const run_cmd = b.addRunArtifact(exe);
        run_cmd.step.dependOn(b.getInstallStep());
        if (b.args) |args| {
            run_cmd.addArgs(args);
        }
        const run_step = b.step("run", "Run the app");
        run_step.dependOn(&run_cmd.step);
    }

    // zig build editor
    {
        const editor_dep = b.dependency("editor", .{ .target = target, .optimize = mode });
        const editor_artifact = editor_dep.artifact("crosswire_editor");
        editor_artifact.addIncludePath(.{ .path = try b.build_root.join(b.allocator, &.{"levels/editor_include/"}) });
        const run_cmd = b.addRunArtifact(editor_artifact);
        run_cmd.step.dependOn(editor_dep.builder.getInstallStep());
        const editor_step = b.step("editor", "Run the level editor");
        editor_step.dependOn(&run_cmd.step);
    }

    // windows requires that no targets use pkg-config. of course.
    // because its a unix thing.
    switch (target.getOsTag()) {
        .windows => for (targets.items) |t| {
            unsetPkgConfig(t);
        },
        else => {},
    }

    // make step that runs all of the tests
    // TODO: add some custom step that just checks to see if we're in testing mode or not
    const run_tests_step = b.step("run_tests", "Compile and run all the tests");
    const install_tests_step = b.step("install_tests", "Install all the tests but don't run them");
    for (tests.items) |test_exe| {
        const test_install = b.addInstallArtifact(test_exe, .{});
        install_tests_step.dependOn(&test_install.step);

        test_exe.linkLibrary(tests_lib);
        const test_run = b.addRunArtifact(test_exe);
        if (b.args) |args| {
            test_run.addArgs(args);
        }
        run_tests_step.dependOn(&test_run.step);
    }

    targets.appendSlice(tests.toOwnedSlice() catch @panic("OOM")) catch @panic("OOM");

    zcc.createStep(b, "cdb", try targets.toOwnedSlice());
}

fn linkLibrariesFor(c: *std.Build.Step.Compile) void {
    c.linkLibrary(raylib.artifact());
    c.linkLibrary(chipmunk.artifact());
    c.linkLibCpp();
    {
        c.step.dependOn(fmt.?.builder.getInstallStep());
        c.addIncludePath(.{
            .path = fmt_include_path.?,
        });
    }
}

// Recursively unset all link objects' use_pkg_config setting
// fix for https://github.com/ziglang/zig/issues/14341
fn unsetPkgConfig(compile: *std.Build.Step.Compile) void {
    for (compile.link_objects.items) |*lo| {
        switch (lo.*) {
            .system_lib => |*system_lib| {
                system_lib.use_pkg_config = .no;
            },
            .other_step => |child_compile| unsetPkgConfig(child_compile),
            else => {},
        }
    }
}
