# mini-llvm

| main | develop |
|:-:|:-:|
| [![build](https://github.com/mini-llvm/mini-llvm/actions/workflows/build.yml/badge.svg?branch=main)](https://github.com/mini-llvm/mini-llvm/actions/workflows/build.yml?query=branch%3Amain) [![build-mips64-linux](https://github.com/mini-llvm/mini-llvm/actions/workflows/build-mips64-linux.yml/badge.svg?branch=main)](https://github.com/mini-llvm/mini-llvm/actions/workflows/build-mips64-linux.yml?query=branch%3Amain) | [![build](https://github.com/mini-llvm/mini-llvm/actions/workflows/build.yml/badge.svg?branch=develop)](https://github.com/mini-llvm/mini-llvm/actions/workflows/build.yml?query=branch%3Adevelop) [![build-mips64-linux](https://github.com/mini-llvm/mini-llvm/actions/workflows/build-mips64-linux.yml/badge.svg?branch=develop)](https://github.com/mini-llvm/mini-llvm/actions/workflows/build-mips64-linux.yml?query=branch%3Adevelop) |

## Design

![Design](assets/design.svg)

## Supported Targets

- `riscv64`

## Supported IR Features

### Types

- `void`
- Integer types
  - `i1`
  - `i8`
  - `i16`
  - `i32`
  - `i64`
- Floating-point types
  - `float`
  - `double`
- `ptr`
- Array types
- Function types
- `label`

### Constants

- Integer constants
- Floating-point constants
- `null`
- Array constants
- `poison`

### Instructions

- Terminators
  - `br`
  - `ret`
- Binary operators
  - Integer
    - Arithmetic
      - `add`
      - `sub`
      - `mul`
      - `sdiv`
      - `udiv`
      - `srem`
      - `urem`
      - `and`
      - `or`
      - `xor`
      - `shl`
      - `lshr`
      - `ashr`
    - Relational
      - `icmp`
  - Floating-point
    - Arithmetic
      - `fadd`
      - `fsub`
      - `fmul`
      - `fdiv`
      - `frem`
    - Relational
      - `fcmp`
- Unary operators
  - Floating-point
    - Arithmetic
      - `fneg`
- Casting operators
  - Integer
    - `trunc`
    - `sext`
    - `zext`
  - Floating-point
    - `fptrunc`
    - `fpext`
  - Integer to floating-point
    - `sitofp`
    - `uitofp`
  - Floating-point to integer
    - `fptosi`
    - `fptoui`
  - `ptrtoint`
  - `inttoptr`
  - `bitcast`
- Memory management
  - `alloca`
  - `load`
  - `store`
- `getelemenetptr`
- `select`
- `call`
- `phi`

## Supported Optimizations (Work in Progress)

### IR

- `AlgebraicSimplification`
- `ArrayFlattening`
- `BasicBlockMerging`
- `BranchSimplification`
- `ConstantFolding`
- `DeadCodeElimination`
- `DeadStoreElimination`
- `FunctionInlining`
- `GlobalValueNumbering`
- `InstructionCombining`
- `JumpThreading`
- `Mem2Reg`
- `PoisonPropagation`
- `RedundantLoadElimination`
- `StrengthReduction`
- `UnreachableBlockElimination`

### MIR

- `BasicBlockMerging`
- `BasicBlockReordering`
- `CopyPropagation`
- `DeadCodeElimination`
- `JumpThreading`
- `NullOperationElimination`
- `RegisterReuse`
- `StackOffsetEvaluation`
- `TailDuplication`
- `UnreachableBlockElimination`
- `ZeroRegisterReplacement`
- `RISCVConstantPropagation` (`riscv64`)

### MC

- `RedundantLabelElimination`
- `RISCVFallthrough` (`riscv64`)

## Supported Register Allocators (Work in Progress)

- `NaiveAllocator`
- `LinearScanAllocator`

## Usage

<table>
<tr>
<td>example.ll</td>
</tr>
<tr>
<td>

```llvm
define i32 @add(i32 %0, i32 %1) {
2:
  %3 = add i32 %0, %1
  ret i32 %3
}
```

</td>
</tr>
</table>

```sh
mini-llc --target=riscv64 -o example.s example.ll
```

## Building

| Linux | macOS | Windows |
|:-:|:-:|:-:|
| ✓ | ✗ | ✗ |

| GCC | Clang |
|:-:|:-:|
| ≥ 14 | ≥ 18 |

| CMake |
|:-:|
| ≥ 3.20 |

```sh
git clone --depth=1 --recurse-submodules --shallow-submodules <repo-url>
```

```sh
sudo apt-get update
sudo apt-get -y install g++-14 cmake ninja-build
cd <repo-dir>
mkdir build && cd build
cmake -DCMAKE_CXX_COMPILER=g++-14 -DCMAKE_BUILD_TYPE=Release -DMINI_LLVM_TESTS=ON -G Ninja ..
cmake --build .
./tools/mini-llc/mini-llc --help
```
## Running Tests

### Unit Tests

```sh
cd <repo-dir>/build
ctest .
```

### Integration Tests

```sh
sudo apt-get update
sudo apt-get -y install gcc-14-riscv64-linux-gnu qemu-user
sudo mkdir -p /usr/gnemul
sudo ln -s /usr/riscv64-linux-gnu /usr/gnemul/qemu-riscv64
cd <repo-dir>/tests/tools/mini-llc
export MINI_LLC_COMMAND="../../../build/tools/mini-llc/mini-llc"
export LINKER_COMMAND="riscv64-linux-gnu-gcc-14"
export EMULATOR_COMMAND="qemu-riscv64"
export DIFF_COMMAND="diff"
export MINI_LLC_TIMEOUT=60
export LINKER_TIMEOUT=60
export EMULATOR_TIMEOUT=60
./test-all.sh --target=riscv64
```

## Installing

```sh
cd <repo-dir>/build
cmake --install .
```

## License

mini-llvm is licensed under the [MIT license](https://opensource.org/licenses/MIT).
