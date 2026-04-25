from dataclasses import dataclass
import argparse
from pathlib import Path
import subprocess


@dataclass
class RendererShaderGenerateArguments:
    shader_file: Path
    shader_name: str

    source_header: Path
    source_source: Path

    generated_header: Path
    generated_source: Path
    # build_dynamic: bool
    # manifest_toml: Path
    # source_plugin_register_inc: Path
    # source_plugin_dependencies: Path
    # generated_plugin_register_inc: Path
    # generated_plugin_dependencies: Path

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

def compile_glsl_to_spirv(shader_file: Path) -> bytes:
    try:
        result = subprocess.run(
            ["glslc", str(shader_file), "-o", "-"],
            check=True,
            capture_output=True
        )

        return result.stdout
    except subprocess.CalledProcessError as e:
        print(f"ERROR: Failed to compile shader {shader_file}")
        print(e.stderr.decode('utf-8', errors='replace'))
        raise SystemExit(1)
        

def parse_renderer_shader_arguments() -> RendererShaderGenerateArguments:
    parser = argparse.ArgumentParser(description="Generate plugin files")

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


def main():
    arguments = parse_renderer_shader_arguments()

    with open("C:/Users/Blue-WaveMathijsFran/Documents/GitHub/plugin-framework/plugins/renderer_vulkan/shaders/build/gradient.comp.spv", "rb") as f:
        pre_compiled_bytes = f.read()
    new_compiled_bytes = compile_glsl_to_spirv(arguments.shader_file)

    print(pre_compiled_bytes)
    print(new_compiled_bytes)

    print("Is same same?")
    print(pre_compiled_bytes == new_compiled_bytes)



if __name__ == "__main__":
    main()
