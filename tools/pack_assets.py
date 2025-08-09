"""
Pack assets into a ZIP file, transforming them if necessary.
"""
import os
import re
import subprocess
from typing import Callable
from zipfile import ZipFile, ZIP_DEFLATED


def replace_ext(filepath: str, ext: str) -> str:
    return os.path.splitext(filepath)[0] + ext


def process_texture(filepath: str, output_path: str) -> str | None:
    """
    Compress textures using Basis Universal compressor `basisu` if installed.
    """
    try:
        fmt = "basis"
        output_path = replace_ext(output_path, f".{fmt}")
        if not os.path.exists(output_path) or os.path.getmtime(output_path) < max(os.path.getmtime(filepath), os.path.getmtime(__file__)):
            subprocess.run(["basisu", f"-{fmt}", "-etc1s", "-q", "255", filepath, "-output_file", output_path])
        return f".{fmt}"
    except:
        pass


def process_texture_atlas(filepath: str, output_path: str) -> None:
    """
    Compacts texture atlas XML files.
    Our parser ignores XML format and simply reads the attributes in order.
    """
    assert(os.path.exists(os.path.splitext(filepath)[0] + ".png"))
    contents = []
    wanted_content = re.compile(r'name=[^/]+')
    with open(filepath, "r") as f:
        for line in f:
            if m := wanted_content.search(line):
                contents.append(m.group(0))
    with open(output_path, "w") as f:
        f.write("\n".join(contents))


ASSET_PROCESSOR: dict[str, Callable[[str, str], None | str]] = {
    ".png": process_texture,
    ".xml": process_texture_atlas,
}


def pack_assets(assets_folder: str, zipname: str, build_folder: str):
    with ZipFile(zipname, 'w', compression=ZIP_DEFLATED) as zipfile:
        for dirname, dirs, files in os.walk(assets_folder):
            for filename in files:
                # skip hidden files, like ".DS_Store"
                if filename.startswith("."):
                    continue
                filepath = os.path.join(dirname, filename)
                archivepath = os.path.relpath(filepath, assets_folder)
                build_filepath = os.path.join(build_folder, archivepath)
                os.makedirs(os.path.dirname(build_filepath), exist_ok=True)
                ext = os.path.splitext(filepath)[1]
                if (f := ASSET_PROCESSOR.get(ext)) and (new_ext := f(filepath, build_filepath)):
                    zipfile.write(replace_ext(build_filepath, new_ext), replace_ext(archivepath, new_ext))
                elif os.path.exists(build_filepath):
                    zipfile.write(build_filepath, archivepath)
                else:
                    zipfile.write(filepath, archivepath)


if __name__ == "__main__":
    import argparse

    argparser = argparse.ArgumentParser(description=__doc__)
    argparser.add_argument("assets_folder", help="Path to the root of the assets folder")
    argparser.add_argument("zip_name", help="Path to the generated zip file")
    argparser.add_argument("build_folder", help="Path to the build folder, where intermediate build files will be written to")
    args = argparser.parse_args()
    pack_assets(args.assets_folder, args.zip_name, args.build_folder)
