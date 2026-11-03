#!/usr/bin/env python3
"""
Parallel Google Test runner for Fakelua using GTest sharding.
Spawns multiple workers in parallel to dramatically cut down test run times.
"""

import argparse
import os
import re
import subprocess
import sys
import time
from concurrent.futures import ThreadPoolExecutor, as_completed


def parse_args():
    parser = argparse.ArgumentParser(description="Run fakelua unit tests in parallel")
    parser.add_argument(
        "-j", "--jobs",
        type=int,
        default=os.cpu_count() or 4,
        help="Number of parallel shards (default: cpu_count)"
    )
    parser.add_argument(
        "-f", "--filter",
        type=str,
        default="-*redis*",
        help="GTest filter pattern (default: -*redis*)"
    )
    parser.add_argument(
        "--build-dir",
        type=str,
        default=None,
        help="Path to CMake build directory (auto-detected if omitted)"
    )
    return parser.parse_args()


def find_build_dir(hint=None):
    if hint:
        p = os.path.abspath(hint)
        if os.path.exists(os.path.join(p, "bin", "unit_tests")):
            return p
        if os.path.exists(os.path.join(p, "unit_tests")):
            return p
        raise FileNotFoundError(f"unit_tests not found in {hint}")

    candidates = ["build-macos", "build", "cmake-build-debug", "cmake-build-release"]
    repo_root = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
    for c in candidates:
        candidate_bin = os.path.join(repo_root, c, "bin", "unit_tests")
        if os.path.isfile(candidate_bin):
            return os.path.join(repo_root, c)
        candidate_bin2 = os.path.join(repo_root, c, "unit_tests")
        if os.path.isfile(candidate_bin2):
            return os.path.join(repo_root, c)

    raise FileNotFoundError("Could not auto-detect build directory containing unit_tests. Please specify with --build-dir")


import shutil
import tempfile


def run_shard(shard_idx, total_shards, bin_path, base_bin_dir, test_filter):
    # Create isolated scratch directory for this shard so file I/O tests don't collide
    shard_scratch_dir = tempfile.mkdtemp(prefix=f"flua_shard_{os.getpid()}_{shard_idx}_")
    try:
        # Symlink all entries from base_bin_dir (lua script dirs, bins, etc.)
        for entry in os.listdir(base_bin_dir):
            src = os.path.join(base_bin_dir, entry)
            dst = os.path.join(shard_scratch_dir, entry)
            try:
                os.symlink(src, dst)
            except OSError:
                pass

        env = os.environ.copy()
        env["TMPDIR"] = shard_scratch_dir
        env["GTEST_TOTAL_SHARDS"] = str(total_shards)
        env["GTEST_SHARD_INDEX"] = str(shard_idx)

        cmd = [bin_path]
        if test_filter:
            cmd.append(f"--gtest_filter={test_filter}")

        start_time = time.time()
        proc = subprocess.run(
            cmd,
            cwd=shard_scratch_dir,
            env=env,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
        )
        elapsed = time.time() - start_time
        return shard_idx, proc.returncode, elapsed, proc.stdout, proc.stderr
    finally:
        shutil.rmtree(shard_scratch_dir, ignore_errors=True)


def main():
    args = parse_args()
    build_dir = find_build_dir(args.build_dir)
    
    bin_dir = os.path.join(build_dir, "bin")
    if os.path.isfile(os.path.join(bin_dir, "unit_tests")):
        unit_tests_bin = os.path.join(bin_dir, "unit_tests")
        run_cwd = bin_dir
    else:
        unit_tests_bin = os.path.join(build_dir, "unit_tests")
        run_cwd = build_dir

    print(f"==================================================")
    print(f" Fakelua Parallel Test Runner")
    print(f" Shards (jobs): {args.jobs}")
    print(f" Filter:        {args.filter}")
    print(f" Binary:        {unit_tests_bin}")
    print(f" CWD:           {run_cwd}")
    print(f"==================================================")

    # When running the full suite (-*redis*), separate out network tests (test_net.*)
    # because they bind to fixed local TCP/WS ports (e.g. 19992) and cannot run concurrently.
    main_filter = args.filter
    run_net_separately = False
    if args.filter == "-*redis*":
        main_filter = "-*redis*:test_net.*"
        run_net_separately = True

    start_total = time.time()
    results = [None] * args.jobs

    with ThreadPoolExecutor(max_workers=args.jobs) as pool:
        futures = {
            pool.submit(run_shard, i, args.jobs, unit_tests_bin, run_cwd, main_filter): i
            for i in range(args.jobs)
        }
        for future in as_completed(futures):
            idx, code, elapsed, stdout, stderr = future.result()
            results[idx] = (code, elapsed, stdout, stderr)
            status = "PASSED" if code == 0 else f"FAILED (code {code})"
            print(f"  [Shard {idx + 1:2d}/{args.jobs}] {status} in {elapsed:.2f}s")

    # Run network tests in isolation if full suite was requested
    if run_net_separately:
        print(f"  [Network Tests] Running test_net.* sequentially to avoid port collision...")
        net_idx, net_code, net_elapsed, net_stdout, net_stderr = run_shard(
            0, 1, unit_tests_bin, run_cwd, "test_net.*"
        )
        results.append((net_code, net_elapsed, net_stdout, net_stderr))
        status = "PASSED" if net_code == 0 else f"FAILED (code {net_code})"
        print(f"  [Network Tests] {status} in {net_elapsed:.2f}s")

    total_time = time.time() - start_total
    all_passed = all(r[0] == 0 for r in results if r)

    print("==================================================")
    total_tests_ran = 0
    failures = []

    for idx, (code, elapsed, stdout, stderr) in enumerate(results):
        # Parse tests ran
        m = re.search(r"(\d+) tests? from \d+ test suites? ran", stdout)
        if m:
            total_tests_ran += int(m.group(1))
        if code != 0:
            failures.append((idx, stdout, stderr))

    if all_passed:
        print(f" ALL TESTS PASSED! ({total_tests_ran} tests across {args.jobs} shards in {total_time:.2f}s)")
        print("==================================================")
        sys.exit(0)
    else:
        print(f" FAILED SHARDS: {len(failures)}/{args.jobs}")
        print("==================================================")
        for idx, stdout, stderr in failures:
            print(f"\n--- Output of failed Shard {idx + 1} ---")
            lines = stdout.splitlines()
            for line in lines[-50:]:  # Print last 50 lines
                print(line)
            if stderr:
                print("--- STDERR ---")
                print(stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()
