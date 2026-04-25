from dataclasses import dataclass
import argparse
from pathlib import Path
import subprocess
from plugin_sdk_core.utils import configure_file
import struct


# TODO: Make a utility that automatically sets the values rather than defining it 3 times
@dataclass
class RendererShaderGenerateArguments:
    shader_file: Path
    shader_name: str

    source_header: Path
    source_source: Path

    generated_header: Path
    generated_source: Path

    @classmethod
    def from_args(cls, args: argparse.Namespace) -> "RendererShaderGenerateArguments":
        return cls(
            shader_file=args.shader_file_path,
            shader_name=args.shader_name,
            source_header=args.source_header_path,
            source_source=args.source_source_path,
            generated_header=args.generated_header_path,
            generated_source=args.generated_source_path,
        )


def parse_renderer_shader_arguments() -> RendererShaderGenerateArguments:
    parser = argparse.ArgumentParser(description="Generate shader files")

    parser.add_argument(
        "--shader-file-path",
        required=True,
        type=Path,
    )

    parser.add_argument(
        "--shader-name",
        required=True,
    )

    parser.add_argument(
        "--source-header-path",
        required=True,
        type=Path,
    )
    parser.add_argument(
        "--source-source-path",
        required=True,
        type=Path,
    )

    parser.add_argument(
        "--generated-header-path",
        required=True,
        type=Path,
    )
    parser.add_argument(
        "--generated-source-path",
        required=True,
        type=Path,
    )

    args = parser.parse_args()

    return RendererShaderGenerateArguments.from_args(args)


def compile_glsl_to_spirv(shader_file: Path) -> bytes:
    try:
        result = subprocess.run(
            ["glslc", str(shader_file), "-o", "-"], check=True, capture_output=True
        )

        return result.stdout
    except subprocess.CalledProcessError as e:
        print(f"ERROR: Failed to compile shader {shader_file}")
        print(e.stderr.decode("utf-8", errors="replace"))
        raise ValueError("Error running glslc command") from e


def generate_shader_code_files(
    header_source: Path,
    source_source: Path,
    header_destination: Path,
    source_destination: Path,
    shader_name: str,
    spirv_bytes: bytes,
):
    bytes_len = len(spirv_bytes)
    if bytes_len % 4 != 0:
        raise ValueError(f"SPIR-V byte length ({bytes_len}) is not a multiple of 4")

    u32_len = bytes_len // 4
    u32_values = struct.unpack(f"<{u32_len}I", spirv_bytes)
    shader_u32_code = [f"0x{u32_value:08X}" for u32_value in u32_values]

    replacements = [
        ("SHADER_NAME", shader_name),
        ("SCREAMING_SHADER_NAME", shader_name.upper()),
        ("SHADER_HEADER_FILE_NAME", header_destination.name),
        ("SHADER_U32_CODE", ",".join(shader_u32_code)),
        ("SHADER_U32_LEN", str(u32_len)),
        ("SHADER_BYTES_LEN", str(bytes_len)),
    ]

    configure_file(header_source, header_destination, replacements, False)
    configure_file(source_source, source_destination, replacements, False)


def main():
    arguments = parse_renderer_shader_arguments()

    spirv_bytes = compile_glsl_to_spirv(arguments.shader_file)
    generate_shader_code_files(
        arguments.source_header,
        arguments.source_source,
        arguments.generated_header,
        arguments.generated_source,
        arguments.shader_name,
        spirv_bytes,
    )


if __name__ == "__main__":
    main()
