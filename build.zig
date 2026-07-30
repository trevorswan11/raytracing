const std = @import("std");
const zon = @import("build.zig.zon");
pub const stdx = @import("stdx");

const CDBGenerator = stdx.CDBGenerator;
const LOCCounter = stdx.LOCCounter;

pub fn build(b: *std.Build) !void {
    const optimize = b.standardOptimizeOption(.{
        .preferred_optimize_mode = .ReleaseFast,
    });

    const profile = b.option(bool, "profile", "Enable chromium tracing") orelse false;
    const stdx_dep = b.dependency("stdx", .{
        .target = b.graph.host,
        .optimize = optimize,
        .profile = profile,
        .building_for_dep = true,
        .run_cdb_gen = false,
    });

    var compiler_flags: stdx.ArrayList([]const u8) = .fromSlice(b, &stdx.utils.base_cxx_flags);
    compiler_flags.appendSlice(&.{ "-DMAGIC_ENUM_RANGE_MAX=255", "-DSPDLOG_COMPILED_LIB" });
    stdx.CDBGenerator.addCdbFlags(b, &compiler_flags.wrapped);
    switch (optimize) {
        .Debug => compiler_flags.appendSlice(&.{ "-g", "-DRAY_DEBUG" }),
        .ReleaseSafe => compiler_flags.appendSlice(&.{"-DRAY_RELEASE"}),
        .ReleaseFast, .ReleaseSmall => compiler_flags.appendSlice(&.{ "-DNDEBUG", "-DRAY_DIST" }),
    }

    const install_tests_only = b.option(
        bool,
        "install-tests-only",
        "Install tests without running them (default: false)",
    ) orelse false;

    const cdb_gen: *CDBGenerator = .init(b);
    var cdb_steps: stdx.ArrayList(*std.Build.Step) = .init(b);

    _ = try addArtifacts(b, .{
        .optimize = optimize,
        .cxx_flags = compiler_flags.wrapped.items,
        .cdb_steps = &cdb_steps,
        .install_tests_only = install_tests_only,
        .stdx_dep = stdx_dep,
        .profile = profile,
    });
    for (cdb_steps.wrapped.items) |cdb_step| cdb_gen.step.dependOn(cdb_step);

    try addToolingSteps(b, .{
        .cdb_gen = cdb_gen,
        .cppcheck = stdx_dep.artifact("cppcheck"),
    });
}

const version_str = zon.version;
const version = std.SemanticVersion.parse(version_str) catch @compileError("Malformed version");

fn addArtifacts(b: *std.Build, config: struct {
    optimize: std.builtin.OptimizeMode,
    cxx_flags: []const []const u8,
    cdb_steps: *stdx.ArrayList(*std.Build.Step),
    install_tests_only: bool = false,
    stdx_dep: *std.Build.Dependency,
    profile: bool = false,
}) !struct {
    libray: *std.Build.Step.Compile,
    raytracer: *std.Build.Step.Compile,
    testray: *std.Build.Step.Compile,
} {
    const target = b.graph.host;
    const config_h = b.addConfigHeader(.{ .include_path = "raytracer/config.h" }, .{
        .RAY_VERSION_STR = version_str,
        .RAY_VERSION_MAJOR = @as(i64, version.major),
        .RAY_VERSION_MINOR = @as(i64, version.minor),
        .RAY_VERSION_PATCH = @as(i64, version.patch),
        .RAY_VERSION_PRE = version.pre orelse "",
        .RAY_GIT_INFO = stdx.utils.getGitInfo(b),
        .RAY_WINDOWS = target.result.os.tag == .windows,
        .RAY_LINUX = target.result.os.tag == .linux,
        .RAY_APPLE = target.result.os.tag == .macos,
    });

    const include, const src, const tests = .{ "include", "src/raytracer", "tests" };
    const libstdx = config.stdx_dep.artifact("stdx");
    const stb_dep = b.dependency("stb", .{});

    // Static library
    const libray = b.addLibrary(.{
        .name = "raytracer",
        .root_module = stdx.utils.createModule(b, .{
            .target = target,
            .optimize = config.optimize,
            .include_paths = &.{ b.path(include), b.path(src) },
            .system_include_paths = &.{stb_dep.path(".")},
            .cxx = .{
                .files = try stdx.utils.collectFiles(
                    b,
                    src,
                    .{ .allowed_extensions = &.{".cc"} },
                ),
                .flags = config.cxx_flags,
            },
            .config_headers = &.{config_h},
            .link_libraries = &.{libstdx},
        }),
    });
    libray.installHeadersDirectory(b.path(include), "", .{ .include_extensions = &.{".hh"} });
    config.cdb_steps.append(&libray.step);
    b.installArtifact(libray);

    // Executable
    const raytracer = stdx.utils.createExecutable(b, .{
        .target = target,
        .optimize = config.optimize,
        .cxx = .{
            .files = &.{"src/main.cc"},
            .flags = config.cxx_flags,
        },
        .link_libraries = &.{ libstdx, libray },
    }, .{
        .name = "raytracer",
        .behavior = .{
            .installable = .{
                .cmd_name = "run",
                .cmd_desc = "Run raytracer with provided command line arguments",
            },
        },
    });
    b.installArtifact(raytracer);
    config.cdb_steps.append(&raytracer.step);

    // Catch2 tests
    const test_artifact = stdx.builders.strappedTest(b, .{
        .target = target,
        .optimize = config.optimize,
        .stdx = .{ .dep = config.stdx_dep },
        .cxx_files = try stdx.utils.collectFiles(b, tests, .{}),
        .cxx_flags = config.cxx_flags,
        .profile = config.profile,
        .include_paths = &.{ b.path(include), b.path(tests) },
        .link_libraries = &.{libray},
        .config_headers = &.{config_h},
        .executable_config = .{
            .name = "raytracer",
            .behavior = .{
                .installable = .{
                    .cmd_name = "test",
                    .cmd_desc = "Build/run raytracer tests",
                    .install_dir = tests,
                    .install_only = config.install_tests_only,
                },
            },
        },
    });
    config.cdb_steps.append(&test_artifact.step);

    return .{
        .libray = libray,
        .raytracer = raytracer,
        .testray = test_artifact,
    };
}

fn addToolingSteps(b: *std.Build, config: struct {
    cdb_gen: *CDBGenerator,
    cppcheck: *std.Build.Step.Compile,
}) !void {
    const tooling_paths: stdx.steps.FmtPaths = .{
        .cxx = blk: {
            var paths: stdx.ArrayList([]const u8) = .init(b);
            try stdx.utils.collectFilesInto(b, "include", .{ .allowed_extensions = &.{".hh"} }, &paths);
            try stdx.utils.collectFilesInto(b, "src", .{ .allowed_extensions = &.{ ".hh", ".cc" } }, &paths);
            try stdx.utils.collectFilesInto(b, "tests", .{ .allowed_extensions = &.{ ".hh", ".cc" } }, &paths);
            break :blk paths.wrapped.items;
        },
        .zig = &.{"build.zig"},
    };

    _ = stdx.steps.addFmt(b, .{
        .paths = tooling_paths,
        .formatter = .{ .version = "21.1.8" },
    }) catch {};

    _ = stdx.steps.addCppcheck(b, .{
        .cppcheck = config.cppcheck,
        .cdb_gen = config.cdb_gen,
    });

    var counted_files: stdx.ArrayList([]const u8) = .init(b);
    counted_files.appendSlice(tooling_paths.cxx);
    counted_files.appendSlice(tooling_paths.zig);
    _ = LOCCounter.init(b, counted_files.wrapped.items);
}
